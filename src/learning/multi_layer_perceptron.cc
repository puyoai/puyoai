#include "learning/multi_layer_perceptron.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include "base/file/file.h"

namespace {

inline float activator(float x)
{
    return std::tanh(x);
}

inline float d_activator(float x)
{
    return 1 / std::cosh(x) / std::cosh(x);
}

} // namespace

namespace learning {

MultiLayerPerceptron::MultiLayerPerceptron(int in, int hid, int out) :
    num_input_(in),
    num_hidden_(hid),
    num_output_(out)
{
    o1_.reset(new float[in + 1]);

    i2_.reset(new float[hid]);
    o2_.reset(new float[hid + 1]);
    e2_.reset(new float[hid]);
    w2_.reset(new float[(in + 1) * hid]);

    i3_.reset(new float[out]);
    e3_.reset(new float[out]);
    w3_.reset(new float[(hid + 1) * out]);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> distribution(-1.0, 1.0);

    for (int i = 0; i < (in + 1) * hid; ++i) {
        w2_[i] = distribution(mt);
    }
    for (int i = 0; i < (hid + 1) * out; ++i) {
        w3_[i] = distribution(mt);
    }
}

MultiLayerPerceptron::~MultiLayerPerceptron()
{
}

int MultiLayerPerceptron::hidden_layer_weight_size() const
{
    return ((num_input_ + 1) * num_hidden_);
}

int MultiLayerPerceptron::output_layer_weight_size() const
{
    return ((num_hidden_ + 1) * num_output_);
}

bool MultiLayerPerceptron::train(int label, const float x[], float learning_rate, float l2_normalization)
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

    // normalization
    if (l2_normalization != 0.0) {
        for (int i = 0; i < hidden_layer_weight_size(); ++i) {
            w2_[i] -= learning_rate * l2_normalization * w2_[i];
        }
        for (int i = 0; i < output_layer_weight_size(); ++i) {
            w3_[i] -= learning_rate * l2_normalization * w3_[i];
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

void MultiLayerPerceptron::setHiddenLayerParameter(const float values[])
{
    memcpy(w2_.get(), values, hidden_layer_weight_size());
}

void MultiLayerPerceptron::setOutputLayerParameter(const float values[])
{
    memcpy(w3_.get(), values, output_layer_weight_size());
}

bool MultiLayerPerceptron::saveParameterAsCSource(const char* path, const char* prefix) const
{
    std::stringstream body;

    body << "#include <cstddef>" << std::endl;
    body << "#include <cstdint>" << std::endl;

    body << "const std::size_t " << prefix << "_HIDDEN_LAYER_WEIGHT_SIZE = "
         << hidden_layer_weight_size() << ";" << std::endl;
    body << "const std::size_t " << prefix << "_OUTPUT_LAYER_WEIGHT_SIZE = "
         << output_layer_weight_size() << ";" << std::endl;
    body << std::endl;

    body << "const float " << prefix << "_HIDDEN_LAYER_WEIGHT[] = {";
    for (int i = 0; i < hidden_layer_weight_size(); ++i) {
        if (i % 4 == 0) {
            body << std::endl << "    ";
        } else {
            body << " ";
        }
        body << std::scientific << std::showpos << std::setprecision(10) << w2_[i] << ",";
    }
    body << std::endl << "};" << std::endl;
    body << std::endl;

    body << "const float " << prefix << "_OUTPUT_LAYER_WEIGHT[] = {";
    for (int i = 0; i < output_layer_weight_size(); ++i) {
        if (i % 4 == 0) {
            body  << std::endl << "    ";
        } else {
            body << " ";
        }
        body << std::scientific << std::showpos << std::setprecision(10) << w3_[i] << ",";
    }
    body << std::endl << "};" << std::endl;
    body << std::endl;

    if (!file::writeFile(path, body.str()))
        return false;
    return true;
}

} // namespace learning
