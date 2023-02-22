#include "./grpc-server.h"

namespace fs = std::filesystem;
using namespace std;
using namespace afs;

using afs::AFS;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;

using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue;

static std::string rootDirectory;

Status AFS_Server::ReadDir(ServerContext* context, const Path* request, ServerWriter<afs::ReadDirResponse>* writer) {
  std::cout << "trigger redir" << std::endl;

  string path = Utility::concatenatePath(rootDirectory, request->path());

  std::vector<std::string> alldata;
  struct dirent* de;
  DIR* dp = opendir(path.c_str());
  if (dp == NULL) {
    std::cout << "DIR is null in server redir function" << std::endl;
    return Status::OK;
  }

  while ((de = readdir(dp)) != NULL) {
    std::string data;
    data.resize(sizeof(struct dirent));
    memcpy(&data[0], de, sizeof(struct dirent));
    std::cout << "data: " << data << std::endl;
    alldata.push_back(data);
    /*
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;
    if (filler(buf, de->d_name, &st, 0)) break;
    */
  }
  closedir(dp);

  ReadDirResponse* response = new ReadDirResponse();

  for (auto entry : alldata) {
    response->set_buf(entry);
    writer->Write(*response);
  }

  // (void)offset;
  // (void)fi;
  // response->mutable_buf()->Add(alldata.begin(), alldata.end());
  return Status::OK;
}

Status AFS_Server::MkDir(ServerContext* context, const MkDirRequest* request, MkDirResponse* response) {
  std::cout << "trigger mkdir" << std::endl;

  string path = Utility::concatenatePath(rootDirectory, request->path());

  mode_t mode = (mode_t)request->modet();

  int ret = mkdir(path.c_str(), mode);
  if (ret != 0) {
    response->set_erronum(errno);
  }
  response->set_status(ret);
  return Status::OK;
}

Status AFS_Server::RmDir(ServerContext* context, const Path* request, Response* response) {
  std::cout << "trigger rmdir" << std::endl;

  string path = Utility::concatenatePath(rootDirectory, request->path());

  response->set_status(1);

  if (fs::exists(path)) {
    std::error_code errorCode;
    if (!fs::remove(path, errorCode)) {
      perror("Failed to rm directory.");
      response->set_status(0);
    }
  } else {
    response->set_status(0);
  }
  return Status::OK;
}

Status AFS_Server::Unlink(ServerContext* context, const Path* request, Response* response) {
  std::cout << "trigger unlink" << std::endl;

  string path = Utility::concatenatePath(rootDirectory, request->path());

  response->set_status(1);
  if (fs::exists(path)) {
    std::error_code errorCode;
    if (!fs::remove(path, errorCode)) {
      perror("Failed to rm file.");
      response->set_status(0);
    }
  } else {
    response->set_status(0);
  }
  return Status::OK;
}

Status AFS_Server::GetAttr(ServerContext* context, const Path* request, /*char* string*/ StatInfo* response) {
  std::cout << yellow << "AFS_Server::GetAttr" << reset << std::endl;

  string path = Utility::concatenatePath(rootDirectory, request->path());

  struct stat sfile;
  if (lstat(path.c_str(), &sfile) != -1) {
    cout << "⚫ lstat found" << path << endl;

    response->set_status(0);
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
  } else {
    cout << "⚫ lstat not found: " << path << endl;

    response->set_status(-1);
    response->set_errornum(errno);
  }

  // type cast from lstat to string and backagain

  return Status::OK;
}

Status AFS_Server::Open(ServerContext* context, const OpenRequest* request, OpenResponse* response) {
  std::cout << "trigger server open" << std::endl;
  int rc;

  string path = Utility::concatenatePath(rootDirectory, request->path());

  // rc = creat(path.c_str(), request->mode());

  rc = open(path.c_str(), request->mode(), S_IRWXG | S_IRWXO | S_IRWXU);
  if (rc == -1) {
    // cout << "create returned wrong value\n";
    response->set_err(-errno);
    return Status::OK;
  }
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  response->set_timestamp(spec.tv_sec);
  response->set_err(0);
  return Status::OK;
}

