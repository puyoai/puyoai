#ifndef LEARNING_AROW_H_
#define LEARNING_AROW_H_

#include <string>
#include <vector>

// Arow, Adaptive Regularization of Weight Vectors, is a linear classifier.
// Based on this paper
// http://papers.nips.cc/paper/3848-adaptive-regularization-of-weight-vectors.pdf
class Arow {
public:
    explicit Arow(size_t size = 16 * 16 * 3, double rate = 0.1);

    void setRate(double r) { rate_ = r; }

    // The size of |features| must be |size_|.
    double margin(const double features[]) const;
    double margin(const std::vector<double>& features) const;
    double confidence(const std::vector<double>& features) const;
    int update(const std::vector<double>& features, int label);
    int predict(const std::vector<double>& features) const;

    void setMean(std::vector<double>);
    void setCov(std::vector<double>);

    const std::vector<double>& mean() const { return mean_; }
    const std::vector<double>& cov() const { return cov_; }

private:
    const size_t size_;
    double rate_;

    std::vector<double> mean_;
    std::vector<double> cov_;
};

#endif // CAPTURE_RECOGNITION_RECOGNITION_AROW_H_
