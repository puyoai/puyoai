#ifndef NET_HTTPD_HTTP_SERVER_H_
#define NET_HTTPD_HTTP_SERVER_H_

#include <memory>
#include <string>
#include <unordered_map>

#include <microhttpd.h>

#include "net/httpd/http_handler.h"

class HttpServer {
public:
    explicit HttpServer(int port);
    ~HttpServer();

    bool start();
    void stop();

    void installHandler(const std::string& path, HttpHandler);
    void installStaticFileHandler(const std::string& path,
                                  const std::string& filepath,
                                  const std::string& mime);

    // When no handler is matched, we get the content of this path.
    void setAssetDirectory(const std::string& path);

private:
    static int accessHandler(void* cls, struct MHD_Connection* connection,
                             const char* url, const char* method, const char* version,
                             const char* upload_data, size_t* upload_data_size, void** con_cls);
    static void requestCompleted(void* cls, struct MHD_Connection* connection,
                                 void** con_cls, MHD_RequestTerminationCode);

    int port_;
    struct MHD_Daemon* httpd_;
    std::unordered_map<std::string, HttpHandler> handlers_;
    std::string assetDirPath_;
};

#endif // NET_HTTPD_HTTP_SERVER_H_
