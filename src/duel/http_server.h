#ifndef DUEL_HTTPSERVER_H_
#define DUEL_HTTPSERVER_H_

#include <memory>
#include <string>
#include <unordered_map>

class HttpRequest {
};

class HttpResponse {
public:
    HttpResponse() : status_(200) {}

    void setStatus(int status) { status_ = status; }
    void setContent(const std::string& s) { content_ = s; }

    int status() const { return status_; }
    // TODO(mayah): Do refactoring.
    void* content() const { return const_cast<void*>(static_cast<const void*>(content_.c_str())); }
    size_t contentSize() const { return content_.size(); }
private:
    int status_;
    std::string content_;
};

class HttpHandler {
public:
    virtual ~HttpHandler() {};
    virtual void handle(HttpRequest*, HttpResponse*) = 0;
};

class HttpServer {
public:
    static const int kPort = 8000;

    HttpServer();
    ~HttpServer();

    bool start();
    void stop();

    // Don't take ownership of HttpHandler. HttpHandler should be alive during HttpServer is alive.
    void installHandler(const std::string& path, HttpHandler*);

private:
    static int accessHandler(void* cls, struct MHD_Connection* connection,
                             const char* url, const char* method, const char* version,
                             const char* upload_data, size_t* upload_data_size, void** con_cls);

    struct MHD_Daemon* httpd_;
    std::unordered_map<std::string, HttpHandler*> handlers_;
};

#endif
