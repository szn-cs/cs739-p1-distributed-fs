/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <experimental/filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
namespace fs = std::experimental::filesystem;

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "afs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;

using afs::CustomAFS;
using afs::Path;
using afs::ReadReply;
using afs::ReadRequest;
using afs::Response;
using afs::StatInfo;
using afs::WriteReply;
using afs::WriteRequest;
// EXAMPLE
using afs::HelloReply;
using afs::HelloRequest;

#define CHUNK_SIZE 1572864

std::string root_dir = fs::current_path().generic_string() + "/root_dir/";
fs::path path_root_dir(root_dir);

// Logic and data behind the server's behavior.
class AFSServerServiceImpl final : public CustomAFS::Service {
 public:
  Status Mkdir(ServerContext* context, const Path* request,
               Response* response) override {
    // std::cout << "trigger mkdir" << std::endl;
    std::string new_dir_path = root_dir + request->path();
    fs::path path_new_dir(new_dir_path);

    response->set_status(1);
    if (!fs::exists(new_dir_path)) {
      if (!fs::create_directories(new_dir_path)) {
        perror("Failed to initialize the root directory.");
        response->set_status(0);
      }

    } else {
      response->set_status(0);
    }
    return Status::OK;
  }

  Status Rmdir(ServerContext* context, const Path* request,
               Response* response) override {
    std::cout << "trigger rmdir" << std::endl;
    std::string remove_dir = root_dir + request->path();
    fs::path path_rm_dir(remove_dir);

    response->set_status(1);

    if (fs::exists(path_rm_dir)) {
      std::error_code errorCode;
      if (!fs::remove(path_rm_dir, errorCode)) {
        perror("Failed to rm directory.");
        response->set_status(0);
      }
    } else {
      response->set_status(0);
    }
    return Status::OK;
  }

  Status Unlink(ServerContext* context, const Path* request,
                Response* response) override {
    std::cout << "trigger unlink" << std::endl;
    std::string remove_file = root_dir + request->path();
    fs::path path_rm_file(remove_file);

    response->set_status(1);
    if (fs::exists(path_rm_file)) {
      std::error_code errorCode;
      if (!fs::remove(path_rm_file, errorCode)) {
        perror("Failed to rm file.");
        response->set_status(0);
      }
    } else {
      response->set_status(0);
    }
    return Status::OK;
  }

  Status GetAttr(ServerContext* context, const Path* request,
                 StatInfo* response) override {
    std::cout << "trigger getattr" << std::endl;
    std::string getattr_file = root_dir + request->path();
    fs::path path_getattr_file(getattr_file);

    response->set_status(0);
    if (fs::exists(path_getattr_file)) {
      struct stat sfile;
      stat(getattr_file.c_str(), &sfile);
      response->set_stdev(sfile.st_dev);
      response->set_stino(sfile.st_ino);
      response->set_stmode(sfile.st_mode);
      response->set_stnlink(sfile.st_nlink);
      response->set_stuid(sfile.st_uid);
      response->set_stgid(sfile.st_gid);
      response->set_strdev(sfile.st_rdev);
      response->set_stsize(sfile.st_size);
      response->set_stblksize(sfile.st_blksize);
      response->set_stblocks(sfile.st_blocks);
      response->set_statime(sfile.st_atime);
      response->set_stmtime(sfile.st_mtime);
      response->set_stctime(sfile.st_ctime);
      response->set_status(1);
    }
    return Status::OK;
  }

  Status Read(ServerContext* context, const ReadRequest* request,
              ServerWriter<ReadReply>* writer) override {
    std::cout << "trigger server read" << std::endl;
    int numOfBytes = 0;
    struct timespec spec;

    ReadReply* reply = new ReadReply();
    reply->set_numbytes(numOfBytes);

    int res;
    std::string path = root_dir + request->path();
    printf("ReadFileStream: %s \n", path.c_str());

    int size = request->size();
    int offset = request->offset();

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
      reply->set_err(-errno);
      reply->set_numbytes(INT_MIN);
      return Status::OK;
    }

