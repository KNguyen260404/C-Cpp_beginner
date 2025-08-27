#include <iostream>
#include <vector>
#include <complex>
#include <memory>
#include <string>
#include <map>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <sstream>
#include <fstream>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
class QuantumState;
class QuantumGate;
class QuantumCircuit;
class QuantumSimulator;
class QuantumAlgorithm;

// Complex number utilities
using Complex = std::complex<double>;
using Matrix = std::vector<std::vector<Complex>>;

// Quantum state representation
class QuantumState {
private:
    std::vector<Complex> amplitudes;
    int numQubits;
    
public:
    QuantumState(int n) : numQubits(n) {
        int size = 1 << n; // 2^n
        amplitudes.resize(size, Complex(0, 0));
        amplitudes[0] = Complex(1, 0); // Initialize to |00...0⟩
    }
    
    QuantumState(const std::vector<Complex>& amps) : amplitudes(amps) {
        numQubits = static_cast<int>(log2(amps.size()));
    }
    
    int getNumQubits() const { return numQubits; }
    int getSize() const { return amplitudes.size(); }
    
    Complex getAmplitude(int state) const {
        if (state >= 0 && state < amplitudes.size()) {
            return amplitudes[state];
        }
        return Complex(0, 0);
    }
    
    void setAmplitude(int state, const Complex& amp) {
        if (state >= 0 && state < amplitudes.size()) {
            amplitudes[state] = amp;
        }
    }
    
    std::vector<Complex> getAmplitudes() const { return amplitudes; }
    
    // Normalize the quantum state
    void normalize() {
        double norm = 0.0;
        for (const auto& amp : amplitudes) {
            norm += std::norm(amp);
        }
        norm = sqrt(norm);
        
        if (norm > 1e-10) {
            for (auto& amp : amplitudes) {
                amp /= norm;
            }
        }
    }
    
    // Calculate probability of measuring a specific state
    double getProbability(int state) const {
        if (state >= 0 && state < amplitudes.size()) {
            return std::norm(amplitudes[state]);
        }
        return 0.0;
    }
    
    // Measure the quantum state (collapse to classical state)
    int measure() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        double random = dis(gen);
        double cumulative = 0.0;
        
        for (int i = 0; i < amplitudes.size(); i++) {
            cumulative += getProbability(i);
            if (random <= cumulative) {
                // Collapse to this state
                std::fill(amplitudes.begin(), amplitudes.end(), Complex(0, 0));
                amplitudes[i] = Complex(1, 0);
                return i;
            }
        }
        
