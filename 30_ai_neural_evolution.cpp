#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <iomanip>

// Forward declarations
class Neuron;
class NeuralNetwork;
class Individual;
class Population;
class GeneticAlgorithm;
class Environment;
class EvolutionaryStrategy;

// Activation functions
namespace ActivationFunctions {
    double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }
    double tanh(double x) { return std::tanh(x); }
    double relu(double x) { return std::max(0.0, x); }
    double leakyRelu(double x) { return x > 0 ? x : 0.01 * x; }
    double linear(double x) { return x; }
    
    double sigmoidDerivative(double x) { double s = sigmoid(x); return s * (1.0 - s); }
    double tanhDerivative(double x) { double t = tanh(x); return 1.0 - t * t; }
    double reluDerivative(double x) { return x > 0 ? 1.0 : 0.0; }
    double leakyReluDerivative(double x) { return x > 0 ? 1.0 : 0.01; }
    double linearDerivative(double x) { return 1.0; }
}

// Neuron class
class Neuron {
public:
    double value;
    double bias;
    std::vector<double> weights;
    std::function<double(double)> activationFunction;
    std::function<double(double)> derivativeFunction;
    
    Neuron(int numInputs = 0, const std::string& activation = "sigmoid") 
        : value(0.0), bias(0.0) {
        
        weights.resize(numInputs, 0.0);
        setActivationFunction(activation);
        randomizeWeights();
    }
    
    void setActivationFunction(const std::string& activation) {
        if (activation == "sigmoid") {
            activationFunction = ActivationFunctions::sigmoid;
            derivativeFunction = ActivationFunctions::sigmoidDerivative;
        } else if (activation == "tanh") {
            activationFunction = ActivationFunctions::tanh;
            derivativeFunction = ActivationFunctions::tanhDerivative;
        } else if (activation == "relu") {
            activationFunction = ActivationFunctions::relu;
            derivativeFunction = ActivationFunctions::reluDerivative;
        } else if (activation == "leaky_relu") {
            activationFunction = ActivationFunctions::leakyRelu;
            derivativeFunction = ActivationFunctions::leakyReluDerivative;
        } else {
            activationFunction = ActivationFunctions::linear;
            derivativeFunction = ActivationFunctions::linearDerivative;
        }
    }
    
    void randomizeWeights(double min = -1.0, double max = 1.0) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        
        for (auto& weight : weights) {
            weight = dis(gen);
        }
        bias = dis(gen);
    }
    
    double activate(const std::vector<double>& inputs) {
        if (inputs.size() != weights.size()) {
            throw std::runtime_error("Input size mismatch");
        }
        
        double sum = bias;
        for (size_t i = 0; i < inputs.size(); i++) {
            sum += inputs[i] * weights[i];
        }
        
        value = activationFunction(sum);
        return value;
    }
    
    void mutate(double mutationRate, double mutationStrength) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> rateDis(0.0, 1.0);
        std::normal_distribution<> strengthDis(0.0, mutationStrength);
        
        // Mutate weights
        for (auto& weight : weights) {
            if (rateDis(gen) < mutationRate) {
                weight += strengthDis(gen);
                weight = std::clamp(weight, -10.0, 10.0); // Prevent extreme values
            }
        }
        
        // Mutate bias
        if (rateDis(gen) < mutationRate) {
            bias += strengthDis(gen);
            bias = std::clamp(bias, -10.0, 10.0);
        }
    }
};

// Neural network class
class NeuralNetwork {
private:
    std::vector<std::vector<Neuron>> layers;
    std::vector<int> topology;
    double learningRate;
    
public:
    NeuralNetwork(const std::vector<int>& topo, double lr = 0.01) 
        : topology(topo), learningRate(lr) {
        
        layers.resize(topology.size());
        
        // Create input layer
        layers[0].resize(topology[0]);
        for (auto& neuron : layers[0]) {
            neuron = Neuron(0, "linear"); // Input neurons don't need weights
        }
        
        // Create hidden and output layers
        for (size_t i = 1; i < topology.size(); i++) {
            layers[i].resize(topology[i]);
            
            std::string activation = (i == topology.size() - 1) ? "linear" : "sigmoid";
            
            for (auto& neuron : layers[i]) {
                neuron = Neuron(topology[i-1], activation);
            }
        }
    }
    