    std::string buf;
    buf.resize(size);

    int bytesRead = pread(fd, &buf[0], size, offset);
    if (bytesRead != size) {
      printf("Read Send: PREAD didn't read %d bytes from offset %d\n", size,
             offset);
    }

    if (bytesRead == -1) {
      reply->set_err(-errno);
      reply->set_numbytes(INT_MIN);
    }

    int curr = 0;
    while (bytesRead > 0) {
      if (buf.find("crash1") != std::string::npos) {
        std::cout << "Killing server process in read\n";
        kill(getpid(), SIGINT);
      }
      clock_gettime(CLOCK_REALTIME, &spec);
      reply->set_buf(buf.substr(curr, std::min(CHUNK_SIZE, bytesRead)));
      reply->set_numbytes(std::min(CHUNK_SIZE, bytesRead));
      reply->set_err(0);
      reply->set_timestamp(spec.tv_sec);
      curr += std::min(CHUNK_SIZE, bytesRead);
      bytesRead -= std::min(CHUNK_SIZE, bytesRead);

      writer->Write(*reply);
    }

    if (fd > 0) {
      printf("Read Send: Calling close()\n");
      close(fd);
    }
    return Status::OK;
  }

  Status Write(ServerContext* context, ServerReader<WriteRequest>* reader,
               WriteReply* reply) override {
    std::string path;
    WriteRequest request;
    std::string tempFilePath = root_dir;
    int fd = -1;
    int res, size, offset, numOfBytes = 0;
    reply->set_numbytes(numOfBytes);

    while (reader->Read(&request)) {
      path = root_dir + request.path();
      size = request.size();
      offset = request.offset();
      std::string buf = request.buf();
      if (numOfBytes == 0) {
        tempFilePath = path + ".TMP";
        // cout << "Creating new temp file at path = " << tempFilePath << " size
        // = " << size << "\n";
        fd = open(tempFilePath.c_str(), O_CREAT | O_SYNC | O_WRONLY, 0666);
        if (fd == -1) {
          reply->set_err(-errno);
          reply->set_numbytes(INT_MIN);
          return Status::OK;
        }
        // printf("Write Send: %s \n", path.c_str());
      }
      res = pwrite(fd, &buf[0], size, offset);
      fsync(fd);
      // pwrite returns -1 when error, and store type in errno
      if (res == -1) {
        reply->set_err(-errno);
        reply->set_numbytes(INT_MIN);
        printf("Write Send: Pwrite failed!");
        return Status::OK;
      }
      numOfBytes += res;
    }
    if (context->IsCancelled()) {
      fsync(fd);
      close(fd);
      reply->set_err(-errno);
      reply->set_numbytes(INT_MIN);
      return Status::CANCELLED;
    }
    if (fd > 0) {
      if (path.find("crash2") != std::string::npos) {
        // cout << "Killing server process in write()\n";
        kill(getpid(), SIGINT);
      }
      // printf("Write Send: Calling fsync()\n");
      fsync(fd);
      close(fd);
      int res = rename(tempFilePath.c_str(), path.c_str());
      if (res) {
        reply->set_err(-errno);
        reply->set_numbytes(INT_MIN);
      }
    }
    struct stat stbuf;
    int rc = stat(path.c_str(), &stbuf);
    reply->set_timestamp(stbuf.st_mtim.tv_sec);
    reply->set_numbytes(numOfBytes);
    reply->set_err(0);
    return Status::OK;
  }

  // EXAMPLE API
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  AFSServerServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  struct stat info;

  if (!fs::exists(path_root_dir)) {
    if (!fs::create_directories(path_root_dir)) {
      perror("Failed to initialize the root directory.");
    }
  }
  std::cout << "success make the root directory." << std::endl;
  RunServer();

  return 0;
}