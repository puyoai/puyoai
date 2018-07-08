#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <tuple>

#include <gflags/gflags.h>
#include <glog/logging.h>

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

#ifdef USE_LIBCURL
# include <curl/curl.h>
#endif
#include <json/json.h>

DEFINE_string(problem, "", "use problem");
DEFINE_bool(tokopuyo, false, "Use tokopuyo parameter");

using namespace std;

#ifdef USE_LIBCURL

size_t write_callback(void* dst, size_t size, size_t nmemb, void* userp) {
    string* str = reinterpret_cast<string*>(userp);
    str->append(reinterpret_cast<char*>(dst), size * nmemb);
    return size * nmemb;
}

string askPuyoServer(const PlainField& field, const Kumipuyo& next1, const Kumipuyo& next2, int rest_hand) {
    // convert to json.
    // example: {"field":"....................................................................................",
    //           "next1":"RR",
    //           "next2":"BB",
    //           "rest":50}

    stringstream ss;
    ss << "{"
       << "\"field\":\"" << field.toString('.') << "\","
       << "\"next1\":\"" << next1.axis << next1.child << "\","
       << "\"next2\":\"" << next2.axis << next2.child << "\","
       << "\"rest\":" << rest_hand
       << "}";
    string body = ss.str();
    LOG(INFO) << body;

    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG(ERROR) << "failed to init curl";
        return string();
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
        return string();
    }

    // LOG(INFO) << buffer;

    curl_easy_cleanup(curl);

    return buffer;
}
#endif

class InteractiveAI : public DebuggableMayahAI {
public:
    InteractiveAI(int argc, char* argv[]) : DebuggableMayahAI(argc, argv) {}
};

Problem makeProblem()
{
    KumipuyoSeq generated = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    LOG(INFO) << "seq=" << generated.toString();

    Problem problem;
    if (!FLAGS_problem.empty()) {
        problem = Problem::readProblem(FLAGS_problem);

        // Add generated sequence after problem.
        problem.myState.seq.append(generated);
        problem.enemyState.seq.append(generated);
    } else {
        problem.myState.seq = generated;
        problem.enemyState.field = CoreField(
            "5...65"
            "4...66"
            "545645"
            "456455"
            "545646"
            "545646"
            "564564"
            "456456"
            "456456"
            "456456");
        problem.enemyState.seq = KumipuyoSeq("666666");
        problem.enemyState.seq.append(generated);
    }

    return problem;
}

// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    InteractiveAI ai(argc, argv);
    if (FLAGS_tokopuyo) {
        ai.removeNontokopuyoParameter();
        ai.setUsesRensaHandTree(false);
        cout << "tokopuyo mode: Enemy hand won't be considered." << endl;
    } else {
        cout << "NON TOKOPUYO MODE: Enemy hand will be considered. Use --tokopuyo if you want to use tokopuyo mode." << endl;
    }

    Problem problem = makeProblem();

    FrameRequest req;
    req.frameId = 1;
    ai.gameWillBegin(req);

    req.frameId = 2;
    req.playerFrameRequest[0].field = problem.myState.field.toPlainField();
    req.playerFrameRequest[1].field = problem.enemyState.field.toPlainField();
    req.playerFrameRequest[0].kumipuyoSeq = problem.myState.seq;
    req.playerFrameRequest[1].kumipuyoSeq = problem.enemyState.seq;

    ai.mutableMyPlayerState()->field = problem.myState.field;
    ai.mutableMyPlayerState()->seq = problem.enemyState.seq;
    ai.mutableEnemyPlayerState()->field = problem.enemyState.field;
    ai.mutableEnemyPlayerState()->seq = problem.enemyState.seq;

    for (int i = 0; i < 50; ++i) {
        // frameId 1 will be used for initializing now. Let's avoid it.
        const int frameId = 2 + i;
        req.frameId = frameId;

        // Call these callback for gazer.
        {
            double t1 = currentTime();
            ai.gaze(frameId, CoreField(req.playerFrameRequest[1].field), req.playerFrameRequest[1].kumipuyoSeq);
            double t2 = currentTime();
            cout << "gazer time = " << (t2 - t1) << endl;
        }

        // Waits for user enter.
        while (true) {
            CoreField currentField(req.playerFrameRequest[0].field);
            const KumipuyoSeq& seq = req.playerFrameRequest[0].kumipuyoSeq;

            FieldPrettyPrinter::printMultipleFields(
                { currentField.toPlainField(), req.playerFrameRequest[1].field },
                { seq.subsequence(0, 2), req.playerFrameRequest[1].kumipuyoSeq });
            double t1 = currentTime();
            ThoughtResult aiThoughtResult = ai.thinkPlan(frameId, currentField, seq.subsequence(0, 2),
                                                         ai.myPlayerState(), ai.enemyPlayerState(),
                                                         PatternThinker::DEFAULT_DEPTH, PatternThinker::DEFAULT_NUM_ITERATION);
            const Plan& aiPlan = aiThoughtResult.plan;

            double t2 = currentTime();
            if (aiPlan.decisions().empty())
                cout << "No decision";
            else {
                for (size_t i = 0; i < aiPlan.decisions().size(); ++i) {
                    if (i)
                        cout << "-";
                    cout << aiPlan.decision(i).toString();
                }
            }
            cout << " time = " << ((t2 - t1) * 1000) << " [ms]" << endl;

#ifdef USE_LIBCURL
            {
                cout << "ask_puyo_server (2)" << endl;
                string json_str = askPuyoServer(currentField.toPlainField(), seq.get(0), seq.get(1), 50 - i);
                if (!json_str.empty()) {
                    Json::Reader reader;
                    Json::Value root;
                    if (reader.parse(json_str, root)) {
                        for (int x = 1; x <= 6; ++x) {
                            for (int r = 0; r < 4; ++r) {
                                double p = root["result"]["p"][x - 1][r].asDouble();
                                char buf[1024];
                                sprintf(buf, "(%d, %d)=%8.4f", x, r, p);
                                cout << buf << "  ";
                            }
                            cout << endl;
                        }
                        cout << "q=" << root["result"]["q"] << endl;
                    } else {
                        cout << "failed to parse" << endl;
                    }
                }
            }
#endif

            string str;
            cout << "command? ";
            getline(cin, str);

            if (str == "")
                break;
            if (str == "reload") {
                ai.loadEvaluationParameter();
                continue;
            }

            int x1, r1, x2, r2;
            int r = sscanf(str.c_str(), "%d %d %d %d", &x1, &r1, &x2, &r2);
            if (r == 2 || r == 4) {
                vector<Decision> decisions;
                if (r == 2) {
                    Decision d1(x1, r1);
                    if (!d1.isValid())
                        continue;
                    decisions.push_back(d1);
                } else if (r == 4) {
                    Decision d1(x1, r1);
                    Decision d2(x2, r2);
                    if (!d1.isValid() || !d2.isValid())
                        continue;
                    decisions.push_back(d1);
                    decisions.push_back(d2);
                } else {
                    continue;
                }

                ThoughtResult myThoughtResult = ai.thinkPlan(frameId, currentField,  seq.subsequence(0, 2),
                                                             ai.myPlayerState(), ai.enemyPlayerState(),
                                                             PatternThinker::DEFAULT_DEPTH, PatternThinker::DEFAULT_NUM_ITERATION, false, &decisions);

                CollectedFeatureCoefScore mycf = ai.evalWithCollectingFeature(
                    RefPlan(myThoughtResult.plan),
                    seq.subsequence(0, 2).subsequence(myThoughtResult.plan.decisions().size()),
                    frameId, PatternThinker::DEFAULT_NUM_ITERATION,
                    ai.myPlayerState(), ai.enemyPlayerState(), myThoughtResult.midEvalResult, false,
                    ai.gazer().gazeResult());
                CollectedFeatureCoefScore aicf = ai.evalWithCollectingFeature(
                    RefPlan(aiThoughtResult.plan),
                    seq.subsequence(0, 2).subsequence(aiThoughtResult.plan.decisions().size()),
                    frameId, PatternThinker::DEFAULT_NUM_ITERATION,
                    ai.myPlayerState(), ai.enemyPlayerState(), aiThoughtResult.midEvalResult, false,
                    ai.gazer().gazeResult());

                CoreField myTargetField(myThoughtResult.plan.field());
                myTargetField.dropPuyoList(mycf.mainRensaScore().puyosToComplement);

                CoreField aiTargetField(aiThoughtResult.plan.field());
                aiTargetField.dropPuyoList(aicf.mainRensaScore().puyosToComplement);

                KumipuyoSeq seqToShow { seq.get(2), seq.get(3) };
                FieldPrettyPrinter::printMultipleFields(
                    { myThoughtResult.plan.field().toPlainField(), aiThoughtResult.plan.field().toPlainField(), myTargetField.toPlainField(), aiTargetField.toPlainField() },
                    { seqToShow, seqToShow, KumipuyoSeq(), KumipuyoSeq() });

                cout << CollectedFeatureCoefScore::scoreComparisionString(mycf, aicf, ai.evaluationParameterMap()) << endl;
                cout << "MY: " << myThoughtResult.message << endl;
                cout << "AI: " << aiThoughtResult.message << endl;
            }
        }

        ThoughtResult thoughtResult = ai.thinkPlan(frameId,
                                                   CoreField(req.playerFrameRequest[0].field),
                                                   req.playerFrameRequest[0].kumipuyoSeq.subsequence(0, 2),
                                                   ai.myPlayerState(),
                                                   ai.enemyPlayerState(),
                                                   PatternThinker::DEFAULT_DEPTH, PatternThinker::DEFAULT_NUM_ITERATION);
        {
            CoreField f(req.playerFrameRequest[0].field);
            f.dropKumipuyo(thoughtResult.plan.decisions().front(), req.playerFrameRequest[0].kumipuyoSeq.front());
            f.simulate();
            req.playerFrameRequest[0].field = f.toPlainField();
        }
        req.playerFrameRequest[0].kumipuyoSeq.dropFront();
    }

    return 0;
}