        return amplitudes.size() - 1;
    }
    
    // Measure specific qubit
    int measureQubit(int qubit) {
        if (qubit < 0 || qubit >= numQubits) return -1;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        // Calculate probability of measuring 0 or 1
        double prob0 = 0.0, prob1 = 0.0;
        
        for (int i = 0; i < amplitudes.size(); i++) {
            if ((i >> qubit) & 1) {
                prob1 += getProbability(i);
            } else {
                prob0 += getProbability(i);
            }
        }
        
        int result = (dis(gen) < prob0) ? 0 : 1;
        
        // Collapse the state
        for (int i = 0; i < amplitudes.size(); i++) {
            if (((i >> qubit) & 1) != result) {
                amplitudes[i] = Complex(0, 0);
            }
        }
        
        normalize();
        return result;
    }
    
    // Get classical representation of state
    std::string toBinaryString(int state) const {
        std::string result;
        for (int i = numQubits - 1; i >= 0; i--) {
            result += ((state >> i) & 1) ? '1' : '0';
        }
        return result;
    }
    
    // Print quantum state
    void print() const {
        std::cout << "Quantum State (|ψ⟩):" << std::endl;
        bool first = true;
        
        for (int i = 0; i < amplitudes.size(); i++) {
            if (std::abs(amplitudes[i]) > 1e-10) {
                if (!first) std::cout << " + ";
                
                double real = amplitudes[i].real();
                double imag = amplitudes[i].imag();
                
                if (std::abs(imag) < 1e-10) {
                    std::cout << std::fixed << std::setprecision(4) << real;
                } else if (std::abs(real) < 1e-10) {
                    std::cout << std::fixed << std::setprecision(4) << imag << "i";
                } else {
                    std::cout << "(" << std::fixed << std::setprecision(4) 
                              << real << " + " << imag << "i)";
                }
                
                std::cout << "|" << toBinaryString(i) << "⟩";
                first = false;
            }
        }
        
        if (first) {
            std::cout << "0|00...0⟩";
        }
        std::cout << std::endl;
    }
    
    // Print probability distribution
    void printProbabilities() const {
        std::cout << "Probability Distribution:" << std::endl;
        for (int i = 0; i < amplitudes.size(); i++) {
            double prob = getProbability(i);
            if (prob > 1e-10) {
                std::cout << "|" << toBinaryString(i) << "⟩: " 
                          << std::fixed << std::setprecision(4) << prob * 100 << "%" << std::endl;
            }
        }
    }
    
    // Calculate fidelity with another state
    double fidelity(const QuantumState& other) const {
        if (numQubits != other.numQubits) return 0.0;
        
        Complex overlap(0, 0);
        for (int i = 0; i < amplitudes.size(); i++) {
            overlap += std::conj(amplitudes[i]) * other.amplitudes[i];
        }
        
        return std::norm(overlap);
    }
};

// Quantum gate base class
class QuantumGate {
public:
    std::string name;
    std::vector<int> qubits;
    Matrix matrix;
    
    QuantumGate(const std::string& n, const std::vector<int>& q, const Matrix& m)
        : name(n), qubits(q), matrix(m) {}
    
    virtual ~QuantumGate() = default;
    
    virtual void apply(QuantumState& state) const = 0;
    virtual std::string toString() const {
        std::stringstream ss;
        ss << name << "(";
        for (size_t i = 0; i < qubits.size(); i++) {
            if (i > 0) ss << ", ";
            ss << qubits[i];
        }
        ss << ")";
        return ss.str();
    }
};

// Single qubit gates
class PauliX : public QuantumGate {
public:
    PauliX(int qubit) : QuantumGate("X", {qubit}, {{Complex(0,0), Complex(1,0)}, 
                                                   {Complex(1,0), Complex(0,0)}}) {}
    
    void apply(QuantumState& state) const override {
        int qubit = qubits[0];
        int size = state.getSize();
        std::vector<Complex> newAmps(size);
        
        for (int i = 0; i < size; i++) {
            int flipped = i ^ (1 << qubit);
            newAmps[i] = state.getAmplitude(flipped);
        }
        
        for (int i = 0; i < size; i++) {
            state.setAmplitude(i, newAmps[i]);
        }
    }
};

class PauliY : public QuantumGate {
public:
    PauliY(int qubit) : QuantumGate("Y", {qubit}, {{Complex(0,0), Complex(0,-1)}, 
                                                   {Complex(0,1), Complex(0,0)}}) {}
    
    void apply(QuantumState& state) const override {
        int qubit = qubits[0];
        int size = state.getSize();
        std::vector<Complex> newAmps(size);
        
        for (int i = 0; i < size; i++) {
            int flipped = i ^ (1 << qubit);
            if ((i >> qubit) & 1) {
                newAmps[i] = Complex(0, -1) * state.getAmplitude(flipped);
            } else {
                newAmps[i] = Complex(0, 1) * state.getAmplitude(flipped);
            }
        }
        
        for (int i = 0; i < size; i++) {
            state.setAmplitude(i, newAmps[i]);
        }
    }
};

class PauliZ : public QuantumGate {
public:
    PauliZ(int qubit) : QuantumGate("Z", {qubit}, {{Complex(1,0), Complex(0,0)}, 
                                                   {Complex(0,0), Complex(-1,0)}}) {}
    
