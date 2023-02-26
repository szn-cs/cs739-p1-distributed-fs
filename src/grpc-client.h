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
#include <termcolor/termcolor.hpp>

#include "afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;

using namespace afs;
using afs::WiscAFS;

// EXAMPLE API keep it to amke sure thigns are working
using afs::HelloReply, afs::HelloRequest;

#define TIMEOUT 60 * 1000  // this is in ms
#define CHUNK_SIZE 1572864

class GRPC_Client {
 public:
  GRPC_Client(std::shared_ptr<Channel> channel);

  int createDirectory(const std::string& path, mode_t mode, int& errornum);
  int removeDirectory(const std::string& path);
  int readDirectory(const std::string& path, int& errornum, std::vector<std::string>& results);
  int removeFile(const std::string& path);
  int getFileAttributes(const std::string& path, struct stat* buf, int& errornum, int& logical_clock);
  // TODO: replace with Fetch & Store
  int createEmptyFile(const std::string& path, const int& mode, long& timestamp);
  int getFileContents(const std::string& path, /*const int& size, const int& offset,*/ int& numBytes, std::string& buf, long& timestamp);
  int putFileContents(const std::string& path, const std::string& buf, const int& size, const int& offset, int& numBytes, int& logical_clock);

  /** EXAMPLE: keep it to make sure things are working. Assembles the client's payload, sends it and presents the response back from the server. */
  std::string SayHello(const std::string& user);

 private:
  std::unique_ptr<WiscAFS::Stub> stub_;
};
