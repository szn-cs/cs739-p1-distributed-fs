
#include "./grpc-client.h"
using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan;

GRPC_Client::GRPC_Client(std::shared_ptr<Channel> channel) : stub_(WiscAFS::NewStub(channel)) {}

int GRPC_Client::getFileAttributes(const std::string& path, struct stat* buf, int& errornum, int& logical_clock) {
  //std::cout << yellow << "GRPC_Client::getFileAttributes" << reset << std::endl;

  Attributes reply;
  ClientContext context;
  Path request;
  request.set_path(path);
  Status status = stub_->getFileAttributes(&context, request, &reply);

  if (!status.ok()) {
    //std::cout << red << "GRPC error @getFileAttributes: " << status.error_code() << ": " << status.error_message() << reset << std::endl;
    return -1;
  }

  if (reply.status() != 0) {
    errornum = reply.errornum();
    return -1;
  }

  // retrieve status info from grpc message
  buf->st_dev = reply.grpc_st_dev();
  buf->st_ino = reply.grpc_st_ino();
  buf->st_mode = reply.grpc_st_mode();
  buf->st_nlink = reply.grpc_st_nlink();
  buf->st_uid = reply.grpc_st_uid();
  buf->st_gid = reply.grpc_st_gid();
  buf->st_rdev = reply.grpc_st_rdev();
  buf->st_size = reply.grpc_st_size();
  buf->st_blksize = reply.grpc_st_blksize();
  buf->st_blocks = reply.grpc_st_blocks();
  buf->st_atime = reply.grpc_st_atime();
  buf->st_mtime = reply.grpc_st_mtime();
  buf->st_ctime = reply.grpc_st_ctime();
  logical_clock = reply.logical_clock();

  return 0;
}

int GRPC_Client::getFileContents(const std::string& path, /*const int& size,const int& offset,*/ int& numBytes, std::string& buf, long& timestamp) {
  //std::cout << yellow << "GRPC_Client::getFileContents" << reset << std::endl;
  ReadRequest request;
  ReadReply reply;
  ClientContext context;

  request.set_path(path);
  // request.set_size(size);
  // request.set_offset(offset);

  std::chrono::time_point<std::chrono::system_clock> deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  context.set_deadline(deadline);

  std::unique_ptr<ClientReader<ReadReply>> reader(stub_->getFileContents(&context, request));  // contact server

  while (reader->Read(&reply)) {
    // if (reply.buf().find("crash3") != std::string::npos) {
    //   //std::cout << "Killing client process in read()\n";
    //   kill(getpid(), SIGABRT);
    // }
    // //std::cout << reply.buf() << std::endl;
    buf.append(reply.buf());
    if (reply.numbytes() < 0)
      break;
  }

  Status status = reader->Finish();
  if (!status.ok()) {
    //std::cout << red << "GRPC error @getFileContents: " << status.error_code() << ": " << status.error_message() << reset << std::endl;
    return -1;
  }

  if (reply.err() != 0)
    return reply.err();

  numBytes = reply.numbytes();
  timestamp = reply.timestamp();
  //std::cout << "grpc getFileContents client complete " << numBytes << " " << timestamp << std::endl;

  return 0;
}

int GRPC_Client::readDirectory(const std::string& path, int& errornum, std::vector<std::string>& results) {
  //std::cout << yellow << "GRPC_Client::readDirectory" << reset << std::endl;
  // return cppWrapper_readdir(path, buf, filler, offset, fi);
  // const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi
  Path request;
  afs::ReadDirResponse response;
  ClientContext context;
  request.set_path(path);
  std::unique_ptr<ClientReader<afs::ReadDirResponse>> reader(stub_->readDirectory(&context, request));

  while (reader->Read(&response)) {
    results.push_back(response.buf());
    if (response.err() < 0) {
      break;
    }
  }

  Status status = reader->Finish();

  return status.ok() ? 0 : status.error_code();
  /*
  Status status = stub_->readDirectory(&context, request, &response);
  if (status.ok()) {
    // -----------------after receive repeated de from grpc server, store them into result_ptr
    for (int i = 0; i < response.buf_size(); ++i) {
      results.push_back(response.buf(i));
    }
    return 0;
  } else {
    errornum = -1;
    //std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return -1;
  }
  */
}

