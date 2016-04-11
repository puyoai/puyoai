#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/file/path.h"
#include "base/wait_group.h"
#include "core/pattern/pattern_book.h"
#include "net/httpd/http_server.h"

DECLARE_string(pattern_book);
DECLARE_string(data_dir);
DECLARE_string(src_dir);

DEFINE_int32(port, 8000, "httpd port");

using namespace std;

void onSubmit(const HttpRequest& request, HttpResponse* response)
{
    for (const auto& entry : request.valueMap()) {
        cout << entry.first << " -> " << entry.second << endl;
    }

    response->setStatus(200);
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    PatternBook patternBook;
    CHECK(patternBook.load(FLAGS_pattern_book));

    HttpServer server(FLAGS_port);

    server.installStaticFileHandler("/",
                                    file::joinPath(SRC_DIR, "cpu/mayah/pattern_maker/index.html"),
                                    "text/html");
    server.installStaticFileHandler("/static/style.css",
                                    file::joinPath(SRC_DIR, "cpu/mayah/pattern_maker/style.css"),
                                    "text/css");
    server.installStaticFileHandler("/static/script.js",
                                    file::joinPath(SRC_DIR, "cpu/mayah/pattern_maker/script.js"),
                                    "text/javascript");

    server.installStaticFileHandler("/static/bg.png", file::joinPath(FLAGS_data_dir, "assets", "bg.png"), "image/png");
    server.installStaticFileHandler("/static/puyo.png", file::joinPath(FLAGS_data_dir, "assets", "puyo.png"), "image/png");

    server.installHandler("/api/post", onSubmit);

    server.start();

    // Wait until Ctrl-C.
    WaitGroup wg;
    wg.add(1);
    wg.waitUntilDone();

    server.stop();

    return 0;
}
