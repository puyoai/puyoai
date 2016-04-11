#ifndef NET_HTTPD_HTTP_HANDLER_H_
#define NET_HTTPD_HTTP_HANDLER_H_

#include <functional>
#include <map>
#include <string>
#include <vector>

class HttpRequest {
public:
    explicit HttpRequest(const char* method) : method_(method) {}

    const std::string& method() const { return method_; }

    std::string* mutableValue(const std::string& key) { return &valueMap_[key]; }
    const std::map<std::string, std::string>& valueMap() const { return valueMap_; }

private:
    std::string method_;
    std::map<std::string, std::string> valueMap_;
};

class HttpResponse {
public:
    HttpResponse() : status_(200) {}

    void setStatus(int status) { status_ = status; }
    void setContent(std::string s) { content_ = std::move(s); }
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

#endif // NET_HTTPD_HTTP_HANDLER_H_
