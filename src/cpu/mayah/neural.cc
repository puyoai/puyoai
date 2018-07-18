#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <tuple>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <curl/curl.h>
#include <json/json.h>

#include "base/strings.h"
#include "base/time.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/core_field.h"
#include "core/field_pretty_printer.h"
#include "core/frame_request.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/probability/puyo_set_probability.h"
#include "solver/problem.h"

#include "evaluator.h"
#include "pattern_thinker.h"
#include "mayah_ai.h"

using namespace std;

struct NeuralNetRequest {
    NeuralNetRequest() = default;
    NeuralNetRequest(const PlainField& plain_field, const Kumipuyo& next1, const Kumipuyo& next2, int rest_hand) :
        plain_field(plain_field),
        next1(next1),
        next2(next2),
        rest_hand(rest_hand) {}

    PlainField plain_field;
    Kumipuyo next1;
    Kumipuyo next2;
    int rest_hand = 0;
};

struct NeuralNetResponse {
    bool ok;
    double possibility[6][4];
    double q;

    static NeuralNetResponse invalid() {
        NeuralNetResponse resp;
        resp.ok = false;
        return resp;
    }
};

size_t write_callback(void* dst, size_t size, size_t nmemb, void* userp) {
    string* str = reinterpret_cast<string*>(userp);
    str->append(reinterpret_cast<char*>(dst), size * nmemb);
    return size * nmemb;
}

NeuralNetResponse ask_puyo_server(const NeuralNetRequest& request) {
    // convert to json.
    // example: {"field":"....................................................................................",
    //           "next1":"RR",
    //           "next2":"BB",
    //           "rest":50}

    stringstream ss;
    ss << "{"
       << "\"field\":\"" << request.plain_field.toString('.') << "\","
       << "\"next1\":\"" << request.next1.axis << request.next1.child << "\","
       << "\"next2\":\"" << request.next2.axis << request.next2.child << "\","
       << "\"rest\":" << request.rest_hand
       << "}";
    string body = ss.str();
    LOG(INFO) << body;

    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG(ERROR) << "failed to init curl";

        return NeuralNetResponse::invalid();
    }

    string buffer;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:2424/eval");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&buffer));
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        LOG(ERROR) << "failed to get data";
        curl_easy_cleanup(curl);
        return NeuralNetResponse::invalid();
    }

    // LOG(INFO) << buffer;

    curl_easy_cleanup(curl);

    if (buffer.empty()) {
        return NeuralNetResponse::invalid();
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(buffer, root)) {
        return NeuralNetResponse::invalid();
    }

    NeuralNetResponse resp;
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            double p = root["result"]["p"][x - 1][r].asDouble();
            resp.possibility[x - 1][r] = p;
        }
    }
    resp.q = root["result"]["q"].asDouble();
    resp.ok = true;

    return resp;
}


// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    KumipuyoSeq current_seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    PlainField current_field;

    for (int i = 0; i < 50; ++i) {
        // Show current field.
        FieldPrettyPrinter::print(current_field, current_seq);

        // ask neural net server.
        NeuralNetResponse resp = ask_puyo_server(NeuralNetRequest(current_field, current_seq.get(0), current_seq.get(1), 50 - i));
        if (!resp.ok) {
            cout << "failed to ask puyo server";
            return 1;
        }

        printf("q = %f\n", resp.q);
        for (int x = 1; x <= 6; ++x) {
            for (int r = 0; r < 4; ++r) {
                printf("(%d, %d) = %8.4f  ", x, r, resp.possibility[x-1][r]);
            }
            printf("\n");
        }

        double best = resp.possibility[0][0];
        int best_x = 1;
        int best_r = 0;
        for (int x = 1; x <= 6; ++x) {
            for (int r = 0; r < 4; ++r) {
                if ((x == 1 && r == 3) || (x == 6 && r == 1)) {
                    continue;
                }

                if (resp.possibility[x-1][r] > best) {
                    best = resp.possibility[x-1][r];
                    best_x = x;
                    best_r = r;
                }
            }
        }

        string str;
        cout << "command? ";
        getline(cin, str);

        CoreField cf(current_field);
        cf.dropKumipuyo(Decision(best_x, best_r), current_seq.front());
        RensaResult rensa_result = cf.simulate();
        if (rensa_result.chains > 0) {
            cout << rensa_result << endl;
        }

        current_field = cf.toPlainField();
        current_seq.dropFront();     
    }

    return 0;
}
