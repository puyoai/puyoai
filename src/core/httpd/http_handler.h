#ifndef CORE_HTTPD_HTTP_HANDLER_H_
#define CORE_HTTPD_HTTP_HANDLER_H_

#include <functional>
#include <string>
#include <vector>

class HttpRequest {
};

class HttpResponse {
public:
    HttpResponse() : status_(200) {}

    void setStatus(int status) { status_ = status; }
    void setContent(const std::string& s) { content_ = s; }
    void addHeader(const std::string& key, const std::string& value) { header_.emplace_back(key, value); }

    int status() const { return status_; }
    const std::vector<std::pair<std::string, std::string>>& headers() const { return header_; }

    // TODO(mayah): Do refactoring.
    void* content() const { return const_cast<void*>(static_cast<const void*>(content_.c_str())); }
    size_t contentSize() const { return content_.size(); }

private:
    int status_;
    std::string content_;
    std::vector<std::pair<std::string, std::string>> header_;
};

typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpHandler;

#endif
