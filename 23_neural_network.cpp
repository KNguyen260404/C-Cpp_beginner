#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <iomanip>

class NeuralNetwork {
private:
    struct Layer {
        std::vector<std::vector<double>> weights;
        std::vector<double> biases;
        std::vector<double> activations;
        std::vector<double> errors;
    };

    std::vector<Layer> layers;
    double learningRate;
    std::mt19937 rng;

    // Activation function (sigmoid)
    double sigmoid(double x) {
        return 1.0 / (1.0 + exp(-x));
    }

    // Derivative of sigmoid
    double sigmoidDerivative(double x) {
        double sig = sigmoid(x);
        return sig * (1.0 - sig);
    }

public:
    NeuralNetwork(const std::vector<int>& topology, double lr = 0.1) 
        : learningRate(lr), rng(std::random_device{}()) {
        
        // Initialize layers based on topology
        for (size_t i = 0; i < topology.size(); ++i) {
            Layer layer;
            
            // Initialize activations for this layer
            layer.activations.resize(topology[i], 0.0);
            layer.errors.resize(topology[i], 0.0);
            
            // Initialize weights and biases if not the last layer
            if (i < topology.size() - 1) {
                std::uniform_real_distribution<double> dist(-1.0, 1.0);
                
                // Initialize weights between this layer and next layer
                layer.weights.resize(topology[i], std::vector<double>(topology[i+1]));
                for (int j = 0; j < topology[i]; ++j) {
                    for (int k = 0; k < topology[i+1]; ++k) {
                        layer.weights[j][k] = dist(rng);
                    }
                }
                
                // Initialize biases for the next layer
                layer.biases.resize(topology[i+1], 0.0);
                for (int j = 0; j < topology[i+1]; ++j) {
                    layer.biases[j] = dist(rng);
                }
            }
            
            layers.push_back(layer);
        }
    }

    // Forward propagation
    std::vector<double> feedForward(const std::vector<double>& inputs) {
        // Set input layer activations
        for (size_t i = 0; i < inputs.size(); ++i) {
            layers[0].activations[i] = inputs[i];
        }
        
        // Process hidden and output layers
        for (size_t i = 0; i < layers.size() - 1; ++i) {
            Layer& currentLayer = layers[i];
            Layer& nextLayer = layers[i + 1];
            
            // Calculate activations for next layer
            for (size_t j = 0; j < nextLayer.activations.size(); ++j) {
                double sum = nextLayer.biases[j];
                
                for (size_t k = 0; k < currentLayer.activations.size(); ++k) {
                    sum += currentLayer.activations[k] * currentLayer.weights[k][j];
                }
                
                nextLayer.activations[j] = sigmoid(sum);
            }
        }
        
        // Return output layer activations
        return layers.back().activations;
    }

    // Backpropagation
    void backPropagate(const std::vector<double>& targets) {
        // Calculate output layer errors
        Layer& outputLayer = layers.back();
        for (size_t i = 0; i < outputLayer.activations.size(); ++i) {
            double output = outputLayer.activations[i];
            outputLayer.errors[i] = (targets[i] - output) * output * (1.0 - output);
        }
        
        // Calculate hidden layer errors (backpropagation)
        for (int i = layers.size() - 2; i >= 0; --i) {
            Layer& currentLayer = layers[i];
            Layer& nextLayer = layers[i + 1];
            
            // Calculate errors for current layer
            for (size_t j = 0; j < currentLayer.activations.size(); ++j) {
                double error = 0.0;
                
                for (size_t k = 0; k < nextLayer.errors.size(); ++k) {
                    error += nextLayer.errors[k] * currentLayer.weights[j][k];
                }
                
                currentLayer.errors[j] = error * currentLayer.activations[j] * (1.0 - currentLayer.activations[j]);
            }
        }
        
        // Update weights and biases
        for (size_t i = 0; i < layers.size() - 1; ++i) {
            Layer& currentLayer = layers[i];
            Layer& nextLayer = layers[i + 1];
            
            // Update weights
            for (size_t j = 0; j < currentLayer.activations.size(); ++j) {
                for (size_t k = 0; k < nextLayer.activations.size(); ++k) {
                    currentLayer.weights[j][k] += learningRate * nextLayer.errors[k] * currentLayer.activations[j];
                }
            }
            
            // Update biases
            for (size_t j = 0; j < nextLayer.biases.size(); ++j) {
                nextLayer.biases[j] += learningRate * nextLayer.errors[j];
            }
        }
    }

