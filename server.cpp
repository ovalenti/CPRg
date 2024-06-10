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
#include <memory>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

static int connect_to_localhost() {
  int fd = -1;
  int rc;
  struct sockaddr_in sockaddr = { 0 };

  fd = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(50051);
  sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  rc = connect(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
  if (rc < 0) {
    perror("connect");
    close(fd);
    return -1;
  }
  return fd;
}

void RunServer() {
  GreeterServiceImpl service;

  struct grpc::experimental::ExternalConnectionAcceptor::NewConnectionParameters new_connection = { 0 };

  grpc::EnableDefaultHealthCheckService(true);
  //grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  new_connection.fd = connect_to_localhost();
  if (new_connection.fd < 0)
	  return;

  std::unique_ptr<grpc::experimental::ExternalConnectionAcceptor> connection_acceptor =
	  builder.experimental().AddExternalConnectionAcceptor(
			  ServerBuilder::experimental_type::ExternalConnectionType::FROM_FD,
			  grpc::InsecureServerCredentials());

  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  connection_acceptor->HandleNewConnection(&new_connection);

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