    void apply(QuantumState& state) const override {
        int qubit = qubits[0];
        int size = state.getSize();
        
        for (int i = 0; i < size; i++) {
            if ((i >> qubit) & 1) {
                state.setAmplitude(i, -state.getAmplitude(i));
            }
        }
    }
};

class Hadamard : public QuantumGate {
public:
    Hadamard(int qubit) : QuantumGate("H", {qubit}, {{Complex(1/sqrt(2),0), Complex(1/sqrt(2),0)}, 
                                                     {Complex(1/sqrt(2),0), Complex(-1/sqrt(2),0)}}) {}
    
    void apply(QuantumState& state) const override {
        int qubit = qubits[0];
        int size = state.getSize();
        std::vector<Complex> newAmps(size);
        
        for (int i = 0; i < size; i++) {
            int flipped = i ^ (1 << qubit);
            Complex amp0 = state.getAmplitude(i & ~(1 << qubit));
            Complex amp1 = state.getAmplitude(i | (1 << qubit));
            
            if ((i >> qubit) & 1) {
                newAmps[i] = (amp0 - amp1) / sqrt(2);
            } else {
                newAmps[i] = (amp0 + amp1) / sqrt(2);
            }
        }
        
        for (int i = 0; i < size; i++) {
            state.setAmplitude(i, newAmps[i]);
        }
    }
};

class PhaseGate : public QuantumGate {
private:
    double phase;
    
public:
    PhaseGate(int qubit, double p) : QuantumGate("P", {qubit}, {}), phase(p) {}
    
    void apply(QuantumState& state) const override {
        int qubit = qubits[0];
        int size = state.getSize();
        Complex phaseComplex = std::exp(Complex(0, phase));
        
        for (int i = 0; i < size; i++) {
            if ((i >> qubit) & 1) {
                state.setAmplitude(i, state.getAmplitude(i) * phaseComplex);
            }
        }
    }
    
    std::string toString() const override {
        return "P(" + std::to_string(qubits[0]) + ", " + std::to_string(phase) + ")";
    }
};

class RotationX : public QuantumGate {
private:
    double angle;
    
public:
    RotationX(int qubit, double theta) : QuantumGate("RX", {qubit}, {}), angle(theta) {}
    
    void apply(QuantumState& state) const override {
        int qubit = qubits[0];
        int size = state.getSize();
        std::vector<Complex> newAmps(size);
        
        Complex cos_half = Complex(cos(angle/2), 0);
        Complex sin_half = Complex(0, -sin(angle/2));
        
        for (int i = 0; i < size; i++) {
            int flipped = i ^ (1 << qubit);
            Complex amp0 = state.getAmplitude(i & ~(1 << qubit));
            Complex amp1 = state.getAmplitude(i | (1 << qubit));
            
            if ((i >> qubit) & 1) {
                newAmps[i] = sin_half * amp0 + cos_half * amp1;
            } else {
                newAmps[i] = cos_half * amp0 + sin_half * amp1;
            }
        }
        
        for (int i = 0; i < size; i++) {
            state.setAmplitude(i, newAmps[i]);
        }
    }
    
    std::string toString() const override {
        return "RX(" + std::to_string(qubits[0]) + ", " + std::to_string(angle) + ")";
    }
};

// Two-qubit gates
class CNOT : public QuantumGate {
public:
    CNOT(int control, int target) : QuantumGate("CNOT", {control, target}, {}) {}
    
    void apply(QuantumState& state) const override {
        int control = qubits[0];
        int target = qubits[1];
        int size = state.getSize();
        
        for (int i = 0; i < size; i++) {
            if ((i >> control) & 1) { // Control qubit is 1
                int flipped = i ^ (1 << target);
                Complex temp = state.getAmplitude(i);
                state.setAmplitude(i, state.getAmplitude(flipped));
                state.setAmplitude(flipped, temp);
            }
        }
    }
};

class CZ : public QuantumGate {
public:
    CZ(int control, int target) : QuantumGate("CZ", {control, target}, {}) {}
    
