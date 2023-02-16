
#include "./grpc-client.h"

AFSClient::AFSClient(std::shared_ptr<Channel> channel)
    : stub_(CustomAFS::NewStub(channel)) {}

int AFSClient::clientMkdir(const std::string& path) {
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

int AFSClient::clientRmdir(const std::string& path) {
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

int AFSClient::clientUnlink(const std::string& path) {
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

int AFSClient::clientGetAttr(const std::string& path) {
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

int AFSClient::clientRead(const std::string& path, const int& size,
                          const int& offset, int& numBytes, std::string& buf,
                          long& timestamp) {
  std::cout << "trigger grpc client read on path: " << path << "\n";
  ReadRequest request;
  request.set_path(path);
  request.set_size(size);
  request.set_offset(offset);
  std::cout << "1\n";
  ReadReply reply;
  ClientContext context;
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  context.set_deadline(deadline);

  std::unique_ptr<ClientReader<ReadReply>> reader(
      stub_->Read(&context, request));

  while (reader->Read(&reply)) {
    if (reply.buf().find("crash3") != std::string::npos) {
      std::cout << "Killing client process in read()\n";
      kill(getpid(), SIGABRT);
    }
    std::cout << reply.buf() << std::endl;
    buf.append(reply.buf());
    if (reply.numbytes() < 0) {
      break;
    }
  }
  Status status = reader->Finish();
  if (status.ok()) {
    numBytes = reply.numbytes();
    timestamp = reply.timestamp();
    std::cout << "grpc Read client " << numBytes << " " << timestamp
              << std::endl;
    return reply.err();
  }
  std::cout << "There was an error in the server Read " << status.error_code()
            << std::endl;
  return status.error_code();
}

int AFSClient::clientWrite(const std::string& path, const std::string& buf,
                           const int& size, const int& offset, int& numBytes,
                           long& timestamp) {
  std::cout << "GRPC client write\n";
  WriteRequest request;
  WriteReply reply;
  ClientContext context;
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  context.set_deadline(deadline);
  std::unique_ptr<ClientWriter<WriteRequest>> writer(
      stub_->Write(&context, &reply));
  int bytesLeft = size;
  int curr = offset;
  while (bytesLeft >= 0) {
    request.set_path(path);
    request.set_buf(buf.substr(curr, std::min(CHUNK_SIZE, bytesLeft)));
    request.set_size(std::min(CHUNK_SIZE, bytesLeft));
    request.set_offset(curr);
    curr += CHUNK_SIZE;
    bytesLeft -= CHUNK_SIZE;
    if (!writer->Write(request)) {
      // Broken stream.
      break;
    }
    if (buf.find("crash4") != std::string::npos) {
      std::cout << "Killing client process in write()\n";
      kill(getpid(), SIGABRT);
    }
  }
  writer->WritesDone();
  Status status = writer->Finish();

  // std::cout << "There was an error in the server Write "
  //           << status.error_code() << std::endl;

  if (status.ok()) {
    numBytes = reply.numbytes();
    timestamp = reply.timestamp();
    return reply.err();
  }
  // cout << "There was an error in the server Write " << status.error_code()
  // << endl;

  return status.error_code();
}

/** EXAMPLE: keep it to make sure things are working
 * Assembles the client's payload, sends it and presents the response back
 * from the server.
 */
std::string AFSClient::SayHello(const std::string& user) {
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
