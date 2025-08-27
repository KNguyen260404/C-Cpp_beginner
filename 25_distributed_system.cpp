#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <memory>
#include <atomic>
#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <future>
#include <unordered_map>
#include <set>

// Forward declarations
class Node;
class Message;
class Task;
class LoadBalancer;

// Message types for distributed communication
enum class MessageType {
    TASK_REQUEST,
    TASK_RESPONSE,
    HEARTBEAT,
    LOAD_INFO,
    TASK_DISTRIBUTION,
    NODE_JOIN,
    NODE_LEAVE,
    CONSENSUS_REQUEST,
    CONSENSUS_RESPONSE,
    DATA_REPLICATION
};

// Distributed message structure
class Message {
public:
    MessageType type;
    std::string senderId;
    std::string receiverId;
    std::string payload;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    int priority;
    
    Message(MessageType t, const std::string& sender, const std::string& receiver, 
            const std::string& data, int prio = 0)
        : type(t), senderId(sender), receiverId(receiver), payload(data), 
          timestamp(std::chrono::steady_clock::now()), priority(prio) {}
};

// Task representation
class Task {
public:
    std::string id;
    std::string type;
    std::string data;
    int complexity;
    std::chrono::time_point<std::chrono::steady_clock> createdTime;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;
    std::string assignedNode;
    bool completed;
    std::string result;
    
    Task(const std::string& taskId, const std::string& taskType, 
         const std::string& taskData, int comp)
        : id(taskId), type(taskType), data(taskData), complexity(comp),
          createdTime(std::chrono::steady_clock::now()), completed(false) {}
    
    double getExecutionTime() const {
        if (completed) {
            return std::chrono::duration<double>(endTime - startTime).count();
        }
        return 0.0;
    }
};

// Network simulator for distributed communication
class NetworkSimulator {
private:
    std::map<std::string, std::queue<Message>> messageQueues;
    std::mutex networkMutex;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<> latencyDist;
    std::uniform_real_distribution<> dropRate;
    
public:
    NetworkSimulator() : gen(rd()), latencyDist(10, 100), dropRate(0.0, 0.05) {}
    
    void sendMessage(const Message& msg) {
        std::lock_guard<std::mutex> lock(networkMutex);
        
        // Simulate network latency and packet loss
        if (dropRate(gen) > 0.02) {  // 2% packet loss
            // Simulate network delay
            std::this_thread::sleep_for(std::chrono::milliseconds(
                static_cast<int>(latencyDist(gen))));
            
            messageQueues[msg.receiverId].push(msg);
        }
    }
    
    bool receiveMessage(const std::string& nodeId, Message& msg) {
        std::lock_guard<std::mutex> lock(networkMutex);
        if (!messageQueues[nodeId].empty()) {
            msg = messageQueues[nodeId].front();
            messageQueues[nodeId].pop();
            return true;
        }
        return false;
    }
    
    void registerNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(networkMutex);
        messageQueues[nodeId] = std::queue<Message>();
    }
    
    void unregisterNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(networkMutex);
        messageQueues.erase(nodeId);
    }
};

// Distributed Hash Table for data storage
class DistributedHashTable {
private:
    std::unordered_map<std::string, std::string> localData;
    std::mutex dataMutex;
    std::vector<std::string> nodeRing;  // Consistent hashing ring
    
public:
    void put(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(dataMutex);
        localData[key] = value;
    }
    
    bool get(const std::string& key, std::string& value) {
        std::lock_guard<std::mutex> lock(dataMutex);
        auto it = localData.find(key);
        if (it != localData.end()) {
            value = it->second;
            return true;
        }
        return false;
    }
    
    void remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(dataMutex);
        localData.erase(key);
    }
    
    std::vector<std::string> getAllKeys() {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<std::string> keys;
        for (const auto& pair : localData) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(dataMutex);
        return localData.size();
    }
};

