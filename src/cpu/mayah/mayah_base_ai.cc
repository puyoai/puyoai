#include "mayah_base_ai.h"

DEFINE_string(feature, "feature.toml", "the path to feature parameter");
DEFINE_string(decision_book, SRC_DIR "/cpu/mayah/decision.toml", "the path to decision book");
DEFINE_string(pattern_book, SRC_DIR "/cpu/mayah/pattern.toml", "the path to pattern book");

using namespace std;

MayahBaseAI::MayahBaseAI(int argc, char* argv[], const char* name, std::unique_ptr<Executor> executor) :
    AI(argc, argv, name),
    executor_(std::move(executor))
{
    loadEvaluationParameter();
    CHECK(decisionBook_.load(FLAGS_decision_book));
    CHECK(patternBook_.load(FLAGS_pattern_book));

    VLOG(1) << evaluationParameterMap_.toString();

    beam_thinker_.reset(new BeamThinker(executor_.get()));
    pattern_thinker_.reset(new PatternThinker(evaluationParameterMap_,
                                              decisionBook_,
                                              patternBook_,
                                              executor_.get()));

    google::FlushLogFiles(google::GLOG_INFO);
}

bool MayahBaseAI::loadEvaluationParameter()
{
    if (evaluationParameterMap_.load(FLAGS_feature))
        return true;

    // When not found, we try to load from the source directory.
    std::string filename = std::string(SRC_DIR) + "/cpu/mayah/" + FLAGS_feature;
    if (evaluationParameterMap_.load(filename))
        return true;

    return false;
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
