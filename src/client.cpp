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
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "afs.grpc.pb.h"

using afs::CustomAFS;
using afs::Path;
using afs::Response;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class AFSClient {
public:
  AFSClient(std::shared_ptr<Channel> channel)
      : stub_(CustomAFS::NewStub(channel)) {}

  int Mkdir(const std::string& path) {
    // Follows the same pattern as SayHello.
    Path request;
    request.set_path(path);
    Response reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Mkdir(&context, request, &reply);
    if (status.ok()) {
      // std::cout << status.status() << std::endl;
      return reply.status();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return -1;
    }
  }

  int Rmdir(const std::string& path) {
    Path request;
    request.set_path(path);
    Response reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Rmdir(&context, request, &reply);
    if (status.ok()) {
      // std::cout << status.status() << std::endl;
      return reply.status();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return -1;
    }
  }

  int Unlink(const std::string& path) {
    Path request;
    request.set_path(path);
    Response reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Unlink(&context, request, &reply);
    if (status.ok()) {
      return reply.status();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return -1;
    }
  }

private:
  std::unique_ptr<CustomAFS::Stub> stub_;
};

int main(int, char**) {

  std::string target_str = "localhost:50051";
  AFSClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  // std::string path("/test.txt");
  // int reply = client.Unlink(path);
  // std::cout << "reply: " << reply << std::endl;

  return 0;
}