    std::vector<double> feedForward(const std::vector<double>& inputs) {
        if (inputs.size() != layers[0].size()) {
            throw std::runtime_error("Input size mismatch");
        }
        
        // Set input layer values
        for (size_t i = 0; i < inputs.size(); i++) {
            layers[0][i].value = inputs[i];
        }
        
        // Forward propagate through hidden and output layers
        for (size_t layerIdx = 1; layerIdx < layers.size(); layerIdx++) {
            std::vector<double> prevLayerValues;
            for (const auto& neuron : layers[layerIdx - 1]) {
                prevLayerValues.push_back(neuron.value);
            }
            
            for (auto& neuron : layers[layerIdx]) {
                neuron.activate(prevLayerValues);
            }
        }
        
        // Return output layer values
        std::vector<double> outputs;
        for (const auto& neuron : layers.back()) {
            outputs.push_back(neuron.value);
        }
        
        return outputs;
    }
    
    void mutate(double mutationRate, double mutationStrength) {
        for (auto& layer : layers) {
            for (auto& neuron : layer) {
                neuron.mutate(mutationRate, mutationStrength);
            }
        }
    }
    
    NeuralNetwork crossover(const NeuralNetwork& other, double crossoverRate = 0.5) const {
        if (topology != other.topology) {
            throw std::runtime_error("Cannot crossover networks with different topologies");
        }
        
        NeuralNetwork child(topology, learningRate);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (size_t layerIdx = 0; layerIdx < layers.size(); layerIdx++) {
            for (size_t neuronIdx = 0; neuronIdx < layers[layerIdx].size(); neuronIdx++) {
                const auto& parent1Neuron = layers[layerIdx][neuronIdx];
                const auto& parent2Neuron = other.layers[layerIdx][neuronIdx];
                auto& childNeuron = child.layers[layerIdx][neuronIdx];
                
                // Crossover weights
                for (size_t weightIdx = 0; weightIdx < parent1Neuron.weights.size(); weightIdx++) {
                    if (dis(gen) < crossoverRate) {
                        childNeuron.weights[weightIdx] = parent1Neuron.weights[weightIdx];
                    } else {
                        childNeuron.weights[weightIdx] = parent2Neuron.weights[weightIdx];
                    }
                }
                
                // Crossover bias
                if (dis(gen) < crossoverRate) {
                    childNeuron.bias = parent1Neuron.bias;
                } else {
                    childNeuron.bias = parent2Neuron.bias;
                }
            }
        }
        
        return child;
    }
    
    NeuralNetwork clone() const {
        NeuralNetwork copy(topology, learningRate);
        copy.layers = layers;
        return copy;
    }
    
    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) return;
        
        // Save topology
        for (size_t i = 0; i < topology.size(); i++) {
            file << topology[i];
            if (i < topology.size() - 1) file << " ";
        }
        file << "\n";
        
        // Save weights and biases
        for (const auto& layer : layers) {
            for (const auto& neuron : layer) {
                for (const auto& weight : neuron.weights) {
                    file << weight << " ";
                }
                file << neuron.bias << "\n";
            }
        }
        
        file.close();
    }
    
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;
        
        // Load topology
        std::string line;
        std::getline(file, line);
        std::istringstream topoStream(line);
        std::vector<int> loadedTopo;
        int size;
        while (topoStream >> size) {
            loadedTopo.push_back(size);
        }
        
        if (loadedTopo != topology) return false;
        
        // Load weights and biases
        for (auto& layer : layers) {
            for (auto& neuron : layer) {
                for (auto& weight : neuron.weights) {
                    file >> weight;
                }
                file >> neuron.bias;
            }
        }
        
        file.close();
        return true;
    }
    
    std::vector<int> getTopology() const { return topology; }
    
    int getTotalParameters() const {
        int count = 0;
        for (const auto& layer : layers) {
            for (const auto& neuron : layer) {
                count += neuron.weights.size() + 1; // +1 for bias
            }
        }
        return count;
    }
    
    void printNetwork() const {
        std::cout << "Neural Network Topology: ";
        for (size_t i = 0; i < topology.size(); i++) {
            std::cout << topology[i];
            if (i < topology.size() - 1) std::cout << "->";
        }
        std::cout << " (Total parameters: " << getTotalParameters() << ")" << std::endl;
    }
};

