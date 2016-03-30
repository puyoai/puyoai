#include "tool/multi_layer_perceptron.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <random>
#include <string>

namespace {

inline float activator(float x)
{
    return tanh(x);
}

inline float d_activator(float x)
{
    double c = cosh(x);
    return 1 / (c * c);
}

} // namespace

namespace learning {

MultiLayerPerceptron::MultiLayerPerceptron(int num_input, int num_hidden, int num_output) :
    num_input_(num_input),
    num_hidden_(num_hidden),
    num_output_(num_output)
{
    o1_.reset(new float[num_input + 1]);

    i2_.reset(new float[num_hidden]);
    o2_.reset(new float[num_hidden + 1]);
    e2_.reset(new float[num_hidden + 1]);
    w2_.reset(new float[(num_input + 1) * num_hidden]);

    i3_.reset(new float[num_output]);
    e3_.reset(new float[num_output]);
    w3_.reset(new float[(num_hidden + 1) * num_output]);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> distribution(-1.0, 1.0);

    for (int i = 0; i < (num_input + 1) * num_hidden; ++i) {
        w2_[i] = distribution(mt);
    }
    for (int i = 0; i < (num_hidden + 1) * num_output; ++i) {
        w3_[i] = distribution(mt);
    }
}

MultiLayerPerceptron::~MultiLayerPerceptron()
{
}

bool MultiLayerPerceptron::train(int label, const float x[], float learning_rate)
{
    forward(x);
    int correct_label = std::max_element(i3_.get(), i3_.get() + num_output_) - i3_.get();

    for (int i = 0; i < num_output_; ++i) {
        if (label == i) {
            e3_[i] = i3_[i] - 1;
        } else {
            e3_[i] = i3_[i];
        }
    }

    for (int i = 0; i < num_hidden_ + 1; ++i) {
        for (int j = 0; j < num_output_; ++j) {
            w3_[i * num_output_ + j] -= learning_rate * e3_[j] * o2_[i];
        }
    }

    for (int i = 0; i < num_hidden_; ++i) {
        float t = 0;
        for (int j = 0; j < num_output_; ++j) {
            t += w3_[i * num_output_ + j] * e3_[j];
        }
        e2_[i] = t * d_activator(i2_[i]);
    }

    for (int i = 0; i < num_input_ + 1; ++i) {
        for (int j = 0; j < num_hidden_; ++j) {
            w2_[i * num_hidden_ + j] -= learning_rate * o1_[i] * e2_[j];
        }
    }

    return label == correct_label;
}

int MultiLayerPerceptron::predict(const float x[])
{
    forward(x);
    return std::max_element(i3_.get(), i3_.get() + num_output_) - i3_.get();
}

void MultiLayerPerceptron::forward(const float x[])
{
    for (int i = 0; i < num_input_; ++i) {
        o1_[i] = x[i];
    }
    o1_[num_input_] = 1.0;

    std::fill(i2_.get(), i2_.get() + num_hidden_, 0.0);
    for (int j = 0; j < num_input_ + 1; ++j) {
        for (int i = 0; i < num_hidden_; ++i) {
            i2_[i] += w2_[j * num_hidden_ + i] * o1_[j];
        }
    }

    for (int i = 0; i < num_hidden_; ++i) {
        o2_[i] = activator(i2_[i]);
    }
    o2_[num_hidden_] = 1;

    std::fill(i3_.get(), i3_.get() + num_output_, 0.0);
    for (int j = 0; j < num_hidden_ + 1; ++j) {
        for (int i = 0; i < num_output_; ++i) {
            i3_[i] += w3_[j * num_output_ + i] * o2_[j];
        }
    }
}

} // namespace learning
