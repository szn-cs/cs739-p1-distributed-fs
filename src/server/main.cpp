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

// todo

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <memory>
#include <string>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "afs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using afs::CustomAFS;
using afs::Path;
using afs::Response;
// Logic and data behind the server's behavior.
class AFSServerServiceImpl final : public CustomAFS::Service {
  Status Mkdir(ServerContext* context, const Path* request,
                  Response* response) override {
    
    std::cout << "trigger mkdir" << std::endl;
    std::string new_directory_path = request->path();
    std::string absolute_dir = fs::current_path();
    absolute_dir += "/root_dir";
    absolute_dir += new_directory_path;
    fs::path path_new_dir(absolute_dir);

    response->set_status(1);
    if (!fs::exists(path_new_dir)){
      if (!fs::create_directories(path_new_dir)) {
          perror("Failed to initialize the root directory.");
          response->set_status(0);
      }
      
    } else {
      response->set_status(0);
    }
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
  std::string root_dir = fs::current_path();
  root_dir += "/root_dir";
  std::cout << root_dir << std::endl;
  fs::path path_root_dir(root_dir);
  
  if (!fs::exists(path_root_dir)){
    if (!fs::create_directories(path_root_dir)) {
        perror("Failed to initialize the root directory.");
    }
  }
  std::cout << "success make the root directory." << std::endl;
  RunServer();

  return 0;
}
