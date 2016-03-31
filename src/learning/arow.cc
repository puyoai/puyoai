#include "learning/arow.h"

#include <glog/logging.h>
#include <stdio.h>

using namespace std;

Arow::Arow(size_t size, double rate) :
    size_(size),
    rate_(rate),
    mean_(size),
    cov_(size)
{
    std::fill(cov_.begin(), cov_.end(), 1.0);
}

double Arow::margin(const double features[]) const
{
    double result = 0.0;
    for (size_t i = 0; i < size_; ++i) {
        result += mean_[i] * features[i];
    }
    return result;
}

double Arow::margin(const vector<double>& features) const
{
    DCHECK_EQ(features.size(), size_);

    double result = 0.0;
    for (size_t i = 0; i < size_; ++i) {
        result += mean_[i] * features[i];
    }
    return result;
}

double Arow::confidence(const vector<double>& features) const
{
    DCHECK_EQ(features.size(), size_);

    double result = 0.0;
    for (size_t i = 0; i < size_; ++i) {
        result += cov_[i] * features[i] * features[i];
    }
    return result;
}

int Arow::update(const vector<double>& features, int label)
{
    DCHECK_EQ(features.size(), size_);

    double m = margin(features);
    int loss = m * label < 0 ? 1 : 0;
    if (m * label >= 1)
        return 0;

    double v = confidence(features);
    double beta = 1.0 / (v + rate_);
    double alpha = (1.0 - label * m) * beta;

    // update mean_
    for (size_t i = 0; i < size_; ++i) {
        mean_[i] += alpha * label * cov_[i] * features[i];
    }

    // update cov_ariance
    for (size_t i = 0; i < size_; ++i) {
        cov_[i] = 1.0 / ((1.0 / cov_[i]) + features[i] * features[i] / rate_);
    }

    return loss;
}

int Arow::predict(const vector<double>& features) const
{
    DCHECK_EQ(features.size(), size_);

    double m = margin(features);
    return m > 0 ? 1 : -1;
}

void Arow::setMean(vector<double> mean)
{
    CHECK_EQ(mean_.size(), size_);
    mean_ = std::move(mean);
}

void Arow::setCov(vector<double> cov)
{
    CHECK_EQ(cov_.size(), size_);
    cov_ = std::move(cov);
}