// Load balancing algorithms
class LoadBalancer {
private:
    std::map<std::string, double> nodeLoads;
    std::map<std::string, int> nodeCapacities;
    std::mutex loadMutex;
    
public:
    void updateNodeLoad(const std::string& nodeId, double load, int capacity) {
        std::lock_guard<std::mutex> lock(loadMutex);
        nodeLoads[nodeId] = load;
        nodeCapacities[nodeId] = capacity;
    }
    
    std::string selectNodeRoundRobin(const std::vector<std::string>& availableNodes) {
        static size_t currentIndex = 0;
        if (availableNodes.empty()) return "";
        
        std::string selected = availableNodes[currentIndex % availableNodes.size()];
        currentIndex++;
        return selected;
    }
    
    std::string selectNodeLeastLoaded(const std::vector<std::string>& availableNodes) {
        std::lock_guard<std::mutex> lock(loadMutex);
        
        std::string bestNode;
        double minLoad = std::numeric_limits<double>::max();
        
        for (const std::string& nodeId : availableNodes) {
            auto it = nodeLoads.find(nodeId);
            if (it != nodeLoads.end() && it->second < minLoad) {
                minLoad = it->second;
                bestNode = nodeId;
            }
        }
        
        return bestNode.empty() ? (availableNodes.empty() ? "" : availableNodes[0]) : bestNode;
    }
    
    std::string selectNodeWeightedRandom(const std::vector<std::string>& availableNodes) {
        std::lock_guard<std::mutex> lock(loadMutex);
        
        if (availableNodes.empty()) return "";
        
        std::vector<double> weights;
        for (const std::string& nodeId : availableNodes) {
            auto loadIt = nodeLoads.find(nodeId);
            auto capIt = nodeCapacities.find(nodeId);
            
            double load = (loadIt != nodeLoads.end()) ? loadIt->second : 0.5;
            int capacity = (capIt != nodeCapacities.end()) ? capIt->second : 100;
            
            // Higher capacity and lower load = higher weight
            double weight = capacity / (1.0 + load);
            weights.push_back(weight);
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> dist(weights.begin(), weights.end());
        
        return availableNodes[dist(gen)];
    }
    
    void removeNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(loadMutex);
        nodeLoads.erase(nodeId);
        nodeCapacities.erase(nodeId);
    }
    
    std::map<std::string, double> getNodeLoads() {
        std::lock_guard<std::mutex> lock(loadMutex);
        return nodeLoads;
    }
};

// Consensus algorithm (Simplified Raft)
class ConsensusManager {
private:
    std::string nodeId;
    std::vector<std::string> clusterNodes;
    std::string currentLeader;
    int currentTerm;
    std::string votedFor;
    std::map<std::string, int> voteResponses;
    std::mutex consensusMutex;
    bool isLeader;
    
public:
    ConsensusManager(const std::string& id) 
        : nodeId(id), currentTerm(0), isLeader(false) {}
    
    void addNode(const std::string& id) {
        std::lock_guard<std::mutex> lock(consensusMutex);
        clusterNodes.push_back(id);
    }
    
    void removeNode(const std::string& id) {
        std::lock_guard<std::mutex> lock(consensusMutex);
        clusterNodes.erase(
            std::remove(clusterNodes.begin(), clusterNodes.end(), id),
            clusterNodes.end());
    }
    
    bool startElection() {
        std::lock_guard<std::mutex> lock(consensusMutex);
        currentTerm++;
        votedFor = nodeId;
        voteResponses.clear();
        voteResponses[nodeId] = currentTerm;
        
        std::cout << "Node " << nodeId << " starting election for term " << currentTerm << std::endl;
        
        // In a real implementation, we would send vote requests to other nodes
        // For simulation, we'll assume some nodes vote for us
        int votesNeeded = (clusterNodes.size() + 1) / 2 + 1;
        int votesReceived = 1; // Self vote
        
        // Simulate receiving votes from other nodes
        for (size_t i = 0; i < clusterNodes.size() && votesReceived < votesNeeded; i++) {
            if (rand() % 100 < 70) { // 70% chance of receiving vote
                voteResponses[clusterNodes[i]] = currentTerm;
                votesReceived++;
            }
        }
        
        if (votesReceived >= votesNeeded) {
            isLeader = true;
            currentLeader = nodeId;
            std::cout << "Node " << nodeId << " became leader for term " << currentTerm << std::endl;
            return true;
        }
        
        return false;
    }
    