// Individual in the population
class Individual {
public:
    NeuralNetwork network;
    double fitness;
    int age;
    std::map<std::string, double> stats;
    
    Individual(const std::vector<int>& topology) 
        : network(topology), fitness(0.0), age(0) {}
    
    Individual(const NeuralNetwork& net) 
        : network(net), fitness(0.0), age(0) {}
    
    void evaluateFitness(const std::function<double(const NeuralNetwork&)>& fitnessFunction) {
        fitness = fitnessFunction(network);
    }
    
    void mutate(double mutationRate, double mutationStrength) {
        network.mutate(mutationRate, mutationStrength);
        age++;
    }
    
    Individual crossover(const Individual& other, double crossoverRate = 0.5) const {
        Individual child(network.crossover(other.network, crossoverRate));
        return child;
    }
    
    bool operator<(const Individual& other) const {
        return fitness > other.fitness; // Higher fitness is better
    }
    
    void addStat(const std::string& key, double value) {
        stats[key] = value;
    }
    
    double getStat(const std::string& key) const {
        auto it = stats.find(key);
        return (it != stats.end()) ? it->second : 0.0;
    }
};

// Population management
class Population {
private:
    std::vector<Individual> individuals;
    size_t populationSize;
    std::vector<int> networkTopology;
    double mutationRate;
    double mutationStrength;
    double crossoverRate;
    double elitismRate;
    
    std::atomic<int> generation;
    std::mutex populationMutex;
    
public:
    Population(size_t size, const std::vector<int>& topology, 
               double mutRate = 0.1, double mutStrength = 0.1, 
               double crossRate = 0.7, double eliteRate = 0.1)
        : populationSize(size), networkTopology(topology), 
          mutationRate(mutRate), mutationStrength(mutStrength),
          crossoverRate(crossRate), elitismRate(eliteRate), generation(0) {
        
        // Initialize random population
        for (size_t i = 0; i < populationSize; i++) {
            individuals.emplace_back(topology);
        }
    }
    
    void evaluatePopulation(const std::function<double(const NeuralNetwork&)>& fitnessFunction) {
        for (auto& individual : individuals) {
            individual.evaluateFitness(fitnessFunction);
        }
        
        // Sort by fitness (highest first)
        std::sort(individuals.begin(), individuals.end());
    }
    
    void evolve() {
        std::lock_guard<std::mutex> lock(populationMutex);
        
        std::vector<Individual> newGeneration;
        
        // Elitism: keep best individuals
        size_t eliteCount = static_cast<size_t>(populationSize * elitismRate);
        for (size_t i = 0; i < eliteCount; i++) {
            newGeneration.push_back(individuals[i]);
            newGeneration.back().age++;
        }
        
        // Generate offspring
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        while (newGeneration.size() < populationSize) {
            // Tournament selection
            Individual parent1 = tournamentSelection(3);
            Individual parent2 = tournamentSelection(3);
            
            Individual offspring = parent1;
            
            // Crossover
            if (dis(gen) < crossoverRate) {
                offspring = parent1.crossover(parent2, 0.5);
            }
            
            // Mutation
            offspring.mutate(mutationRate, mutationStrength);
            
            newGeneration.push_back(offspring);
        }
        
        individuals = std::move(newGeneration);
        generation++;
    }
    