    void apply(QuantumState& state) const override {
        int control = qubits[0];
        int target = qubits[1];
        int size = state.getSize();
        
        for (int i = 0; i < size; i++) {
            if (((i >> control) & 1) && ((i >> target) & 1)) {
                state.setAmplitude(i, -state.getAmplitude(i));
            }
        }
    }
};

class SWAP : public QuantumGate {
public:
    SWAP(int qubit1, int qubit2) : QuantumGate("SWAP", {qubit1, qubit2}, {}) {}
    
    void apply(QuantumState& state) const override {
        int q1 = qubits[0];
        int q2 = qubits[1];
        int size = state.getSize();
        
        for (int i = 0; i < size; i++) {
            int bit1 = (i >> q1) & 1;
            int bit2 = (i >> q2) & 1;
            
            if (bit1 != bit2) {
                int swapped = i ^ (1 << q1) ^ (1 << q2);
                if (i < swapped) { // Avoid double swapping
                    Complex temp = state.getAmplitude(i);
                    state.setAmplitude(i, state.getAmplitude(swapped));
                    state.setAmplitude(swapped, temp);
                }
            }
        }
    }
};

// Quantum circuit
class QuantumCircuit {
private:
    int numQubits;
    std::vector<std::unique_ptr<QuantumGate>> gates;
    
public:
    QuantumCircuit(int n) : numQubits(n) {}
    
    void addGate(std::unique_ptr<QuantumGate> gate) {
        gates.push_back(std::move(gate));
    }
    
    // Convenience methods for adding gates
    void addX(int qubit) { addGate(std::make_unique<PauliX>(qubit)); }
    void addY(int qubit) { addGate(std::make_unique<PauliY>(qubit)); }
    void addZ(int qubit) { addGate(std::make_unique<PauliZ>(qubit)); }
    void addH(int qubit) { addGate(std::make_unique<Hadamard>(qubit)); }
    void addPhase(int qubit, double phase) { addGate(std::make_unique<PhaseGate>(qubit, phase)); }
    void addRX(int qubit, double angle) { addGate(std::make_unique<RotationX>(qubit, angle)); }
    void addCNOT(int control, int target) { addGate(std::make_unique<CNOT>(control, target)); }
    void addCZ(int control, int target) { addGate(std::make_unique<CZ>(control, target)); }
    void addSWAP(int qubit1, int qubit2) { addGate(std::make_unique<SWAP>(qubit1, qubit2)); }
    
    void execute(QuantumState& state) {
        if (state.getNumQubits() != numQubits) {
            throw std::runtime_error("State and circuit qubit count mismatch");
        }
        
        for (const auto& gate : gates) {
            gate->apply(state);
        }
    }
    
    void print() const {
        std::cout << "Quantum Circuit (" << numQubits << " qubits):" << std::endl;
        for (const auto& gate : gates) {
            std::cout << "  " << gate->toString() << std::endl;
        }
    }
    
    int getNumQubits() const { return numQubits; }
    size_t getGateCount() const { return gates.size(); }
};

// Quantum algorithms
class QuantumAlgorithm {
public:
    virtual ~QuantumAlgorithm() = default;
    virtual std::string getName() const = 0;
    virtual QuantumCircuit createCircuit() const = 0;
    virtual void run(QuantumSimulator& simulator) = 0;
};

// Deutsch-Jozsa Algorithm
class DeutschJozsaAlgorithm : public QuantumAlgorithm {
private:
    std::vector<int> oracleFunction;
    int numInputQubits;
    
public:
    DeutschJozsaAlgorithm(const std::vector<int>& oracle) 
        : oracleFunction(oracle), numInputQubits(static_cast<int>(log2(oracle.size()))) {}
    
    std::string getName() const override { return "Deutsch-Jozsa"; }
    