Status AFS_Server::Read(ServerContext* context, const ReadRequest* request, ServerWriter<ReadReply>* writer) {
  std::cout << "trigger server read" << std::endl;
  int numOfBytes = 0;
  int res;
  struct timespec spec;
  ReadReply* reply = new ReadReply();

  string path = Utility::concatenatePath(rootDirectory, request->path());

  std::cout << "ReadFileStream: " << path << std::endl;

  std::ifstream is;
  is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    // read data to buffer with offset and size
    is.open(path, std::ios::binary | std::ios::ate);
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    /*
    if (offset + size > is.tellg()) {
      std::cout << "The offset + size is greater then the file size.\n"
                << "file size: " << is.tellg() << "\n"
                << "offset: " << offset << "\n"
                << "size: " << size << std::endl;
    }
    */
    // is.seekg(offset, std::ios::beg);
    if (length == 0) {
      clock_gettime(CLOCK_REALTIME, &spec);
      reply->set_buf("");
      reply->set_numbytes(0);
      reply->set_err(0);
      reply->set_timestamp(spec.tv_sec);
      writer->Write(*reply);
      std::cout << "File is empty." << std::endl;
      is.close();
      return Status::OK;
    }

    std::string buffer(length, '\0');
    is.read(&buffer[0], length);
    std::cout << "buffer: " << buffer << std::endl;
    // send data chunk to client
    int bytesRead = 0;
    int minSize = std::min(CHUNK_SIZE, length);

    while (bytesRead < is.tellg()) {
      clock_gettime(CLOCK_REALTIME, &spec);
      std::string subBuffer = buffer.substr(bytesRead, minSize);
      // if (subBuffer.find("SERVER_READ_CRASH") != std::string::npos) {
      //   std::cout << "Killing server process in read\n";
      //   kill(getpid(), SIGINT);
      // }
      reply->set_buf(subBuffer);
      reply->set_numbytes(minSize);
      reply->set_err(0);
      reply->set_timestamp(spec.tv_sec);
      bytesRead += minSize;
      writer->Write(*reply);
    }

    is.close();
  } catch (std::ifstream::failure e) {
    reply->set_buf("");
    reply->set_numbytes(0);
    reply->set_err(-1);
    reply->set_timestamp(-1);
    writer->Write(*reply);
    std::cout << "Caught a failure when read.\n"
              << "Explanatory string: " << e.what() << '\n'
              << "Error code: " << e.code() << std::endl;
    is.close();
  }
  return Status::OK;
  /*
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
  */
}

Status AFS_Server::Write(ServerContext* context, ServerReader<WriteRequest>* reader, WriteReply* reply) {
  std::cout << "trigger server write" << std::endl;
  std::string path;
  WriteRequest request;
  std::string tempFilePath = rootDirectory;
  int fd = -1;
  int res, size, offset, numOfBytes = 0;
  reply->set_numbytes(numOfBytes);

  while (reader->Read(&request)) {
    path = Utility::concatenatePath(rootDirectory, request.path());

    size = request.size();
    offset = request.offset();
    std::string buf = request.buf();
    if (numOfBytes == 0) {
      tempFilePath = path + ".TMP";
      // cout << "Creating new temp file at path = " << tempFilePath << " size
      // = " << size << "\n";
      fd = open(tempFilePath.c_str(), O_CREAT | O_SYNC | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
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
Status AFS_Server::SayHello(ServerContext* context, const HelloRequest* request, HelloReply* reply) {
  std::string prefix("Hello ");
  reply->set_message(prefix + request->name());
  return Status::OK;
}

void RunServer(std::string address) {
  AFS_Server service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << termcolor::yellow << "⚡ Server listening on " << address << termcolor::reset << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  struct stat info;

  // set defaults
  const std::string address("0.0.0.0:50051");
  rootDirectory = fs::current_path().generic_string() + "/tmp/fsServer/";

  // set configs from arguments
  if (argc == 2)
    rootDirectory = argv[1];

  fs::path _rootDirectory(rootDirectory);
  if (!fs::exists(_rootDirectory))
    if (!fs::create_directories(_rootDirectory))
      perror("Failed to create the root directory.");

  std::cout << "Created root directory at: " << fs::absolute(_rootDirectory) << std::endl;
  RunServer(address);

  return 0;
}