#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <termcolor/termcolor.hpp>
#include <vector>

#include "Utility.cpp"
#include "afs.grpc.pb.h"

#define CHUNK_SIZE 1572864

namespace fs = std::filesystem;
using namespace std;
using namespace grpc;
using namespace afs;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue;

// Logic and data behind the server's behavior.
class GRPC_Server final : public AFS::Service {
 public:
  Status readDirectory(ServerContext* context, const Path* request, ServerWriter<afs::ReadDirResponse>* writer) override;
  Status createDirectory(ServerContext* context, const MkDirRequest* request, MkDirResponse* response) override;
  Status removeDirectory(ServerContext* context, const Path* request, Response* response) override;
  Status removeFile(ServerContext* context, const Path* request, Response* response) override;
  Status getFileAttributes(ServerContext* context, const Path* request, /*char* string*/ Attributes* response) override;
  Status Open(ServerContext* context, const OpenRequest* request, OpenResponse* response) override;
  Status getFileContents(ServerContext* context, const ReadRequest* request, ServerWriter<ReadReply>* writer) override;
  Status Write(ServerContext* context, ServerReader<WriteRequest>* reader, WriteReply* reply) override;
  Status SayHello(ServerContext* context, const HelloRequest* request, HelloReply* reply) override;
};