    QuantumCircuit createCircuit() const override {
        QuantumCircuit circuit(numInputQubits + 1); // +1 for ancilla
        
        // Initialize ancilla in |1⟩
        circuit.addX(numInputQubits);
        
        // Apply Hadamard to all qubits
        for (int i = 0; i <= numInputQubits; i++) {
            circuit.addH(i);
        }
        
        // Apply oracle (simplified)
        applyOracle(circuit);
        
        // Apply Hadamard to input qubits
        for (int i = 0; i < numInputQubits; i++) {
            circuit.addH(i);
        }
        
        return circuit;
    }
    
    void run(QuantumSimulator& simulator) override;
    
private:
    void applyOracle(QuantumCircuit& circuit) const {
        // Simplified oracle implementation
        for (size_t i = 0; i < oracleFunction.size(); i++) {
            if (oracleFunction[i] == 1) {
                // Add controlled operations based on input state
                for (int j = 0; j < numInputQubits; j++) {
                    if ((i >> j) & 1) {
                        circuit.addCNOT(j, numInputQubits);
                    }
                }
            }
        }
    }
};

// Grover's Algorithm
class GroverAlgorithm : public QuantumAlgorithm {
private:
    int numQubits;
    int targetState;
    int numIterations;
    
public:
    GroverAlgorithm(int n, int target) : numQubits(n), targetState(target) {
        numIterations = static_cast<int>(M_PI * sqrt(1 << n) / 4);
    }
    
    std::string getName() const override { return "Grover's Search"; }
    
    QuantumCircuit createCircuit() const override {
        QuantumCircuit circuit(numQubits);
        
        // Initialize superposition
        for (int i = 0; i < numQubits; i++) {
            circuit.addH(i);
        }
        
        // Grover iterations
        for (int iter = 0; iter < numIterations; iter++) {
            // Oracle: flip phase of target state
            applyOracle(circuit);
            
            // Diffusion operator
            applyDiffusion(circuit);
        }
        
        return circuit;
    }
    
    void run(QuantumSimulator& simulator) override;
    
private:
    void applyOracle(QuantumCircuit& circuit) const {
        // Flip phase of target state
        for (int i = 0; i < numQubits; i++) {
            if (!((targetState >> i) & 1)) {
                circuit.addX(i);
            }
        }
        
        // Multi-controlled Z gate (simplified)
        if (numQubits > 1) {
            circuit.addCZ(0, 1);
        }
        
        for (int i = 0; i < numQubits; i++) {
            if (!((targetState >> i) & 1)) {
                circuit.addX(i);
            }
        }
    }
    
    void applyDiffusion(QuantumCircuit& circuit) const {
        // H gates
        for (int i = 0; i < numQubits; i++) {
            circuit.addH(i);
        }
        
        // X gates
        for (int i = 0; i < numQubits; i++) {
            circuit.addX(i);
        }
        
        // Multi-controlled Z gate
        if (numQubits > 1) {
            circuit.addCZ(0, 1);
        }
        
        // X gates
        for (int i = 0; i < numQubits; i++) {
            circuit.addX(i);
        }
        
        // H gates
        for (int i = 0; i < numQubits; i++) {
            circuit.addH(i);
        }
    }
};

// Quantum Fourier Transform
class QFTAlgorithm : public QuantumAlgorithm {
private:
    int numQubits;
    
public:
    QFTAlgorithm(int n) : numQubits(n) {}
    
    std::string getName() const override { return "Quantum Fourier Transform"; }
    
    QuantumCircuit createCircuit() const override {
        QuantumCircuit circuit(numQubits);
        
        for (int i = 0; i < numQubits; i++) {
            circuit.addH(i);
            
            for (int j = i + 1; j < numQubits; j++) {
                double phase = 2 * M_PI / (1 << (j - i + 1));
                circuit.addPhase(j, phase);
                circuit.addCNOT(j, i);
                circuit.addPhase(j, -phase);
                circuit.addCNOT(j, i);
            }
        }
        
        // Reverse qubit order
        for (int i = 0; i < numQubits / 2; i++) {
            circuit.addSWAP(i, numQubits - 1 - i);
        }
        
        return circuit;
    }
    
