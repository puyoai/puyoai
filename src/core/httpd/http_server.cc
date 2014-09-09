#include "core/httpd/http_server.h"

#include <string>
#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <microhttpd.h>

#include "base/base.h"
#include "base/path.h"
#include "base/strings.h"

using namespace std;

static bool readContent(const string& filename, string* s)
{
    ifstream ifs(filename);
    if (!ifs)
        return false;

    s->assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return true;
}

static int notFoundHandler(struct MHD_Connection* connection)
{
    const char *page = "<html><head><title>404 NOT FOUND</title></head><body>404 NOT FOUND</body></html>";
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}

static int assetsHandler(struct MHD_Connection* connection, const std::string& filename)
{
    string s;
    if (!readContent(filename, &s)) {
        LOG(INFO) << "assetsHandler " << filename << " does not exist";
        return notFoundHandler(connection);
    }

    struct MHD_Response* response = MHD_create_response_from_buffer(s.size(), (void *)s.c_str(), MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static int handleHandler(struct MHD_Connection* connection, const HttpHandler& handler)
{
    HttpRequest req;
    HttpResponse resp;

    handler(&req, &resp);

    struct MHD_Response* response = MHD_create_response_from_buffer(
        resp.contentSize(), resp.content(), MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, resp.status(), response);
    MHD_destroy_response(response);
    return ret;
}

// static
int HttpServer::accessHandler(void* cls, struct MHD_Connection* connection,
                              const char* url, const char* /*method*/, const char* /*version*/,
                              const char* /*upload_data*/, size_t* /*upload_data_size*/, void** /*con_cls*/)
{
    HttpServer* server = reinterpret_cast<HttpServer*>(cls);

    LOG(INFO) << "access: " << url;

    // Check normal handlers.
    auto it = server->handlers_.find(url);
    if (it != server->handlers_.end())
        return handleHandler(connection, it->second);

    // Check assets handlers.
    string path = joinPath(server->assetDirPath_, url);
    // Check path has the prefix |server->assetDirPath_| not to allow directory listing attack.
    if (isPrefix(path, server->assetDirPath_)) {
        // When directory, we try to access index.html instead.
        if (isDirectory(path))
            return assetsHandler(connection, joinPath(path, "index.html"));
        return assetsHandler(connection, path);
    }

    return notFoundHandler(connection);
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

    httpd_ = MHD_start_daemon(
        MHD_USE_THREAD_PER_CONNECTION,
        port_, NULL, NULL, &HttpServer::accessHandler, (void*)this, MHD_OPTION_END);

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
    DCHECK(handlers_[path] == nullptr);

    handlers_[path] = handler;
}