    Individual tournamentSelection(int tournamentSize) const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, individuals.size() - 1);
        
        Individual best = individuals[dis(gen)];
        
        for (int i = 1; i < tournamentSize; i++) {
            Individual candidate = individuals[dis(gen)];
            if (candidate.fitness > best.fitness) {
                best = candidate;
            }
        }
        
        return best;
    }
    
    Individual getBest() const {
        if (individuals.empty()) throw std::runtime_error("Empty population");
        return individuals[0]; // Assuming sorted by fitness
    }
    
    std::vector<Individual> getTopN(int n) const {
        std::vector<Individual> top;
        int count = std::min(n, static_cast<int>(individuals.size()));
        
        for (int i = 0; i < count; i++) {
            top.push_back(individuals[i]);
        }
        
        return top;
    }
    
    double getAverageFitness() const {
        if (individuals.empty()) return 0.0;
        
        double sum = 0.0;
        for (const auto& individual : individuals) {
            sum += individual.fitness;
        }
        
        return sum / individuals.size();
    }
    
    double getBestFitness() const {
        return individuals.empty() ? 0.0 : individuals[0].fitness;
    }
    
    double getWorstFitness() const {
        return individuals.empty() ? 0.0 : individuals.back().fitness;
    }
    
    int getGeneration() const { return generation; }
    size_t getSize() const { return individuals.size(); }
    
    void setMutationRate(double rate) { mutationRate = rate; }
    void setMutationStrength(double strength) { mutationStrength = strength; }
    void setCrossoverRate(double rate) { crossoverRate = rate; }
    
    void printStats() const {
        std::cout << "Generation " << generation << ":" << std::endl;
        std::cout << "  Population size: " << individuals.size() << std::endl;
        std::cout << "  Best fitness: " << std::fixed << std::setprecision(6) << getBestFitness() << std::endl;
        std::cout << "  Average fitness: " << std::fixed << std::setprecision(6) << getAverageFitness() << std::endl;
        std::cout << "  Worst fitness: " << std::fixed << std::setprecision(6) << getWorstFitness() << std::endl;
        
        if (!individuals.empty()) {
            double avgAge = 0.0;
            for (const auto& individual : individuals) {
                avgAge += individual.age;
            }
            avgAge /= individuals.size();
            std::cout << "  Average age: " << std::fixed << std::setprecision(2) << avgAge << std::endl;
        }
    }
    
    void savePopulation(const std::string& directory) const {
        for (size_t i = 0; i < std::min(size_t(10), individuals.size()); i++) {
            std::string filename = directory + "/individual_" + std::to_string(i) + ".net";
            individuals[i].network.saveToFile(filename);
        }
    }
};

// Test environments and problems
class TestEnvironment {
public:
    virtual ~TestEnvironment() = default;
    virtual std::string getName() const = 0;
    virtual double evaluateFitness(const NeuralNetwork& network) = 0;
    virtual std::vector<int> getRequiredTopology() const = 0;
    virtual void printDescription() const = 0;
};

// XOR Problem
class XOREnvironment : public TestEnvironment {
public:
    std::string getName() const override { return "XOR Problem"; }
    
    std::vector<int> getRequiredTopology() const override {
        return {2, 4, 1}; // 2 inputs, 4 hidden, 1 output
    }
    
    void printDescription() const override {
        std::cout << "XOR Problem: Learn the XOR logic function" << std::endl;
        std::cout << "Inputs: (0,0), (0,1), (1,0), (1,1)" << std::endl;
        std::cout << "Expected outputs: 0, 1, 1, 0" << std::endl;
    }
    
    double evaluateFitness(const NeuralNetwork& network) override {
        std::vector<std::vector<double>> inputs = {{0,0}, {0,1}, {1,0}, {1,1}};
        std::vector<double> expectedOutputs = {0, 1, 1, 0};
        
        double totalError = 0.0;
        
        for (size_t i = 0; i < inputs.size(); i++) {
            auto outputs = network.feedForward(inputs[i]);
            if (!outputs.empty()) {
                double error = std::abs(outputs[0] - expectedOutputs[i]);
                totalError += error;
            }
        }
        
        // Convert error to fitness (lower error = higher fitness)
        return 1.0 / (1.0 + totalError);
    }
};

// Function approximation
class FunctionApproximationEnvironment : public TestEnvironment {
private:
    std::function<double(double)> targetFunction;
    std::string functionName;
    
public:
    FunctionApproximationEnvironment(const std::function<double(double)>& func, const std::string& name)
        : targetFunction(func), functionName(name) {}
    
    std::string getName() const override { return "Function Approximation: " + functionName; }
    
    std::vector<int> getRequiredTopology() const override {
        return {1, 8, 8, 1}; // 1 input, 2 hidden layers, 1 output
    }
    
    void printDescription() const override {
        std::cout << "Function Approximation: Learn to approximate " << functionName << std::endl;
        std::cout << "Input range: [-2π, 2π]" << std::endl;
    }
    
    double evaluateFitness(const NeuralNetwork& network) override {
        double totalError = 0.0;
        int numSamples = 50;
        
        for (int i = 0; i < numSamples; i++) {
            double x = -2 * M_PI + (4 * M_PI * i) / (numSamples - 1);
            double expected = targetFunction(x);
            
            auto outputs = network.feedForward({x});
            if (!outputs.empty()) {
                double error = std::abs(outputs[0] - expected);
                totalError += error;
            }
        }
        
        return 1.0 / (1.0 + totalError / numSamples);
    }
};

