#ifndef CAPTURE_RECOGNITION_RECOGNITION_AROW_H_
#define CAPTURE_RECOGNITION_RECOGNITION_AROW_H_

#include <string>
#include <vector>

#include "capture/recognition/recognition_color.h"

// Arow, Adaptive Regularization of Weight Vectors, is a linear classifier.
// Based on this paper
// http://papers.nips.cc/paper/3848-adaptive-regularization-of-weight-vectors.pdf
class Arow {
public:
    static const size_t SIZE = 16 * 16 * 3;
    static constexpr double RATE = 0.1;

    Arow();

    double margin(const double features[Arow::SIZE]) const;
    double margin(const std::vector<double>& features) const;
    double confidence(const std::vector<double>& features) const;
    int update(const std::vector<double>& features, int label);
    int predict(const std::vector<double>& features) const;

    void setMean(std::vector<double>);
    void setCov(std::vector<double>);

    const std::vector<double>& mean() const { return mean_; }
    const std::vector<double>& cov() const { return cov_; }

private:
    std::vector<double> mean_;
    std::vector<double> cov_;
};

#endif // CAPTURE_RECOGNITION_RECOGNITION_AROW_H_