    bool isCurrentLeader() const {
        return isLeader;
    }
    
    std::string getLeader() const {
        return currentLeader;
    }
    
    int getTerm() const {
        return currentTerm;
    }
};

// Individual node in the distributed system
class Node {
private:
    std::string nodeId;
    std::string nodeType;
    int capacity;
    std::atomic<double> currentLoad;
    std::atomic<bool> isActive;
    std::queue<Task> taskQueue;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
    std::thread workerThread;
    std::thread heartbeatThread;
    std::shared_ptr<NetworkSimulator> network;
    std::shared_ptr<LoadBalancer> loadBalancer;
    DistributedHashTable dht;
    ConsensusManager consensus;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>> lastHeartbeat;
    std::mutex heartbeatMutex;
    
    // Performance metrics
    std::atomic<int> tasksCompleted;
    std::atomic<double> totalExecutionTime;
    std::vector<double> responseTimeHistory;
    std::mutex metricsMutex;
    
public:
    Node(const std::string& id, const std::string& type, int cap,
         std::shared_ptr<NetworkSimulator> net, std::shared_ptr<LoadBalancer> lb)
        : nodeId(id), nodeType(type), capacity(cap), currentLoad(0.0), 
          isActive(true), network(net), loadBalancer(lb), consensus(id),
          tasksCompleted(0), totalExecutionTime(0.0) {
        
        network->registerNode(nodeId);
        
        // Start worker and heartbeat threads
        workerThread = std::thread(&Node::workerLoop, this);
        heartbeatThread = std::thread(&Node::heartbeatLoop, this);
    }
    
    ~Node() {
        shutdown();
    }
    
    void shutdown() {
        isActive = false;
        taskCondition.notify_all();
        
        if (workerThread.joinable()) {
            workerThread.join();
        }
        if (heartbeatThread.joinable()) {
            heartbeatThread.join();
        }
        
        network->unregisterNode(nodeId);
        loadBalancer->removeNode(nodeId);
    }
    
    void addTask(const Task& task) {
        std::lock_guard<std::mutex> lock(taskMutex);
        taskQueue.push(task);
        taskCondition.notify_one();
    }
    
    void processMessage(const Message& msg) {
        switch (msg.type) {
            case MessageType::TASK_REQUEST: {
                Task task(msg.payload, "computation", msg.payload, 
                         rand() % 100 + 1);
                addTask(task);
                break;
            }
            case MessageType::HEARTBEAT: {
                std::lock_guard<std::mutex> lock(heartbeatMutex);
                lastHeartbeat[msg.senderId] = std::chrono::steady_clock::now();
                break;
            }
            case MessageType::LOAD_INFO: {
                // Update load balancer with received load info
                std::istringstream iss(msg.payload);
                double load;
                int cap;
                if (iss >> load >> cap) {
                    loadBalancer->updateNodeLoad(msg.senderId, load, cap);
                }
                break;
            }
            case MessageType::CONSENSUS_REQUEST: {
                // Handle consensus/election requests
                handleConsensusRequest(msg);
                break;
            }
            default:
                break;
        }
    }
    
    void handleConsensusRequest(const Message& msg) {
        // Simplified consensus handling
        std::cout << "Node " << nodeId << " received consensus request from " 
                  << msg.senderId << std::endl;
        
        // Send response back
        Message response(MessageType::CONSENSUS_RESPONSE, nodeId, msg.senderId,
                        "vote_granted:" + std::to_string(consensus.getTerm()));
        network->sendMessage(response);
    }
    
