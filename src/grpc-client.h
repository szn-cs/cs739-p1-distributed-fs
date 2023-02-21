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
using afs::MkdirRequest;
using afs::MkdirResponse;
using afs::OpenRequest;
using afs::OpenResponse;
using afs::Path;
using afs::ReadReply;
using afs::ReadRequest;
using afs::RedirResponse;
using afs::Response;
using afs::StatInfo;
using afs::WriteReply;
using afs::WriteRequest;

// EXAMPLE API keep it to amke sure thigns are working
using afs::HelloReply, afs::HelloRequest;

#define TIMEOUT 60 * 1000  // this is in ms
#define CHUNK_SIZE 1572864

class AFSClient {
 public:
  AFSClient(std::shared_ptr<Channel> channel);

  int clientMkdir(const std::string& path, mode_t mode, int& errornum);
  int clientRmdir(const std::string& path);
  int clientRedir(const std::string& path, int& errornum, std::vector<std::string>& results);
  int clientUnlink(const std::string& path);
  int clientGetAttr(const std::string& path, struct stat* buf, int& errornum);
  int clientOpen(const std::string& path, const int& mode, long& timestamp);
  int clientRead(const std::string& path, /*const int& size, const int& offset,*/ int& numBytes, std::string& buf, long& timestamp);
  int clientWrite(const std::string& path, const std::string& buf, const int& size, const int& offset, int& numBytes, long& timestamp);

  /** EXAMPLE: keep it to make sure things are working. Assembles the client's payload, sends it and presents the response back from the server. */
  std::string SayHello(const std::string& user);

 private:
  std::unique_ptr<CustomAFS::Stub> stub_;
};
