#include "core/httpd/http_server.h"

#include <string>
#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <microhttpd.h>

#include "base/base.h"

DECLARE_string(data_dir);

using namespace std;

static string joinPath(const string& lhs, const string& rhs)
{
    string s = lhs + "/" + rhs;
    char* x = realpath(s.c_str(), nullptr);
    string result(x);
    free(x);
    return result;
}

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

static int handleHandler(struct MHD_Connection* connection, HttpHandler* handler)
{
    HttpRequest req;
    HttpResponse resp;
    handler->handle(&req, &resp);

    struct MHD_Response* response = MHD_create_response_from_buffer(
        resp.contentSize(), resp.content(), MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, resp.status(), response);
    MHD_destroy_response(response);
    return ret;
}

// static
int HttpServer::accessHandler(void* cls, struct MHD_Connection* connection,
                              const char* url, const char* method, const char* version,
                              const char* upload_data, size_t* upload_data_size, void** con_cls)
{
    UNUSED_VARIABLE(con_cls);
    HttpServer* server = reinterpret_cast<HttpServer*>(cls);

    LOG(INFO) << "access: " << url;

    auto it = server->handlers_.find(url);
    if (it != server->handlers_.end())
        return handleHandler(connection, it->second);

    // TODO(mayah): Need to write Handler instead of these.
    if (strcmp(url, "/") == 0)
        return assetsHandler(connection, joinPath(FLAGS_data_dir, "assets/index.html"));
    if (strcmp(url, "/puyo.js") == 0)
        return assetsHandler(connection, joinPath(FLAGS_data_dir, "assets/puyo.js"));
    if (strcmp(url, "/puyo.css") == 0)
        return assetsHandler(connection, joinPath(FLAGS_data_dir, "assets/puyo.css"));
    if (strcmp(url, "/puyo.png") == 0)
        return assetsHandler(connection, joinPath(FLAGS_data_dir, "assets/puyo.png"));
    if (strcmp(url, "/yokoku.png") == 0)
        return assetsHandler(connection, joinPath(FLAGS_data_dir, "assets/yokoku.png"));
    if (strcmp(url, "/background.png") == 0)
        return assetsHandler(connection, joinPath(FLAGS_data_dir, "assets/background.png"));

    return notFoundHandler(connection);
}

HttpServer::HttpServer() :
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
        kPort, NULL, NULL, &HttpServer::accessHandler, (void*)this, MHD_OPTION_END);

    return httpd_ != nullptr;
}

void HttpServer::stop()
{
    MHD_stop_daemon(httpd_);
}

void HttpServer::installHandler(const string& path, HttpHandler* handler)
{
    DCHECK(handlers_[path] == nullptr);
    DCHECK(handler != nullptr);

    handlers_[path] = handler;
}

