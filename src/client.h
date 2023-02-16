#pragma once

#include <errno.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;

using afs::CustomAFS;
using afs::Path;
using afs::ReadReply;
using afs::ReadRequest;
using afs::Response;
using afs::StatInfo;
using afs::WriteReply;
using afs::WriteRequest;
// EXAMPLE API keep it to amke sure thigns are working
using afs::HelloReply;
using afs::HelloRequest;

#define TIMEOUT 60 * 1000 // this is in ms
#define CHUNK_SIZE 1572864

class AFSClient {
public:
  AFSClient(std::shared_ptr<Channel> channel);

  int Mkdir(const std::string& path);

  int Rmdir(const std::string& path);

  int Unlink(const std::string& path);

  int GetAttr(const std::string& path);

  int clientReadFileStream(const std::string& path, const int& size,
                           const int& offset, int& numBytes, std::string& buf,
                           long& timestamp);

  int clientWriteFileStream(const std::string& path, const std::string& buf,
                            const int& size, const int& offset, int& numBytes,
                            long& timestamp);

  /** EXAMPLE: keep it to make sure things are working
   * Assembles the client's payload, sends it and presents the response back
   * from the server.
   */
  std::string SayHello(const std::string& user);

private:
  std::unique_ptr<CustomAFS::Stub> stub_;
};
