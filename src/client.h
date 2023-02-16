#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "afs.grpc.pb.h"

using afs::CustomAFS;
using grpc::Channel;

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
