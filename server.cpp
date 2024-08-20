#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "greeter.grpc.pb.h"

// GreeterService implementation
class GreeterServiceImpl final : public example::Greeter::Service {
public:
    grpc::Status SayHello(grpc::ServerContext* context, const example::HelloRequest* request,
                          example::HelloReply* reply) override {

        auto name = request->name();
        std::cout << "[server] received HelloRequest with name: " << name << std::endl;

        std::string prefix("Hello ");
        reply->set_message(prefix + name);
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");

    GreeterServiceImpl service;

    grpc::ServerBuilder builder;

    // InsecureServerCredentials() creates server credentials that do not use encryption or authentication.
    // It is used during development to allow the gRPC server to accept connections without securing them with SSL/TLS.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "[server] listening on " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}