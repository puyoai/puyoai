#include "recognition/arow.h"

#include <stdio.h>

#include <glog/logging.h>

using namespace std;

Arow::Arow()
    : mean(SIZE), cov(SIZE)
{
    std::fill(cov.begin(), cov.end(), 1.0);
}

double Arow::margin(const vector<double>& features) const
{
    double result = 0.0;
    for (int i = 0; i < SIZE; ++i) {
        result += mean[i] * features[i];
    }
    return result;
}

double Arow::margin(const double features[SIZE]) const
{
    double result = 0.0;
    for (int i = 0; i < SIZE; ++i) {
        result += mean[i] * features[i];
    }
    return result;
}

double Arow::confidence(const vector<double>& features) const
{
    double result = 0.0;
    for (int i = 0; i < SIZE; ++i) {
        result += cov[i] * features[i] * features[i];
    }
    return result;
}

int Arow::update(const vector<double>& features, int label)
{
    double m = margin(features);
    int loss = m * label < 0 ? 1 : 0;
    if (m * label >= 1)
        return 0;

    double v = confidence(features);
    double beta = 1.0 / (v + RATE);
    double alpha = (1.0 - label * m) * beta;

    // update mean
    for (int i = 0; i < SIZE; ++i) {
        mean[i] += alpha * label * cov[i] * features[i];
    }

    // update covariance
    for (int i = 0; i < SIZE; ++i) {
        cov[i] = 1.0 / ((1.0 / cov[i]) + features[i] * features[i] / RATE);
    }

    return loss;
}

int Arow::predict(const vector<double>& features) const
{
    double m = margin(features);
    return m > 0 ? 1 : -1;
}

void Arow::save(const string& filename) const
{
    FILE* fp = fopen(filename.c_str(), "wb");
    PCHECK(fp);

    fwrite(&mean[0], sizeof(double), SIZE, fp);
    fwrite(&cov[0], sizeof(double), SIZE, fp);

    fclose(fp);
}

void Arow::load(const string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    PCHECK(fp);

    fread(&mean[0], sizeof(double), SIZE, fp);
    fread(&cov[0], sizeof(double), SIZE, fp);

    fclose(fp);
}
