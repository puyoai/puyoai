#ifndef CPU_MAYAH_MOVE_EVALUATOR_H_
#define CPU_MAYAH_MOVE_EVALUATOR_H_

class RefPlan;

template<typename ScoreCollector>
class MoveEvaluator {
public:
    explicit MoveEvaluator(ScoreCollector* sc) : sc_(sc) {}

    void eval(const RefPlan&);

    void evalFrameFeature(int totalFrames, int numChigiri);

private:
    ScoreCollector* sc_;
};

#endif // CPU_MAYAH_MOVE_EVALUATOR_H_