    void run(QuantumSimulator& simulator) override;
};

// Quantum simulator
class QuantumSimulator {
private:
    std::map<std::string, std::shared_ptr<QuantumAlgorithm>> algorithms;
    
public:
    QuantumSimulator() {
        // Register built-in algorithms
        registerAlgorithm("grover", std::make_shared<GroverAlgorithm>(3, 5));
        registerAlgorithm("qft", std::make_shared<QFTAlgorithm>(3));
        registerAlgorithm("deutsch-jozsa", std::make_shared<DeutschJozsaAlgorithm>(std::vector<int>{0,1,1,0}));
    }
    
    void registerAlgorithm(const std::string& name, std::shared_ptr<QuantumAlgorithm> algorithm) {
        algorithms[name] = algorithm;
    }
    
    void runAlgorithm(const std::string& name) {
        auto it = algorithms.find(name);
        if (it != algorithms.end()) {
            std::cout << "\n=== Running " << it->second->getName() << " Algorithm ===" << std::endl;
            it->second->run(*this);
        } else {
            std::cout << "Algorithm not found: " << name << std::endl;
        }
    }
    
    void executeCircuit(const QuantumCircuit& circuit, QuantumState& state) {
        auto circuitCopy = circuit;
        circuitCopy.execute(state);
    }
    
    void demonstrateGates() {
        std::cout << "\n=== QUANTUM GATE DEMONSTRATIONS ===" << std::endl;
        
        // Pauli X gate demo
        std::cout << "\n--- Pauli X Gate ---" << std::endl;
        QuantumState state1(1);
        std::cout << "Initial state:" << std::endl;
        state1.print();
        
        PauliX x(0);
        x.apply(state1);
        std::cout << "After X gate:" << std::endl;
        state1.print();
        
        // Hadamard gate demo
        std::cout << "\n--- Hadamard Gate ---" << std::endl;
        QuantumState state2(1);
        std::cout << "Initial state:" << std::endl;
        state2.print();
        
        Hadamard h(0);
        h.apply(state2);
        std::cout << "After H gate (superposition):" << std::endl;
        state2.print();
        state2.printProbabilities();
        
        // CNOT gate demo
        std::cout << "\n--- CNOT Gate ---" << std::endl;
        QuantumState state3(2);
        Hadamard h1(0);
        h1.apply(state3);
        std::cout << "After H on qubit 0:" << std::endl;
        state3.print();
        
        CNOT cnot(0, 1);
        cnot.apply(state3);
        std::cout << "After CNOT(0,1) - Bell State:" << std::endl;
        state3.print();
        state3.printProbabilities();
    }
    
    void demonstrateEntanglement() {
        std::cout << "\n=== QUANTUM ENTANGLEMENT DEMO ===" << std::endl;
        
        QuantumCircuit circuit(2);
        circuit.addH(0);
        circuit.addCNOT(0, 1);
        
        QuantumState state(2);
        
        std::cout << "Creating Bell state |Φ+⟩ = (|00⟩ + |11⟩)/√2" << std::endl;
        std::cout << "Circuit:" << std::endl;
        circuit.print();
        
        std::cout << "\nInitial state:" << std::endl;
        state.print();
        
        circuit.execute(state);
        std::cout << "\nAfter circuit execution:" << std::endl;
        state.print();
        state.printProbabilities();
        
        // Measure first qubit
        std::cout << "\nMeasuring first qubit..." << std::endl;
        int result = state.measureQubit(0);
        std::cout << "Result: " << result << std::endl;
        std::cout << "State after measurement:" << std::endl;
        state.print();
    }
    
