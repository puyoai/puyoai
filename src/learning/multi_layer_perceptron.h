#ifndef LEARNING_MULTILAYER_PERCEPTRON_H_
#define LEARNING_MULTILAYER_PERCEPTRON_H_

#include <memory>

namespace learning {

// Defines a 3-layer perceptron.
// This is not thread-safe.
class MultiLayerPerceptron {
public:
    MultiLayerPerceptron(int in, int hid, int out);
    ~MultiLayerPerceptron();

    // Train single data.
    // |x| should have |num_input_| size.
    bool train(int label, const float x[], float learning_rate = 0.1);

    // Returns the label.
    // |x| should have |num_input_| size.
    int predict(const float x[]);

    bool saveParameterAsCSource(const char* path, const char* prefix) const;

private:
    int hidden_layer_weight_size() const;
    int output_layer_weight_size() const;

    void forward(const float x[]);

    const int num_input_;  // the number of input layer neuron.
    const int num_hidden_; // the number of hidden layer nueron.
    const int num_output_; // the number of output layer nueron.

    // Input Layer
    std::unique_ptr<float[]> o1_; // input layer output (= input layer input)

    // Hidden Layer
    std::unique_ptr<float[]> i2_; // hidden layer input
    std::unique_ptr<float[]> o2_; // hidden layer output
    std::unique_ptr<float[]> e2_; // hidden layer error
    std::unique_ptr<float[]> w2_; // hidden layer weight

    // Output Layer
    std::unique_ptr<float[]> i3_; // output layer input (= output layer output)
    std::unique_ptr<float[]> e3_; // output layer error
    std::unique_ptr<float[]> w3_; // output layer weight
};

} // namespace learning

#endif // LEARNING_MULTILAYER_PERCEPTRON_H_
