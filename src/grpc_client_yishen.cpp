#include <iostream>
#include <sys/stat.h>
#include <memory>
#include <string>
#include <chrono>
#include <errno.h>
#include <signal.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientWriter;

using afs::CustomAFS;
using afs::Path;
using afs::Response;
using afs::StatInfo;
using afs::ReadFileStreamReply;
using afs::ReadFileStreamReq;
// EXAMPLE API keep it to amke sure thigns are working
using afs::HelloReply;
using afs::HelloRequest;

#define TIMEOUT 60 * 1000 // this is in ms
#define CHUNK_SIZE 1572864

class AFSClient {
public:
  AFSClient(std::shared_ptr<Channel> channel)
      : stub_(CustomAFS::NewStub(channel)) {}

    int clientReadFileStream(const std::string &path, const int &size, const int &offset, int &numBytes, std::string &buf, long &timestamp)
    {
        std::cout << "grpc client read " << path << "\n";
        ReadFileStreamReq request;
        request.set_path(path);
        request.set_size(size);
        request.set_offset(offset);
        std::cout << "1\n";
        ReadFileStreamReply reply;
        ClientContext context;
        std::chrono::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
        context.set_deadline(deadline);
        std::cout << "1-1\n";
        std::unique_ptr<ClientReader<ReadFileStreamReply>> reader(stub_->ReadFileStream(&context, request));
        std::cout << "2\n";
        while (reader->Read(&reply))
        {   
            std::cout << "3\n";
            if (reply.buf().find("crash3") != std::string::npos)
            {
                std::cout << "Killing client process in read()\n";
                kill(getpid(), SIGABRT);
            }
            std::cout << reply.buf() << std::endl;
            buf.append(reply.buf());
            if (reply.numbytes() < 0)
            {   
                std::cout << "4\n";
                break;
            }
        }
        Status status = reader->Finish();
        if (status.ok())
        {
            numBytes = reply.numbytes();
            timestamp = reply.timestamp();
            std::cout << "grpc Read client " << numBytes << " " << timestamp << std::endl;
            return reply.err();
        }
        std::cout << "There was an error in the server Read " << status.error_code() << std::endl;
        return status.error_code();
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
    // ---------- test read ----------
    AFSClient* client_read;
    std::string buf; 
    long timestamp;
    int numBytes;
    client_read->clientReadFileStream("test.txt", 8, 0, numBytes, buf, timestamp);
  return 0;
}