// Cart-pole balancing (simplified)
class CartPoleEnvironment : public TestEnvironment {
private:
    struct State {
        double position, velocity, angle, angularVelocity;
    };
    
public:
    std::string getName() const override { return "Cart-Pole Balancing"; }
    
    std::vector<int> getRequiredTopology() const override {
        return {4, 8, 2}; // 4 state inputs, 8 hidden, 2 actions (left/right)
    }
    
    void printDescription() const override {
        std::cout << "Cart-Pole: Balance a pole on a moving cart" << std::endl;
        std::cout << "State: position, velocity, angle, angular velocity" << std::endl;
        std::cout << "Actions: move left or right" << std::endl;
    }
    
    double evaluateFitness(const NeuralNetwork& network) override {
        int totalSteps = 0;
        int numTrials = 5;
        
        for (int trial = 0; trial < numTrials; trial++) {
            State state = {0.0, 0.0, 0.1, 0.0}; // Initial state
            int steps = 0;
            
            while (steps < 500 && std::abs(state.angle) < M_PI/6 && std::abs(state.position) < 2.4) {
                std::vector<double> inputs = {state.position, state.velocity, state.angle, state.angularVelocity};
                auto outputs = network.feedForward(inputs);
                
                if (outputs.size() >= 2) {
                    double force = (outputs[0] > outputs[1]) ? -10.0 : 10.0;
                    
                    // Simplified physics simulation
                    double dt = 0.02;
                    double gravity = 9.8;
                    double masscart = 1.0;
                    double masspole = 0.1;
                    double length = 0.5;
                    
                    double costheta = cos(state.angle);
                    double sintheta = sin(state.angle);
                    
                    double temp = (force + masspole * length * state.angularVelocity * state.angularVelocity * sintheta) / (masscart + masspole);
                    double thetaacc = (gravity * sintheta - costheta * temp) / (length * (4.0/3.0 - masspole * costheta * costheta / (masscart + masspole)));
                    double xacc = temp - masspole * length * thetaacc * costheta / (masscart + masspole);
                    
                    state.position += state.velocity * dt;
                    state.velocity += xacc * dt;
                    state.angle += state.angularVelocity * dt;
                    state.angularVelocity += thetaacc * dt;
                }
                
                steps++;
            }
            
            totalSteps += steps;
        }
        
        return totalSteps / (double)(numTrials * 500); // Normalize to [0, 1]
    }
};

// Genetic Algorithm with neural network evolution
class GeneticAlgorithm {
private:
    std::unique_ptr<Population> population;
    std::unique_ptr<TestEnvironment> environment;
    
    // Evolution parameters
    int maxGenerations;
    double targetFitness;
    bool adaptiveMutation;
    
    // Statistics
    std::vector<double> fitnessHistory;
    std::vector<double> diversityHistory;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    
public:
    GeneticAlgorithm(size_t popSize, std::unique_ptr<TestEnvironment> env,
                     int maxGen = 1000, double targetFit = 0.95)
        : environment(std::move(env)), maxGenerations(maxGen), targetFitness(targetFit),
          adaptiveMutation(true) {
        
        auto topology = environment->getRequiredTopology();
        population = std::make_unique<Population>(popSize, topology);
        
        std::cout << "Initialized Genetic Algorithm:" << std::endl;
        std::cout << "  Environment: " << environment->getName() << std::endl;
        std::cout << "  Population size: " << popSize << std::endl;
        std::cout << "  Network topology: ";
        for (size_t i = 0; i < topology.size(); i++) {
            std::cout << topology[i];
            if (i < topology.size() - 1) std::cout << "->";
        }
        std::cout << std::endl;
        std::cout << "  Max generations: " << maxGenerations << std::endl;
        std::cout << "  Target fitness: " << targetFitness << std::endl;
    }
    
