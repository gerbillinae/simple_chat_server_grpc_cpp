#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "greeter.grpc.pb.h"
#include "utility/utility.h"
#include <chrono>
#include <memory>

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

std::string LoadFile(const std::string & path) {
    // Load the CA certificate file into a string
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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

    grpc::SslCredentialsOptions ssl_opts;

    ssl_opts.pem_root_certs = LoadFile(GetExecutableDir() + "/server.crt");
    auto channel = grpc::CreateChannel("localhost:50051", grpc::SslCredentials(ssl_opts));

    std::unique_ptr<example::Greeter::Stub> stub = example::Greeter::NewStub(channel);
    {
        grpc::ClientContext context;
        example::ListenMessagesRequest request;
        std::unique_ptr<grpc::ClientReader<example::Messages>> reader(stub->ListenMessages(&context, request));

        example::Messages messages;
        while(reader->Read(&messages)){
            for (int i = 0; i < messages.messages_size(); i++) {
                std::cout << messages.messages(i).content() << std::endl;
            }
        }

        grpc::Status status = reader->Finish();
        if (!status.ok()) {
            std::cerr << "Stream ended with error" << status.error_message() << std::endl;
        }
    }

    std::string line;
    while (std::getline(std::cin, line))
    {
        example::Message message;
        message.set_content(line);

        example::MessageResponse reply;

        grpc::ClientContext context;
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(2));

        grpc::Status status = stub->SendMessage(&context, message, &reply);

        if (!status.ok()) {
            std::cerr << "[client] RPC failed" << std::endl;
        }
    }

    return 0;
}