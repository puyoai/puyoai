#include "net/httpd/http_server.h"

#include <cstdlib>

#include <string>
#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "base/file/file.h"
#include "base/file/path.h"
#include "base/strings.h"

using namespace std;

const int POST_BUFFER_SIZE = 4096;

class HttpExchange {
public:
    explicit HttpExchange(const char* method) :
        isPost_(strcmp(method, "POST") == 0),
        postProcessor_(nullptr, MHD_destroy_post_processor),
        request_(method) {}
    ~HttpExchange() {}

    void setPostProcessor(unique_ptr<MHD_PostProcessor, decltype(&MHD_destroy_post_processor)> postProcessor)
    {
        postProcessor_ = std::move(postProcessor);
    }
    MHD_PostProcessor* postProcessor() { return postProcessor_.get(); }

    bool isPost() const { return isPost_; }

    const HttpRequest& request() const { return request_; }
    HttpRequest* mutableRequest() { return &request_; }
    HttpResponse* mutableResponse() { return &response_; }

private:
    bool isPost_;
    unique_ptr<MHD_PostProcessor, decltype(&MHD_destroy_post_processor)> postProcessor_;

    HttpRequest request_;
    HttpResponse response_;
};

static int notFoundHandler(MHD_Connection* connection)
{
    const char *page = "<html><head><title>404 NOT FOUND</title></head><body>404 NOT FOUND</body></html>";
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}

static int assetsHandler(MHD_Connection* connection, const std::string& filename)
{
    string s;
    if (!file::readFile(filename, &s)) {
        LOG(INFO) << "assetsHandler " << filename << " does not exist";
        return notFoundHandler(connection);
    }

    struct MHD_Response* response = MHD_create_response_from_buffer(s.size(), (void *)s.c_str(), MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static int handleHandler(MHD_Connection* connection, const HttpHandler& handler, HttpExchange* exchange)
{
    HttpResponse* resp = exchange->mutableResponse();
    handler(exchange->request(), resp);

    struct MHD_Response* response = MHD_create_response_from_buffer(
        resp->contentSize(), resp->content(), MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, resp->status(), response);
    for (const auto& header : resp->headers())
        MHD_add_response_header(response, header.first.c_str(), header.second.c_str());
    MHD_destroy_response(response);
    return ret;
}

static int iterate_post(void* con_cls, enum MHD_ValueKind /*kind*/, const char* key,
                        const char* /*filename*/, const char* /*content_type*/, const char* /*transfer_encoding*/,
                        const char* data, uint64_t /*off*/, size_t size)
{
    HttpExchange* exchange = reinterpret_cast<HttpExchange*>(con_cls);

    HttpRequest* req = exchange->mutableRequest();
    std::string* value = req->mutableValue(key);
    *value += std::string(data, size);

    return MHD_YES;
}

// static
int HttpServer::accessHandler(void* cls, struct MHD_Connection* connection,
                              const char* url, const char* method, const char* /*version*/,
                              const char* upload_data, size_t* upload_data_size, void** con_cls)
{
    HttpServer* server = reinterpret_cast<HttpServer*>(cls);

    if (*con_cls == nullptr) {
        HttpExchange* exchange = new HttpExchange(method);

        if (strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
            unique_ptr<MHD_PostProcessor, decltype(&MHD_destroy_post_processor)> postProcessor(
                MHD_create_post_processor(connection, POST_BUFFER_SIZE, iterate_post, reinterpret_cast<void*>(exchange)),
                MHD_destroy_post_processor);
            if (postProcessor.get() == nullptr)
                return MHD_NO;
            exchange->setPostProcessor(std::move(postProcessor));
        }

        *con_cls = reinterpret_cast<void*>(exchange);
        return MHD_YES;
    }

    HttpExchange* exchange = reinterpret_cast<HttpExchange*>(*con_cls);

    if (strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
        // Check normal handlers.
        auto it = server->handlers_.find(url);
        if (it != server->handlers_.end()) {
            return handleHandler(connection, it->second, exchange);
        }

        // Check assets handlers.
        string path = file::joinPath(server->assetDirPath_, url);
        // Check path has the prefix |server->assetDirPath_| not to allow directory listing attack.
        if (strings::hasPrefix(path, server->assetDirPath_)) {
            // When directory, we try to access index.html instead.
            if (file::isDirectory(path))
                return assetsHandler(connection, file::joinPath(path, "index.html"));
            return assetsHandler(connection, path);
        }

        return notFoundHandler(connection);
    }

    if (strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
        if (*upload_data_size != 0) {
            MHD_post_process(exchange->postProcessor(), upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }

        // Check normal handlers.
        auto it = server->handlers_.find(url);
        if (it != server->handlers_.end()) {
            return handleHandler(connection, it->second, exchange);
        }

        return notFoundHandler(connection);
    }

    return MHD_NO;
}

// static
void HttpServer::requestCompleted(void* /*cls*/, MHD_Connection* /*connection*/,
                                  void** con_cls, MHD_RequestTerminationCode)
{
    HttpExchange* exchange = reinterpret_cast<HttpExchange*>(*con_cls);
    if (exchange == nullptr)
        return;

    delete exchange;
    *con_cls = nullptr;
}

HttpServer::HttpServer(int port) :
    port_(port),
    httpd_(nullptr)
{
}

HttpServer::~HttpServer()
{
}

bool HttpServer::start()
{
    DCHECK(!httpd_);

    httpd_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port_, nullptr, nullptr,
                              &HttpServer::accessHandler, reinterpret_cast<void*>(this),
                              MHD_OPTION_NOTIFY_COMPLETED, &HttpServer::requestCompleted, nullptr,
                              MHD_OPTION_END);

    return httpd_ != nullptr;
}

void HttpServer::stop()
{
    MHD_stop_daemon(httpd_);
}

void HttpServer::setAssetDirectory(const string& path)
{
    // Needs to check with realpath.
    char* x = realpath(path.c_str(), nullptr);
    if (!x) {
        assetDirPath_ = "";
        return;
    }

    assetDirPath_ = x;
    free(x);
}

void HttpServer::installHandler(const string& path, HttpHandler handler)
{
    DCHECK(handler);
    DCHECK(!handlers_[path]);

    handlers_[path] = std::move(handler);
}

void HttpServer::installStaticFileHandler(const std::string& path,
                                          const std::string& filepath,
                                          const std::string& mime)
{
    handlers_[path] = [filepath, mime](const HttpRequest&, HttpResponse* response) {
        std::string s;
        if (!file::readFile(filepath, &s)) {
            response->setStatus(500);
            response->setContent("You requested a missing game.");
            return;
        }

        response->setStatus(200);
        response->setContent(std::move(s));
        response->addHeader("Content-Type", mime);
        return;
    };
}