    // Train the network
    void train(const std::vector<std::vector<double>>& inputs, 
               const std::vector<std::vector<double>>& targets, 
               int epochs) {
        
        for (int e = 0; e < epochs; ++e) {
            double totalError = 0.0;
            
            // Create indices for shuffling
            std::vector<size_t> indices(inputs.size());
            for (size_t i = 0; i < indices.size(); ++i) {
                indices[i] = i;
            }
            std::shuffle(indices.begin(), indices.end(), rng);
            
            // Process each training sample
            for (size_t idx : indices) {
                // Forward pass
                std::vector<double> outputs = feedForward(inputs[idx]);
                
                // Calculate error
                for (size_t i = 0; i < outputs.size(); ++i) {
                    double error = targets[idx][i] - outputs[i];
                    totalError += error * error;
                }
                
                // Backward pass
                backPropagate(targets[idx]);
            }
            
            // Print progress every 100 epochs
            if (e % 100 == 0) {
                std::cout << "Epoch " << e << ", Error: " << totalError / inputs.size() << std::endl;
            }
        }
    }

    // Save the trained model
    void saveModel(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for saving model." << std::endl;
            return;
        }

        // Save number of layers
        file << layers.size() << std::endl;

        // Save layer sizes
        for (const auto& layer : layers) {
            file << layer.activations.size() << " ";
        }
        file << std::endl;

        // Save weights and biases
        for (size_t i = 0; i < layers.size() - 1; ++i) {
            // Save weights
            for (size_t j = 0; j < layers[i].weights.size(); ++j) {
                for (size_t k = 0; k < layers[i].weights[j].size(); ++k) {
                    file << layers[i].weights[j][k] << " ";
                }
                file << std::endl;
            }

            // Save biases
            for (size_t j = 0; j < layers[i].biases.size(); ++j) {
                file << layers[i].biases[j] << " ";
            }
            file << std::endl;
        }

        file.close();
    }

    // Load a trained model
    void loadModel(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for loading model." << std::endl;
            return;
        }

        // Clear existing layers
        layers.clear();

        // Read number of layers
        size_t numLayers;
        file >> numLayers;

        // Read layer sizes
        std::vector<int> topology(numLayers);
        for (size_t i = 0; i < numLayers; ++i) {
            file >> topology[i];
        }

        // Recreate network structure
        *this = NeuralNetwork(topology, learningRate);

        // Load weights and biases
        for (size_t i = 0; i < layers.size() - 1; ++i) {
            // Load weights
            for (size_t j = 0; j < layers[i].weights.size(); ++j) {
                for (size_t k = 0; k < layers[i].weights[j].size(); ++k) {
                    file >> layers[i].weights[j][k];
                }
            }

            // Load biases
            for (size_t j = 0; j < layers[i].biases.size(); ++j) {
                file >> layers[i].biases[j];
            }
        }

        file.close();
    }
};

// XOR problem dataset
void createXORDataset(std::vector<std::vector<double>>& inputs, 
                      std::vector<std::vector<double>>& targets) {
    // Input patterns
    inputs = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    
    // Expected outputs
    targets = {
        {0},
        {1},
        {1},
        {0}
    };
}

int main() {
    std::cout << "Neural Network Implementation" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Create a neural network with topology: 2 inputs, 4 hidden neurons, 1 output
    std::vector<int> topology = {2, 4, 1};
    NeuralNetwork nn(topology, 0.1);
    
    // Create XOR dataset
    std::vector<std::vector<double>> inputs, targets;
    createXORDataset(inputs, targets);
    
    // Train the network
    std::cout << "Training network on XOR problem..." << std::endl;
    nn.train(inputs, targets, 10000);
    
    // Test the network
    std::cout << "\nTesting network:" << std::endl;
    std::cout << "----------------" << std::endl;
    
    for (size_t i = 0; i < inputs.size(); ++i) {
        std::vector<double> output = nn.feedForward(inputs[i]);
        std::cout << "Input: [" << inputs[i][0] << ", " << inputs[i][1] 
                  << "], Expected: " << targets[i][0] 
                  << ", Output: " << std::fixed << std::setprecision(6) << output[0] << std::endl;
    }
    
    // Save the trained model
    nn.saveModel("xor_model.txt");
    std::cout << "\nModel saved to 'xor_model.txt'" << std::endl;
    
    // Load the model and test again
    std::cout << "\nLoading model and testing again:" << std::endl;
    NeuralNetwork loadedNN({2, 4, 1});
    loadedNN.loadModel("xor_model.txt");
    
    for (size_t i = 0; i < inputs.size(); ++i) {
        std::vector<double> output = loadedNN.feedForward(inputs[i]);
        std::cout << "Input: [" << inputs[i][0] << ", " << inputs[i][1] 
                  << "], Output: " << std::fixed << std::setprecision(6) << output[0] << std::endl;
    }
    
    // Advanced usage instructions
    std::cout << "\nAdvanced Usage:" << std::endl;
    std::cout << "To train on your own dataset:" << std::endl;
    std::cout << "1. Prepare your input and target vectors" << std::endl;
    std::cout << "2. Create a network with appropriate topology" << std::endl;
    std::cout << "3. Call train() with your dataset and desired epochs" << std::endl;
    std::cout << "4. Use feedForward() to make predictions" << std::endl;
    
    return 0;
} 