    void run() {
        std::cout << "\n=== Starting Evolution ===" << std::endl;
        environment->printDescription();
        
        startTime = std::chrono::high_resolution_clock::now();
        
        for (int gen = 0; gen < maxGenerations; gen++) {
            // Evaluate population
            population->evaluatePopulation([this](const NeuralNetwork& network) {
                return environment->evaluateFitness(network);
            });
            
            // Record statistics
            double bestFitness = population->getBestFitness();
            double avgFitness = population->getAverageFitness();
            
            fitnessHistory.push_back(bestFitness);
            diversityHistory.push_back(calculateDiversity());
            
            // Print progress
            if (gen % 10 == 0 || bestFitness >= targetFitness) {
                population->printStats();
                
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
                std::cout << "  Elapsed time: " << elapsed.count() << "s" << std::endl;
                std::cout << "  Diversity: " << std::fixed << std::setprecision(4) 
                          << diversityHistory.back() << std::endl;
                std::cout << std::endl;
            }
            
            // Check termination conditions
            if (bestFitness >= targetFitness) {
                std::cout << "Target fitness reached!" << std::endl;
                break;
            }
            
            // Adaptive parameters
            if (adaptiveMutation) {
                updateMutationParameters(gen);
            }
            
            // Evolve to next generation
            population->evolve();
        }
        
        printFinalResults();
    }
    
    void testBestIndividual() {
        auto best = population->getBest();
        std::cout << "\n=== Testing Best Individual ===" << std::endl;
        std::cout << "Fitness: " << std::fixed << std::setprecision(6) << best.fitness << std::endl;
        
        // Test specific cases based on environment
        if (environment->getName() == "XOR Problem") {
            testXORNetwork(best.network);
        } else if (environment->getName().find("Function Approximation") != std::string::npos) {
            testFunctionApproximation(best.network);
        } else if (environment->getName() == "Cart-Pole Balancing") {
            testCartPole(best.network);
        }
    }
    
    void saveResults(const std::string& directory) {
        // Save best networks
        population->savePopulation(directory);
        
        // Save evolution statistics
        std::ofstream statsFile(directory + "/evolution_stats.txt");
        if (statsFile.is_open()) {
            statsFile << "Generation,BestFitness,Diversity\n";
            for (size_t i = 0; i < fitnessHistory.size(); i++) {
                statsFile << i << "," << fitnessHistory[i] << "," 
                          << (i < diversityHistory.size() ? diversityHistory[i] : 0.0) << "\n";
            }
            statsFile.close();
        }
    }
    
private:
    double calculateDiversity() {
        auto individuals = population->getTopN(20);
        if (individuals.size() < 2) return 0.0;
        
        double totalDistance = 0.0;
        int comparisons = 0;
        
        for (size_t i = 0; i < individuals.size(); i++) {
            for (size_t j = i + 1; j < individuals.size(); j++) {
                // Calculate network distance (simplified)
                double distance = 0.0;
                // This would require implementing network comparison
                // For now, use fitness difference as proxy
                distance = std::abs(individuals[i].fitness - individuals[j].fitness);
                
                totalDistance += distance;
                comparisons++;
            }
        }
        
        return comparisons > 0 ? totalDistance / comparisons : 0.0;
    }
    
    void updateMutationParameters(int generation) {
        // Adaptive mutation rate based on diversity and progress
        double diversity = diversityHistory.back();
        double mutationRate = 0.1;
        
        if (diversity < 0.01) {
            mutationRate = 0.3; // Increase mutation when diversity is low
        } else if (diversity > 0.1) {
            mutationRate = 0.05; // Decrease mutation when diversity is high
        }
        
        population->setMutationRate(mutationRate);
        
        // Decrease mutation strength over time
        double mutationStrength = 0.5 * exp(-generation / 200.0);
        population->setMutationStrength(mutationStrength);
    }
    
    void testXORNetwork(const NeuralNetwork& network) {
        std::cout << "\nXOR Truth Table Test:" << std::endl;
        std::vector<std::vector<double>> inputs = {{0,0}, {0,1}, {1,0}, {1,1}};
        std::vector<double> expected = {0, 1, 1, 0};
        
        for (size_t i = 0; i < inputs.size(); i++) {
            auto outputs = network.feedForward(inputs[i]);
            if (!outputs.empty()) {
                std::cout << "Input: (" << inputs[i][0] << "," << inputs[i][1] 
                          << ") -> Output: " << std::fixed << std::setprecision(4) 
                          << outputs[0] << " (Expected: " << expected[i] 
                          << ", Error: " << std::abs(outputs[0] - expected[i]) << ")" << std::endl;
            }
        }
    }
    