    std::string getId() const { return nodeId; }
    std::string getType() const { return nodeType; }
    int getCapacity() const { return capacity; }
    double getCurrentLoad() const { return currentLoad.load(); }
    bool getIsActive() const { return isActive.load(); }
    
    // Performance metrics
    int getTasksCompleted() const { return tasksCompleted.load(); }
    double getAverageResponseTime() const {
        std::lock_guard<std::mutex> lock(metricsMutex);
        if (responseTimeHistory.empty()) return 0.0;
        
        double sum = 0.0;
        for (double time : responseTimeHistory) {
            sum += time;
        }
        return sum / responseTimeHistory.size();
    }
    
    double getThroughput() const {
        return tasksCompleted.load() / (totalExecutionTime.load() + 1.0);
    }
    
    void printStatus() const {
        std::cout << "Node " << nodeId << " [" << nodeType << "]:\n"
                  << "  Capacity: " << capacity << "\n"
                  << "  Current Load: " << std::fixed << std::setprecision(2) 
                  << currentLoad.load() << "\n"
                  << "  Tasks Completed: " << tasksCompleted.load() << "\n"
                  << "  Average Response Time: " << getAverageResponseTime() << "s\n"
                  << "  Throughput: " << getThroughput() << " tasks/s\n"
                  << "  DHT Size: " << dht.size() << " entries\n"
                  << "  Status: " << (isActive ? "Active" : "Inactive") << "\n\n";
    }
    
private:
    void workerLoop() {
        while (isActive) {
            std::unique_lock<std::mutex> lock(taskMutex);
            taskCondition.wait(lock, [this] { 
                return !taskQueue.empty() || !isActive; 
            });
            
            if (!isActive) break;
            
            if (!taskQueue.empty()) {
                Task task = taskQueue.front();
                taskQueue.pop();
                lock.unlock();
                
                executeTask(task);
            }
        }
    }
    
    void executeTask(Task& task) {
        task.startTime = std::chrono::steady_clock::now();
        task.assignedNode = nodeId;
        
        // Update load
        currentLoad = currentLoad.load() + (task.complexity / 100.0);
        
        // Simulate task execution time based on complexity
        int executionTime = task.complexity * 10 + (rand() % 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(executionTime));
        
        // Generate result
        task.result = "Result_" + std::to_string(rand() % 10000);
        task.endTime = std::chrono::steady_clock::now();
        task.completed = true;
        
        // Update metrics
        tasksCompleted++;
        double responseTime = task.getExecutionTime();
        totalExecutionTime += responseTime;
        
        {
            std::lock_guard<std::mutex> lock(metricsMutex);
            responseTimeHistory.push_back(responseTime);
            if (responseTimeHistory.size() > 100) {
                responseTimeHistory.erase(responseTimeHistory.begin());
            }
        }
        
        // Store result in DHT
        dht.put(task.id, task.result);
        
        // Update load
        currentLoad = std::max(0.0, currentLoad.load() - (task.complexity / 100.0));
        
        std::cout << "Node " << nodeId << " completed task " << task.id 
                  << " in " << responseTime << "s" << std::endl;
    }
    
