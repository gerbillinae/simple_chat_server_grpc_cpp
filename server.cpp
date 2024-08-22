#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "greeter.grpc.pb.h"
#include <grpcpp/security/server_credentials.h>
#include "utility/utility.h"
#include <sqlite3.h>
#include <mutex>
#include <format>

class Database {
    std::mutex m_mutex{};
    sqlite3 *DB = nullptr;

    std::vector<std::string> m_messages{};
public:
    const std::vector<std::string> & getMessages();

private:
    Database(std::string db_path) {
        if (DB != nullptr)
            return;

        int exit = sqlite3_open(db_path.c_str(), &DB);

        if (exit) {
            std::cerr << "Error opening database: " << sqlite3_errmsg(DB) << std::endl;
            return;
        } else {
            std::cout << "Opened database successfully!" << std::endl;
        }

        const char *sql = "CREATE TABLE IF NOT EXISTS MESSAGE("
                          "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "MESSAGE TEXT);";

        char *errMsg = 0;
        exit = sqlite3_exec(DB, sql, 0, 0, &errMsg);

        if (exit != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } else {
            std::cout << "Table created successfully!" << std::endl;
        }

        return;
    }

public:
    static Database& GetInstance() {
        static Database database(GetExecutableDir() + "/example.db");
        return database;
    }

    int saveMessage(std::string message) {

        m_messages.push_back(message);

        const char * sql =  "INSERT INTO MESSAGE (MESSAGE) VALUES (?);";

        sqlite3_stmt* stmt;
        auto rc = sqlite3_prepare_v2(DB, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB) << std::endl;
            return 1;
        }

        // Bind the parameter
        // Forth parameter indicates object lifetime. Use SQLITE_STATIC for static data.
        sqlite3_bind_text(stmt, 1, message.c_str(), -1, SQLITE_TRANSIENT);

        // Execute the query
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(DB) << std::endl;
        } else {
            std::cout << "Insert successful" << std::endl;
        }

        getMessages();
        return 0;
    }


    ~Database()
    {
        if (DB)
            sqlite3_close(DB);
    }
};

const std::vector<std::string> & Database::getMessages()
{
    if (!m_messages.empty())
        return m_messages;

    std::string sql = "SELECT (MESSAGE) FROM MESSAGE;";

    std::vector<std::string> messages{};

    char *errMsg = 0;
    auto callback = [](void* data, int argc, char** argv, char** azColName) {
        auto* messages = static_cast<std::vector<std::string>*>(data);
        std::cout << "Row:" << std::endl;
        for (int i = 0; i < argc; i++) {
            // argv[i] contains the value of the column, azColName[i] contains the column name
            std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << std::endl;
            messages->push_back(argv[i]);
        }
        std::cout << std::endl;
        return 0;
    };

    int exit = sqlite3_exec(DB, sql.c_str(), callback, &messages, &errMsg);

    if (exit != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Table created successfully!" << std::endl;
    }

    m_messages = messages;
    return m_messages;
}



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

    grpc::Status SendMessage(grpc::ServerContext* context, const example::Message* request,
                          example::MessageResponse* reply) override {

        auto& content = request->content();
        Database::GetInstance().saveMessage(request->content());
        reply->set_error(0);
        return grpc::Status::OK;
    }

    grpc::Status ListenMessages(grpc::ServerContext* context, const example::ListenMessagesRequest* request,
                                grpc::ServerWriter<example::Messages>* writer) override {

        example::Messages response;
        const auto & messages = Database::GetInstance().getMessages();
        for (const auto & msg : messages)
            response.add_messages()->set_content(msg);
        writer->Write(response);

        return grpc::Status::OK;
    }
};

std::string LoadFileIntoString(const std::string & path) {
    // Load the CA certificate file into a string
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Create server credentials. Client certificate is not required.
std::shared_ptr<grpc::ServerCredentials> CreateServerCredentials(std::string serverKeyPath, std::string serverCrtPath)
{
    grpc::SslServerCredentialsOptions ssl_opts;

    // Provide a cert and key for TLS
    grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert = {LoadFileIntoString(serverKeyPath), LoadFileIntoString(serverCrtPath)};
    ssl_opts.pem_key_cert_pairs.push_back(key_cert);

    // Do not require client-side authentication
    ssl_opts.force_client_auth = false;
    ssl_opts.client_certificate_request = GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE;

    return grpc::SslServerCredentials(ssl_opts);
}

void RunServer() {

    std::string server_address("0.0.0.0:50051");
    std::cout << "creating server credentials" << std::endl;
    // Create server tls credentials
    std::string exeDir = GetExecutableDir();

    auto & db = Database::GetInstance();

    auto messages = db.getMessages();
    for (auto & msg : messages)
        std::cout << msg << std::endl;

    std::string certfile = exeDir + "/server.crt";
    std::string pemfile = exeDir + "/server.key";

    auto creds = CreateServerCredentials(pemfile, certfile);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, creds);

    GreeterServiceImpl service;
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "[server] listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}