int GRPC_Client::createDirectory(const std::string& path, mode_t mode, int& errornum) {
  //std::cout << yellow << "GRPC_Client::createDirectory" << reset << std::endl;
  MkDirRequest request;
  Response reply;
  ClientContext context;
  //std::cout << path << std::endl;
  request.set_path(path);
  request.set_modet(mode);

  Status status = stub_->createDirectory(&context, request, &reply);

  if (!status.ok()) {
    errornum = -1;
    //std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    return -1;
  }

  if (reply.status() != 0)
    errornum = reply.erronum();

  return reply.status();
}

int GRPC_Client::removeDirectory(const std::string& path) {
  //std::cout << yellow << "GRPC_Client::removeDirectory" << reset << std::endl;
  Path request;
  Response reply;
  ClientContext context;

  request.set_path(path);

  Status status = stub_->removeDirectory(&context, request, &reply);
  if (!status.ok()) {
    //std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    return -1;
  }

  if (reply.status() != 0)
    return reply.erronum();

  return 0;
}

int GRPC_Client::removeFile(const std::string& path) {
  //std::cout << yellow << "GRPC_Client::removeFile" << reset << std::endl;
  Path request;
  Response reply;
  ClientContext context;

  request.set_path(path);

  // Here we can use the stub's newly available method we just added.
  Status status = stub_->removeFile(&context, request, &reply);

  if (!status.ok()) {
    //std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    return -1;
  }

  if (reply.status() != 0)
    return reply.erronum();

  return 0;
}

// HOW to call:
//   ret = grpcClient->createEmptyFile(_path, O_RDWR | O_CREAT | S_IRWXU, timestamp);
//    if (ret != 0) return ret;
// TODO: ? touch functionality: only create file without contents
int GRPC_Client::createEmptyFile(const std::string& path, const int& mode, long& timestamp) {
  //std::cout << yellow << "GRPC_Client::createEmptyFile" << reset << std::endl;
  OpenRequest request;
  request.set_path(path);
  request.set_mode(mode);
  OpenResponse reply;
  ClientContext context;
  Status status = stub_->createEmptyFile(&context, request, &reply);
  if (status.ok()) {
    timestamp = reply.timestamp();
    //std::cout << yellow << "reply err" << reply.err() << reset << std::endl;
    return reply.err();
  }
  // grpc fail
  return -1;
  // return status.error_code();
}

int GRPC_Client::putFileContents(const std::string& path, const std::string& buf, const int& size, const int& offset, int& numBytes, int& logical_clock) {
  //std::cout << yellow << "GRPC_Client::putFileContents" << reset << std::endl;
  //std::cout << "GRPC client write\n";
  WriteRequest request;
  WriteReply reply;
  ClientContext context;
  std::chrono::time_point<std::chrono::system_clock> deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(TIMEOUT);
  context.set_deadline(deadline);
  std::unique_ptr<ClientWriter<WriteRequest>> writer(stub_->putFileContents(&context, &reply));
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
  }
  writer->WritesDone();
  Status status = writer->Finish();

  // //std::cout << "There was an error in the server Write " << status.error_code() << std::endl;
  if (status.ok()) {
    numBytes = reply.numbytes();
    logical_clock = reply.logical_clock();
    return reply.err();
  }
  // cout << "There was an error in the server Write " << status.error_code() << endl;

  return status.error_code();
}

/** EXAMPLE: keep it to make sure things are working - Assembles the client's payload, sends it and presents the response back from the server. */
std::string GRPC_Client::SayHello(const std::string& user) {
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
    //std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    return "RPC failed";
  }
}
