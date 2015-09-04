#ifndef CPU_MAYAH_SHAPE_EVALUATOR_H_
#define CPU_MAYAH_SHAPE_EVALUATOR_H_

class CoreField;
class RefPlan;

template<typename ScoreCollector>
class ShapeEvaluator {
public:
    explicit ShapeEvaluator(ScoreCollector* sc) : sc_(sc) {}

    void eval(const CoreField&);

    void evalCountPuyoFeature(const CoreField&);
    void evalConnection(const CoreField&);
    void evalRestrictedConnectionHorizontalFeature(const CoreField&);
    void evalThirdColumnHeightFeature(const CoreField&);
    void evalValleyDepth(const CoreField&);
    void evalRidgeHeight(const CoreField&);
    void evalFieldUShape(const CoreField&);
    void evalFieldRightBias(const CoreField&);
    void evalUnreachableSpace(const CoreField&);

private:
    ScoreCollector* sc_;
};

#endif // CPU_MAYAH_MOVE_EVALUATOR_H_
