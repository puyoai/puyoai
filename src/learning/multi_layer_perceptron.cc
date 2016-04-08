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
    w2_.reset(new float[(in + 1) * hid]);
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

MultiLayerPerceptron::ForwardingIntermediateStorage MultiLayerPerceptron::makeForwadingStorage() const
{
    ForwardingIntermediateStorage data;
    data.o1.reset(new float[num_input_ + 1]);
    data.i2.reset(new float[num_hidden_]);
    data.o2.reset(new float[num_hidden_ + 1]);
    data.i3.reset(new float[num_output_]);
    return data;
}

MultiLayerPerceptron::BackPropagationIntermediateStorage MultiLayerPerceptron::makeBackpropagationStorage() const
{
    BackPropagationIntermediateStorage error_data;
    error_data.e2.reset(new float[num_hidden_]);
    error_data.e3.reset(new float[num_output_]);
    return error_data;
}

int MultiLayerPerceptron::hidden_layer_weight_size() const
{
    return ((num_input_ + 1) * num_hidden_);
}

int MultiLayerPerceptron::output_layer_weight_size() const
{
    return ((num_hidden_ + 1) * num_output_);
}

int MultiLayerPerceptron::predict(const float x[], ForwardingIntermediateStorage* data) const
{
    forward(x, data);
    return std::max_element(data->i3.get(), data->i3.get() + num_output_) - data->i3.get();
}

bool MultiLayerPerceptron::train(int correct_label,
                                 const float x[],
                                 ForwardingIntermediateStorage* data,
                                 BackPropagationIntermediateStorage* error_data,
                                 float learning_rate,
                                 float l2_normalization)
{
    int predicted_label = predict(x, data);

    // calculate error.
    for (int i = 0; i < num_output_; ++i) {
        if (correct_label == i) {
            error_data->e3[i] = data->i3[i] - 1;
        } else {
            error_data->e3[i] = data->i3[i];
        }
    }

    for (int i = 0; i < num_hidden_; ++i) {
        float t = 0;
        for (int j = 0; j < num_output_; ++j) {
            t += w3_[i * num_output_ + j] * error_data->e3[j];
        }
        error_data->e2[i] = t * d_activator(data->i2[i]);
    }

    // back propagation
    for (int i = 0; i < num_hidden_ + 1; ++i) {
        for (int j = 0; j < num_output_; ++j) {
            w3_[i * num_output_ + j] -= learning_rate * error_data->e3[j] * data->o2[i];
        }
    }

    for (int i = 0; i < num_input_ + 1; ++i) {
        for (int j = 0; j < num_hidden_; ++j) {
            w2_[i * num_hidden_ + j] -= learning_rate * data->o1[i] * error_data->e2[j];
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

    return correct_label == predicted_label;
}

void MultiLayerPerceptron::forward(const float x[], ForwardingIntermediateStorage* data) const
{
    for (int i = 0; i < num_input_; ++i) {
        data->o1[i] = x[i];
    }
    data->o1[num_input_] = 1.0;

    std::fill(data->i2.get(), data->i2.get() + num_hidden_, 0.0);
    for (int j = 0; j < num_input_ + 1; ++j) {
        for (int i = 0; i < num_hidden_; ++i) {
            data->i2[i] += w2_[j * num_hidden_ + i] * data->o1[j];
        }
    }

    for (int i = 0; i < num_hidden_; ++i) {
        data->o2[i] = activator(data->i2[i]);
    }
    data->o2[num_hidden_] = 1;

    std::fill(data->i3.get(), data->i3.get() + num_output_, 0.0);
    for (int j = 0; j < num_hidden_ + 1; ++j) {
        for (int i = 0; i < num_output_; ++i) {
            data->i3[i] += w3_[j * num_output_ + i] * data->o2[j];
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
