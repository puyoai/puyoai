#include "mayah_base_ai.h"

#include "base/file/path.h"
#include "core/frame_request.h"

DEFINE_string(feature, SRC_DIR "/cpu/mayah/feature.toml", "the path to feature parameter");
DEFINE_string(decision_book, SRC_DIR "/cpu/mayah/decision.toml", "the path to decision book");
DEFINE_string(pattern_book, SRC_DIR "/cpu/mayah/pattern.toml", "the path to pattern book");

DEFINE_bool(from_wrapper, false, "Make this true in wrapper script.");

using namespace std;

MayahBaseAI::MayahBaseAI(int argc, char* argv[], const char* name, std::unique_ptr<Executor> executor) :
    AI(argc, argv, name),
    executor_(std::move(executor))
{
    loadEvaluationParameter();

    string decision_book_path;
    if (file::exists(FLAGS_decision_book)) {
        decision_book_path = FLAGS_decision_book;
    } else if (!file::isAbsolutePath(FLAGS_decision_book) && file::exists(file::joinPath(SRC_DIR, FLAGS_decision_book))) {
        decision_book_path = file::joinPath(SRC_DIR, FLAGS_decision_book);
    }

    string pattern_book_path;
    if (file::exists(FLAGS_pattern_book)) {
        pattern_book_path = FLAGS_pattern_book;
    } else if (!file::isAbsolutePath(FLAGS_pattern_book) && file::exists(file::joinPath(SRC_DIR, FLAGS_pattern_book))) {
        pattern_book_path = file::joinPath(SRC_DIR, FLAGS_pattern_book);
    }

    LOG(INFO) << "decision_book_path=" << decision_book_path;
    CHECK(!decision_book_path.empty()) << "decision_book_path should not be empty";
    CHECK(decisionBook_.load(decision_book_path)) << "failed to load decision book";
    LOG(INFO) << "decision_book load done";

    LOG(INFO) << "pattern_book_path=" << pattern_book_path;
    CHECK(!pattern_book_path.empty()) << "pattern_book_path should not be empty";
    CHECK(patternBook_.load(pattern_book_path)) << "failed to load pattern book";
    LOG(INFO) << "pattern_book load done";

    VLOG(1) << evaluationParameterMap_.toString();

    beam_thinker_.reset(new BeamThinker(executor_.get()));

    pattern_thinker_.reset(new PatternThinker(evaluationParameterMap_,
                                              decisionBook_,
                                              patternBook_,
                                              executor_.get()));
    rush_thinker_.reset(new RushThinker);
    side_thinker_.reset(new SideThinker);

    LOG(INFO) << "load done";
    google::FlushLogFiles(google::GLOG_INFO);
}

bool MayahBaseAI::loadEvaluationParameter()
{
    string feature_path;
    if (file::exists(FLAGS_feature)) {
        feature_path = FLAGS_feature;
    } else if (!file::isAbsolutePath(FLAGS_feature) && file::exists(file::joinPath(SRC_DIR, FLAGS_feature))) {
        feature_path = file::joinPath(SRC_DIR, FLAGS_feature);
    }

    return evaluationParameterMap_.load(feature_path);
}

bool MayahBaseAI::saveEvaluationParameter() const
{
    return evaluationParameterMap_.save(FLAGS_feature);
}

DropDecision MayahBaseAI::thinkByBeamSearch(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                                            const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    return beam_thinker_->think(frame_id, field, seq, me, enemy, fast);
}

void MayahBaseAI::onGameWillBegin(const FrameRequest& frameRequest)
{
    gazer_.initialize(frameRequest.frameId);
}

void MayahBaseAI::gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq& kumipuyoSeq)
{
    gazer_.gaze(frameId, enemyField, kumipuyoSeq);
}