    void demonstrateInterference() {
        std::cout << "\n=== QUANTUM INTERFERENCE DEMO ===" << std::endl;
        
        QuantumState state(1);
        std::cout << "Demonstrating quantum interference with Mach-Zehnder interferometer" << std::endl;
        
        // First beam splitter (Hadamard)
        Hadamard h1(0);
        h1.apply(state);
        std::cout << "After first beam splitter:" << std::endl;
        state.print();
        
        // Phase shift
        PhaseGate phase(0, M_PI);
        phase.apply(state);
        std::cout << "After phase shift (π):" << std::endl;
        state.print();
        
        // Second beam splitter
        Hadamard h2(0);
        h2.apply(state);
        std::cout << "After second beam splitter (destructive interference):" << std::endl;
        state.print();
        state.printProbabilities();
    }
    
    std::vector<std::string> getAvailableAlgorithms() const {
        std::vector<std::string> names;
        for (const auto& pair : algorithms) {
            names.push_back(pair.first);
        }
        return names;
    }
};

// Algorithm implementations
void DeutschJozsaAlgorithm::run(QuantumSimulator& simulator) {
    std::cout << "Determining if function is constant or balanced..." << std::endl;
    std::cout << "Oracle function: ";
    for (int val : oracleFunction) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    auto circuit = createCircuit();
    std::cout << "\nCircuit:" << std::endl;
    circuit.print();
    
    QuantumState state(numInputQubits + 1);
    simulator.executeCircuit(circuit, state);
    
    std::cout << "\nFinal state:" << std::endl;
    state.print();
    
    // Measure input qubits
    bool isConstant = true;
    for (int i = 0; i < numInputQubits; i++) {
        if (state.measureQubit(i) == 1) {
            isConstant = false;
            break;
        }
    }
    
    std::cout << "\nResult: Function is " << (isConstant ? "CONSTANT" : "BALANCED") << std::endl;
}

void GroverAlgorithm::run(QuantumSimulator& simulator) {
    std::cout << "Searching for target state: |" << targetState << "⟩" << std::endl;
    std::cout << "Search space size: " << (1 << numQubits) << std::endl;
    std::cout << "Number of iterations: " << numIterations << std::endl;
    
    auto circuit = createCircuit();
    std::cout << "\nCircuit has " << circuit.getGateCount() << " gates" << std::endl;
    
    QuantumState state(numQubits);
    simulator.executeCircuit(circuit, state);
    
    std::cout << "\nFinal state probabilities:" << std::endl;
    state.printProbabilities();
    
    // Check success probability
    double successProb = state.getProbability(targetState);
    std::cout << "\nProbability of finding target state: " 
              << std::fixed << std::setprecision(4) << successProb * 100 << "%" << std::endl;
    
    // Measure to get result
    int result = state.measure();
    std::cout << "Measurement result: |" << result << "⟩" << std::endl;
    std::cout << "Success: " << (result == targetState ? "YES" : "NO") << std::endl;
}

void QFTAlgorithm::run(QuantumSimulator& simulator) {
    std::cout << "Applying Quantum Fourier Transform to " << numQubits << " qubits" << std::endl;
    
    // Prepare initial state (example: |001⟩)
    QuantumState state(numQubits);
    if (numQubits >= 3) {
        PauliX x(0);
        x.apply(state);
    }
    
    std::cout << "\nInitial state:" << std::endl;
    state.print();
    
    auto circuit = createCircuit();
    std::cout << "\nApplying QFT..." << std::endl;
    simulator.executeCircuit(circuit, state);
    
    std::cout << "\nState after QFT:" << std::endl;
    state.print();
    state.printProbabilities();
}

// Performance benchmarking
class QuantumBenchmark {
private:
    QuantumSimulator& simulator;
    
public:
    QuantumBenchmark(QuantumSimulator& sim) : simulator(sim) {}
    
