#include "shape_evaluator.h"

#include <cmath>

#include "core/plan/plan.h"
#include "core/core_field.h"

#include "evaluation_parameter.h"
#include "score_collector.h"

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::eval(const CoreField& field)
{
    evalCountPuyoFeature(field);
    evalConnection(field);
    evalRestrictedConnectionHorizontalFeature(field);
    evalThirdColumnHeightFeature(field);
    evalValleyDepth(field);
    evalRidgeHeight(field);
    evalFieldUShape(field);
    evalFieldRightBias(field);
    evalUnreachableSpace(field);
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalCountPuyoFeature(const CoreField& field)
{
    sc_->addScore(NUM_COUNT_PUYOS, field.countColorPuyos(), 1);
    sc_->addScore(NUM_COUNT_OJAMA, field.countColor(PuyoColor::OJAMA));
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalConnection(const CoreField& field)
{
    int count2, count3;
    field.countConnection(&count2, &count3);
    sc_->addScore(CONNECTION_2, count2);
    sc_->addScore(CONNECTION_3, count3);
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalRestrictedConnectionHorizontalFeature(const CoreField& f)
{
    const int MAX_HEIGHT = 3; // instead of FieldConstant::HEIGHT
    for (int y = 1; y <= MAX_HEIGHT; ++y) {
        for (int x = 1; x < FieldConstant::WIDTH; ++x) {
            if (!isNormalColor(f.color(x, y)))
                continue;

            int len = 1;
            while (f.color(x, y) == f.color(x + len, y))
                ++len;

            EvaluationMoveFeatureKey key;
            if (len == 1) {
                continue;
            } else if (len == 2) {
                if (x <= 3 && 4 < x + len) {
                    key = CONNECTION_HORIZONTAL_CROSSED_2;
                } else {
                    key = CONNECTION_HORIZONTAL_2;
                }
            } else if (len == 3) {
                if (x <= 3 && 4 < x + len) {
                    key = CONNECTION_HORIZONTAL_CROSSED_3;
                } else {
                    key = CONNECTION_HORIZONTAL_3;
                }
            } else {
                CHECK(false) << "shouldn't happen: " << len;
            }

            sc_->addScore(key, 1);
            x += len - 1;
        }
    }
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalThirdColumnHeightFeature(const CoreField& field)
{
    sc_->addScore(THIRD_COLUMN_HEIGHT, field.height(3), 1);
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalValleyDepth(const CoreField& field)
{
    for (int x = 1; x <= 6; ++x) {
        if (x == 1 || x == 6)
            sc_->addScore(VALLEY_DEPTH_EDGE, field.valleyDepth(x), 1);
        else
            sc_->addScore(VALLEY_DEPTH, field.valleyDepth(x), 1);
    }
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalRidgeHeight(const CoreField& field)
{
    for (int x = 1; x <= 6; ++x) {
        sc_->addScore(RIDGE_HEIGHT, field.ridgeHeight(x), 1);
    }
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalFieldUShape(const CoreField& field)
{
    static const int DIFF[FieldConstant::MAP_WIDTH] = {
        0, -3, 0, 1, 1, 0, -3, 0,
    };

    static const double FIELD_USHAPE_HEIGHT_COEF[15] = {
        0.0, 0.1, 0.1, 0.3, 0.3,
        0.5, 0.5, 0.7, 0.7, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
    };

    double average = 0;
    for (int x = 1; x <= 6; ++x)
        average += (field.height(x) + DIFF[x]);
    average /= 6;

    double linearValue = 0;
    double squareValue = 0;

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        int h = field.height(x) + DIFF[x];
        double coef = FIELD_USHAPE_HEIGHT_COEF[field.height(x)];
        linearValue += std::abs(h - average) * coef;
        squareValue += (h - average) * (h - average) * coef;
    }

    sc_->addScore(FIELD_USHAPE_LINEAR, linearValue);
    sc_->addScore(FIELD_USHAPE_SQUARE, squareValue);
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalFieldRightBias(const CoreField& field)
{
    static const int DIFF[FieldConstant::MAP_WIDTH] = {
        0, 2, 2, 2, -8, -6, -6, 0
    };

    double average = 0;
    for (int x = 1; x <= 6; ++x)
        average += (field.height(x) + DIFF[x]);
    average /= 6;

    double linearValue = 0;
    double squareValue = 0;

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        int h = field.height(x) + DIFF[x];
        linearValue += std::abs(h - average);
        squareValue += (h - average) * (h - average);
    }

    sc_->addScore(FIELD_RIGHT_BIAS_LINEAR, linearValue);
    sc_->addScore(FIELD_RIGHT_BIAS_SQUARE, squareValue);
}

template<typename ScoreCollector>
void ShapeEvaluator<ScoreCollector>::evalUnreachableSpace(const CoreField& cf)
{
    sc_->addScore(NUM_UNREACHABLE_SPACE, cf.countUnreachableSpaces());
}

template class ShapeEvaluator<FeatureScoreCollector>;
template class ShapeEvaluator<SimpleScoreCollector>;
