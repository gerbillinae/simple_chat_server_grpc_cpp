#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "greeter.grpc.pb.h"

void SayHello(std::shared_ptr<grpc::Channel> channel, std::string name) {

    std::unique_ptr<example::Greeter::Stub> stub = example::Greeter::NewStub(channel);

    example::HelloRequest request;
    request.set_name(name);

    example::HelloReply reply;
    grpc::ClientContext context;

    std::cout << "[client] sending HelloRequest with name: " << name << std::endl;
    grpc::Status status = stub->SayHello(&context, request, &reply);

    if (status.ok()) {
        std::cout << "[client] received: " << reply.message() << std::endl;
    } else {
        std::cerr << "[client] RPC failed" << std::endl;
    }
}

int main(int argc, char** argv) {
    // Check if at least one argument is provided (excluding the program name)
    std::string name;
    if (argc < 2) {
        name = "World";
    }
    else
    {
        name = argv[1];
    }

    std::string target_str("localhost:50051");
    std::cout << "[client] connecting to server at " << target_str << std::endl;
    auto channel = grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials());
    SayHello(channel, name);
    return 0;
}