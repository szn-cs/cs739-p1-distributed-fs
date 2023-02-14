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

#include <iostream>
#include <sys/stat.h>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using afs::CustomAFS;
using afs::Path;
using afs::Response;
using afs::StatInfo;
// EXAMPLE API keep it to amke sure thigns are working
using afs::HelloReply;
using afs::HelloRequest;

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

  int GetAttr(const std::string& path) {
    Path request;
    request.set_path(path);
    StatInfo reply;
    ClientContext context;

    Status status = stub_->GetAttr(&context, request, &reply);
    if (status.ok()) {
      std::cout << reply.stdev() << std::endl;
      std::cout << reply.stino() << std::endl;
      std::cout << reply.stmode() << std::endl;
      std::cout << reply.stnlink() << std::endl;
      std::cout << reply.stuid() << std::endl;
      std::cout << reply.stgid() << std::endl;
      std::cout << reply.strdev() << std::endl;
      std::cout << reply.stsize() << std::endl;
      std::cout << reply.stblksize() << std::endl;
      std::cout << reply.stblocks() << std::endl;
      std::cout << reply.statime() << std::endl;
      std::cout << reply.stmtime() << std::endl;
      std::cout << reply.stctime() << std::endl;
      return reply.status();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return -1;
    }
  }

  /** EXAMPLE: keep it to make sure things are working
   * Assembles the client's payload, sends it and presents the response back
   * from the server.
   */
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<CustomAFS::Stub> stub_;
};

int main(int argc, char* argv[]) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        target_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --target="
                  << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  } else {
    target_str = "localhost:50051";
  }

  AFSClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  // EXAMPLE: keep it to make sure things are working
  std::string user("world");
  std::string reply = client.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  // std::string path("/test.txt");
  // int reply = client.Unlink(path);
  // std::cout << "reply: " << reply << std::endl;

  return 0;
}