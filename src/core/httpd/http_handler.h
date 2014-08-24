#ifndef CORE_HTTPD_HTTP_HANDLER_H_
#define CORE_HTTPD_HTTP_HANDLER_H_

#include <functional>
#include <string>

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

typedef std::function<void (const HttpRequest*, HttpResponse*)> HttpHandler;

#endif