    void runBenchmarks() {
        std::cout << "\n=== QUANTUM SIMULATOR BENCHMARKS ===" << std::endl;
        
        benchmarkGateOperations();
        benchmarkCircuitExecution();
        benchmarkStateSize();
    }
    
private:
    void benchmarkGateOperations() {
        std::cout << "\n--- Gate Operation Benchmarks ---" << std::endl;
        
        std::vector<int> qubitCounts = {5, 10, 15};
        
        for (int n : qubitCounts) {
            if (n > 20) continue; // Avoid memory issues
            
            auto start = std::chrono::high_resolution_clock::now();
            
            QuantumState state(n);
            for (int i = 0; i < 100; i++) {
                Hadamard h(i % n);
                h.apply(state);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << n << " qubits, 100 H gates: " << duration.count() << " μs" << std::endl;
        }
    }
    
    void benchmarkCircuitExecution() {
        std::cout << "\n--- Circuit Execution Benchmarks ---" << std::endl;
        
        for (int n = 3; n <= 12; n += 3) {
            if (n > 15) continue;
            
            auto start = std::chrono::high_resolution_clock::now();
            
            QuantumCircuit circuit(n);
            for (int i = 0; i < n; i++) {
                circuit.addH(i);
            }
            for (int i = 0; i < n - 1; i++) {
                circuit.addCNOT(i, i + 1);
            }
            
            QuantumState state(n);
            circuit.execute(state);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << n << " qubits, " << (n + n - 1) << " gates: " 
                      << duration.count() << " μs" << std::endl;
        }
    }
    
    void benchmarkStateSize() {
        std::cout << "\n--- State Size Analysis ---" << std::endl;
        
        for (int n = 1; n <= 20; n++) {
            size_t stateSize = sizeof(Complex) * (1ULL << n);
            std::cout << n << " qubits: " << (stateSize / 1024.0) << " KB" << std::endl;
            
            if (stateSize > 1024 * 1024 * 100) { // 100 MB limit
                std::cout << "  (Memory limit reached)" << std::endl;
                break;
            }
        }
    }
};

// Demo function
void runQuantumSimulatorDemo() {
    std::cout << "=== QUANTUM COMPUTING SIMULATOR DEMO ===" << std::endl;
    std::cout << "Simulating quantum circuits with superposition, entanglement, and interference" << std::endl;
    
    QuantumSimulator simulator;
    
    // Demonstrate basic quantum gates
    simulator.demonstrateGates();
    
    // Demonstrate quantum entanglement
    simulator.demonstrateEntanglement();
    
    // Demonstrate quantum interference
    simulator.demonstrateInterference();
    
    // Run quantum algorithms
    std::cout << "\n=== QUANTUM ALGORITHMS ===" << std::endl;
    
    auto algorithms = simulator.getAvailableAlgorithms();
    for (const auto& alg : algorithms) {
        simulator.runAlgorithm(alg);
    }
    
    // Run benchmarks
    QuantumBenchmark benchmark(simulator);
    benchmark.runBenchmarks();
    
    // Interactive circuit builder demo
    std::cout << "\n=== CUSTOM CIRCUIT DEMO ===" << std::endl;
    QuantumCircuit customCircuit(3);
    customCircuit.addH(0);
    customCircuit.addH(1);
    customCircuit.addH(2);
    customCircuit.addCNOT(0, 1);
    customCircuit.addCNOT(1, 2);
    customCircuit.addRX(0, M_PI/4);
    customCircuit.addPhase(2, M_PI/2);
    
    std::cout << "Custom circuit:" << std::endl;
    customCircuit.print();
    
    QuantumState customState(3);
    customCircuit.execute(customState);
    
    std::cout << "\nResult:" << std::endl;
    customState.print();
    customState.printProbabilities();
    
    std::cout << "\n=== SIMULATION COMPLETE ===" << std::endl;
    std::cout << "This simulator demonstrates:" << std::endl;
    std::cout << "- Quantum state representation and manipulation" << std::endl;
    std::cout << "- Single and multi-qubit quantum gates" << std::endl;
    std::cout << "- Quantum circuits and algorithms" << std::endl;
    std::cout << "- Quantum phenomena (superposition, entanglement, interference)" << std::endl;
    std::cout << "- Measurement and state collapse" << std::endl;
}

int main() {
    try {
        runQuantumSimulatorDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
