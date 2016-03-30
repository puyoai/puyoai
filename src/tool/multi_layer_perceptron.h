#ifndef TOOL_MULTILAYER_PERCEPTRON_H_
#define TOOL_MULTILAYER_PERCEPTRON_H_

#include <memory>

namespace learning {

// Defines a 3-layer perceptron.
// This is not thread-safe.
class MultiLayerPerceptron {
public:
    MultiLayerPerceptron(int num_input, int num_hidden, int num_output);
    ~MultiLayerPerceptron();

    // Train single data.
    // Returns true if the prediction is correct.
    bool train(int label, const float x[], float learning_rate = 0.1);

    // Returns the label.
    int predict(const float x[]);

private:
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

#endif // TOOL_MULTILAYER_PERCEPTRON_H_