    void heartbeatLoop() {
        while (isActive) {
            // Send heartbeat to all known nodes
            Message heartbeat(MessageType::HEARTBEAT, nodeId, "broadcast", 
                            "alive:" + std::to_string(currentLoad.load()));
            
            // In a real system, we'd broadcast to all nodes
            // For simulation, we'll just update the load balancer
            loadBalancer->updateNodeLoad(nodeId, currentLoad.load(), capacity);
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
};

// Main distributed system coordinator
class DistributedSystem {
private:
    std::vector<std::shared_ptr<Node>> nodes;
    std::shared_ptr<NetworkSimulator> network;
    std::shared_ptr<LoadBalancer> loadBalancer;
    std::queue<Task> pendingTasks;
    std::mutex systemMutex;
    std::atomic<int> taskCounter;
    std::thread distributorThread;
    std::atomic<bool> systemRunning;
    
    // System metrics
    std::chrono::time_point<std::chrono::steady_clock> systemStartTime;
    std::atomic<int> totalTasksSubmitted;
    std::atomic<int> totalTasksCompleted;
    
public:
    DistributedSystem() 
        : network(std::make_shared<NetworkSimulator>()),
          loadBalancer(std::make_shared<LoadBalancer>()),
          taskCounter(0), systemRunning(true),
          systemStartTime(std::chrono::steady_clock::now()),
          totalTasksSubmitted(0), totalTasksCompleted(0) {
        
        distributorThread = std::thread(&DistributedSystem::taskDistributorLoop, this);
    }
    
    ~DistributedSystem() {
        shutdown();
    }
    
    void addNode(const std::string& nodeType, int capacity) {
        std::string nodeId = "Node_" + std::to_string(nodes.size() + 1);
        auto node = std::make_shared<Node>(nodeId, nodeType, capacity, 
                                          network, loadBalancer);
        
        std::lock_guard<std::mutex> lock(systemMutex);
        nodes.push_back(node);
        
        std::cout << "Added " << nodeType << " node: " << nodeId 
                  << " with capacity " << capacity << std::endl;
    }
    
    void removeNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(systemMutex);
        nodes.erase(
            std::remove_if(nodes.begin(), nodes.end(),
                [&nodeId](const std::shared_ptr<Node>& node) {
                    return node->getId() == nodeId;
                }),
            nodes.end());
        
        std::cout << "Removed node: " << nodeId << std::endl;
    }
    
    void submitTask(const std::string& taskType, const std::string& data, int complexity) {
        std::string taskId = "Task_" + std::to_string(taskCounter++);
        Task task(taskId, taskType, data, complexity);
        
        std::lock_guard<std::mutex> lock(systemMutex);
        pendingTasks.push(task);
        totalTasksSubmitted++;
        
        std::cout << "Submitted task: " << taskId << " [" << taskType 
                  << "] complexity: " << complexity << std::endl;
    }
    
    void submitRandomTasks(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> complexityDist(1, 100);
        std::vector<std::string> taskTypes = {"compute", "data_processing", 
                                             "machine_learning", "simulation"};
        
        for (int i = 0; i < count; i++) {
            std::string taskType = taskTypes[gen() % taskTypes.size()];
            std::string data = "data_" + std::to_string(i);
            int complexity = complexityDist(gen);
            
            submitTask(taskType, data, complexity);
            
            // Add some delay between submissions
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void printSystemStatus() {
        std::lock_guard<std::mutex> lock(systemMutex);
        
        auto currentTime = std::chrono::steady_clock::now();
        double uptime = std::chrono::duration<double>(currentTime - systemStartTime).count();
        
        std::cout << "\n=== DISTRIBUTED SYSTEM STATUS ===" << std::endl;
        std::cout << "System Uptime: " << std::fixed << std::setprecision(1) 
                  << uptime << " seconds" << std::endl;
        std::cout << "Total Nodes: " << nodes.size() << std::endl;
        std::cout << "Tasks Submitted: " << totalTasksSubmitted.load() << std::endl;
        std::cout << "Tasks Completed: " << getTotalTasksCompleted() << std::endl;
        std::cout << "Pending Tasks: " << pendingTasks.size() << std::endl;
        std::cout << "System Throughput: " << getTotalTasksCompleted() / (uptime + 1.0) 
                  << " tasks/s" << std::endl;
        
        std::cout << "\n--- NODE STATUS ---" << std::endl;
        for (const auto& node : nodes) {
            node->printStatus();
        }
        
        std::cout << "--- LOAD BALANCER STATUS ---" << std::endl;
        auto nodeLoads = loadBalancer->getNodeLoads();
        for (const auto& load : nodeLoads) {
            std::cout << "  " << load.first << ": " << std::fixed 
                      << std::setprecision(2) << load.second << std::endl;
        }
        std::cout << std::endl;
    }
    
    void shutdown() {
        systemRunning = false;
        
        if (distributorThread.joinable()) {
            distributorThread.join();
        }
        
        // Shutdown all nodes
        for (auto& node : nodes) {
            node->shutdown();
        }
        
        nodes.clear();
    }
    
    int getTotalTasksCompleted() const {
        int total = 0;
        for (const auto& node : nodes) {
            total += node->getTasksCompleted();
        }
        return total;
    }
    
    double getAverageSystemLoad() const {
        if (nodes.empty()) return 0.0;
        
        double totalLoad = 0.0;
        for (const auto& node : nodes) {
            totalLoad += node->getCurrentLoad();
        }
        return totalLoad / nodes.size();
    }
    
private:
    void taskDistributorLoop() {
        while (systemRunning) {
            {
                std::lock_guard<std::mutex> lock(systemMutex);
                if (!pendingTasks.empty() && !nodes.empty()) {
                    Task task = pendingTasks.front();
                    pendingTasks.pop();
                    
                    // Get list of active nodes
                    std::vector<std::string> activeNodes;
                    for (const auto& node : nodes) {
                        if (node->getIsActive()) {
                            activeNodes.push_back(node->getId());
                        }
                    }
                    
                    if (!activeNodes.empty()) {
                        // Use different load balancing strategies
                        std::string selectedNodeId;
                        static int strategy = 0;
                        
                        switch (strategy % 3) {
                            case 0:
                                selectedNodeId = loadBalancer->selectNodeLeastLoaded(activeNodes);
                                break;
                            case 1:
                                selectedNodeId = loadBalancer->selectNodeRoundRobin(activeNodes);
                                break;
                            case 2:
                                selectedNodeId = loadBalancer->selectNodeWeightedRandom(activeNodes);
                                break;
                        }
                        strategy++;
                        
                        // Find the selected node and assign the task
                        for (auto& node : nodes) {
                            if (node->getId() == selectedNodeId) {
                                node->addTask(task);
                                break;
                            }
                        }
                        
                        std::cout << "Distributed task " << task.id 
                                  << " to node " << selectedNodeId << std::endl;
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
};

// Demo function
void runDistributedSystemDemo() {
    std::cout << "=== DISTRIBUTED SYSTEM DEMO ===" << std::endl;
    std::cout << "Initializing distributed computing system..." << std::endl;
    
    DistributedSystem system;
    
    // Add different types of nodes with varying capacities
    system.addNode("compute", 100);
    system.addNode("storage", 150);
    system.addNode("compute", 80);
    system.addNode("gpu", 200);
    system.addNode("memory", 120);
    
    std::cout << "\nSystem initialized. Starting task processing..." << std::endl;
    
    // Submit various tasks
    system.submitTask("matrix_multiplication", "1000x1000", 80);
    system.submitTask("data_analysis", "dataset_large.csv", 60);
    system.submitTask("machine_learning", "neural_network_training", 95);
    system.submitTask("simulation", "physics_simulation", 70);
    
    // Wait for initial tasks to process
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "\nSubmitting batch of random tasks..." << std::endl;
    system.submitRandomTasks(20);
    
    // Monitor system for a while
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system.printSystemStatus();
        
        // Simulate node failure and recovery
        if (i == 5) {
            std::cout << "Simulating node failure..." << std::endl;
            // In a real system, we would handle node failures
        }
        
        // Add more tasks during execution
        if (i % 3 == 0) {
            system.submitRandomTasks(5);
        }
    }
    
    std::cout << "\nFinal system status:" << std::endl;
    system.printSystemStatus();
    
    std::cout << "\nShutting down distributed system..." << std::endl;
}

int main() {
    try {
        runDistributedSystemDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
