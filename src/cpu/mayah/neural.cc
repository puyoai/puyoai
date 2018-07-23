#include "cpu/mayah/neural.h"

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

size_t write_callback(void* dst, size_t size, size_t nmemb, void* userp) {
    string* str = reinterpret_cast<string*>(userp);
    str->append(reinterpret_cast<char*>(dst), size * nmemb);
    return size * nmemb;
}

vector<NeuralNetResponse> ask_puyo_server(const std::vector<NeuralNetRequest>& request) {
    // convert to json.
    // example: {"field":"....................................................................................",
    //           "next1":"RR",
    //           "next2":"BB",
    //           "rest":50}

    std::string body;
    {
        Json::Value root(Json::arrayValue);
        for (size_t i = 0; i < request.size(); ++i) {
            const NeuralNetRequest& req = request[i];
            Json::Value obj;
            obj["field"] = req.plain_field.toString('.');
            {
                stringstream ss;
                ss << req.next1.axis << req.next1.child;
                obj["next1"] = ss.str();
            }
            {
                stringstream ss;
                ss << req.next2.axis << req.next2.child;
                obj["next2"] = ss.str();
            }
            obj["rest"] = req.rest_hand;
            root.append(std::move(obj));
        }

        Json::FastWriter writer;
        body = writer.write(root);
        LOG(INFO) << body;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG(ERROR) << "failed to init curl";

        return std::vector<NeuralNetResponse>();
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
        return std::vector<NeuralNetResponse>();
    }

    curl_easy_cleanup(curl);

    if (buffer.empty()) {
        return std::vector<NeuralNetResponse>();
    }

    std::vector<NeuralNetResponse> response;
    {
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(buffer, root)) {
            return std::vector<NeuralNetResponse>();
        }

        const Json::Value& result = root["result"];
        for (int i = 0; i < static_cast<int>(result.size()); ++i) {
            NeuralNetResponse resp;
            for (int x = 1; x <= 6; ++x) {
                for (int r = 0; r < 4; ++r) {
                    double p = result[i]["p"][x - 1][r].asDouble();
                    resp.possibility[x - 1][r] = p;
                }
            }
            resp.q = result[i]["q"].asDouble();
            resp.ok = true;
            response.push_back(std::move(resp));
        }
    }
    return response;
}



