#ifndef RECOGNITION_RECOGNITION_AROW_H_
#define RECOGNITION_RECOGNITION_AROW_H_

#include <string>
#include <vector>

#include "recognition/recognition_color.h"

class Arow {
public:
    static const int SIZE = 16 * 16 * 3;
    static constexpr double RATE = 0.1;

    Arow();

    double margin(const double features[Arow::SIZE]) const;
    double margin(const std::vector<double>& features) const;
    double confidence(const std::vector<double>& features) const;
    int update(const std::vector<double>& features, int label);
    int predict(const std::vector<double>& features) const;

    void save(const std::string& filename) const;
    void load(const std::string& filename);

private:

    std::vector<double> mean;
    std::vector<double> cov;
};

#endif // CORE_CAPTURE_RECOGNITION_AROW_H_