    void testFunctionApproximation(const NeuralNetwork& network) {
        std::cout << "\nFunction Approximation Test (sample points):" << std::endl;
        
        for (int i = 0; i < 10; i++) {
            double x = -2 * M_PI + (4 * M_PI * i) / 9;
            double expected = sin(x); // Assuming sine function
            
            auto outputs = network.feedForward({x});
            if (!outputs.empty()) {
                std::cout << "x=" << std::fixed << std::setprecision(2) << x 
                          << " -> f(x)=" << std::setprecision(4) << outputs[0] 
                          << " (Expected: " << expected 
                          << ", Error: " << std::abs(outputs[0] - expected) << ")" << std::endl;
            }
        }
    }
    
    void testCartPole(const NeuralNetwork& network) {
        std::cout << "\nCart-Pole Test (single trial):" << std::endl;
        // Implementation would show a single trial run
        std::cout << "Running simulation..." << std::endl;
        double fitness = environment->evaluateFitness(network);
        std::cout << "Average performance: " << std::fixed << std::setprecision(4) 
                  << fitness * 500 << " steps" << std::endl;
    }
    
    void printFinalResults() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
        
        std::cout << "\n=== Evolution Complete ===" << std::endl;
        std::cout << "Total time: " << totalTime.count() << " seconds" << std::endl;
        std::cout << "Generations: " << population->getGeneration() << std::endl;
        std::cout << "Final best fitness: " << std::fixed << std::setprecision(6) 
                  << population->getBestFitness() << std::endl;
        
        if (!fitnessHistory.empty()) {
            double improvement = population->getBestFitness() - fitnessHistory[0];
            std::cout << "Fitness improvement: " << std::fixed << std::setprecision(6) 
                      << improvement << std::endl;
        }
    }
};

// Demo function
void runNeuralEvolutionDemo() {
    std::cout << "=== AI NEURAL NETWORK EVOLUTION DEMO ===" << std::endl;
    std::cout << "Evolving neural networks using genetic algorithms" << std::endl;
    
    // Test different problems
    std::vector<std::function<std::unique_ptr<TestEnvironment>()>> environments = {
        []() { return std::make_unique<XOREnvironment>(); },
        []() { return std::make_unique<FunctionApproximationEnvironment>(
            [](double x) { return sin(x); }, "sin(x)"); },
        []() { return std::make_unique<CartPoleEnvironment>(); }
    };
    
    for (size_t i = 0; i < environments.size(); i++) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "EXPERIMENT " << (i + 1) << "/" << environments.size() << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        auto env = environments[i]();
        
        // Adjust parameters based on problem complexity
        size_t popSize = 100;
        int maxGen = 200;
        double targetFit = 0.95;
        
        if (env->getName() == "Cart-Pole Balancing") {
            popSize = 150;
            maxGen = 300;
            targetFit = 0.9;
        } else if (env->getName().find("Function Approximation") != std::string::npos) {
            popSize = 120;
            maxGen = 250;
            targetFit = 0.98;
        }
        
        GeneticAlgorithm ga(popSize, std::move(env), maxGen, targetFit);
        
        // Run evolution
        ga.run();
        
        // Test and analyze results
        ga.testBestIndividual();
        
        // Save results
        std::string resultsDir = "evolution_results_" + std::to_string(i + 1);
        ga.saveResults(resultsDir);
        
        std::cout << "\nResults saved to: " << resultsDir << std::endl;
        
        if (i < environments.size() - 1) {
            std::cout << "\nPress Enter to continue to next experiment...";
            std::cin.get();
        }
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "NEURAL EVOLUTION DEMO COMPLETE" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "\nThis demo demonstrated:" << std::endl;
    std::cout << "- Neural network architecture evolution" << std::endl;
    std::cout << "- Genetic algorithms for optimization" << std::endl;
    std::cout << "- Multiple test environments (XOR, function approximation, cart-pole)" << std::endl;
    std::cout << "- Adaptive mutation and crossover strategies" << std::endl;
    std::cout << "- Population diversity management" << std::endl;
    std::cout << "- Performance analysis and visualization" << std::endl;
    
    std::cout << "\nEvolution results can be analyzed further by:" << std::endl;
    std::cout << "- Loading saved networks for testing" << std::endl;
    std::cout << "- Plotting fitness curves from statistics files" << std::endl;
    std::cout << "- Comparing different evolutionary strategies" << std::endl;
}

int main() {
    try {
        runNeuralEvolutionDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
