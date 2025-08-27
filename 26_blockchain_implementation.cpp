#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <queue>
#include <atomic>
#include <functional>
#include <cmath>

// Cryptographic hash function (simplified SHA-256-like)
class CryptoHash {
private:
    static const uint32_t K[64];
    static const uint32_t H0[8];
    
    static uint32_t rightRotate(uint32_t value, uint32_t amount) {
        return (value >> amount) | (value << (32 - amount));
    }
    
    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g) {
        return (e & f) ^ (~e & g);
    }
    
    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c) {
        return (a & b) ^ (a & c) ^ (b & c);
    }
    
public:
    static std::string hash(const std::string& input) {
        // Simplified hash function for demonstration
        // In production, use a proper cryptographic library
        std::hash<std::string> hasher;
        size_t hashValue = hasher(input);
        
        // Convert to hex string with leading zeros
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << hashValue;
        return ss.str();
    }
    
    static std::string doubleHash(const std::string& input) {
        return hash(hash(input));
    }
    
    static bool verifyHash(const std::string& data, const std::string& expectedHash) {
        return hash(data) == expectedHash;
    }
    
    static std::string merkleRoot(const std::vector<std::string>& hashes) {
        if (hashes.empty()) return hash("");
        if (hashes.size() == 1) return hashes[0];
        
        std::vector<std::string> newLevel;
        for (size_t i = 0; i < hashes.size(); i += 2) {
            std::string left = hashes[i];
            std::string right = (i + 1 < hashes.size()) ? hashes[i + 1] : left;
            newLevel.push_back(hash(left + right));
        }
        
        return merkleRoot(newLevel);
    }
};

// Digital signature (simplified)
class DigitalSignature {
private:
    std::string privateKey;
    std::string publicKey;
    
public:
    DigitalSignature() {
        generateKeyPair();
    }
    
    void generateKeyPair() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000000, 9999999);
        
        privateKey = std::to_string(dis(gen));
        publicKey = CryptoHash::hash(privateKey);
    }
    
    std::string sign(const std::string& message) const {
        return CryptoHash::hash(message + privateKey);
    }
    
    bool verify(const std::string& message, const std::string& signature) const {
        return signature == CryptoHash::hash(message + privateKey);
    }
    
    std::string getPublicKey() const { return publicKey; }
    std::string getPrivateKey() const { return privateKey; }
    
    static bool verifySignature(const std::string& message, 
                               const std::string& signature,
                               const std::string& publicKey) {
        // Simplified verification - in reality this would be more complex
        return !signature.empty() && !publicKey.empty();
    }
};

// Transaction class
class Transaction {
public:
    std::string txId;
    std::string sender;
    std::string receiver;
    double amount;
    double fee;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    std::string signature;
    std::map<std::string, std::string> metadata;
    
    Transaction(const std::string& from, const std::string& to, 
                double amt, double txFee = 0.001)
        : sender(from), receiver(to), amount(amt), fee(txFee),
          timestamp(std::chrono::system_clock::now()) {
        
        generateTxId();
    }
    
    void generateTxId() {
        std::stringstream ss;
        ss << sender << receiver << amount << fee 
           << std::chrono::duration_cast<std::chrono::milliseconds>(
               timestamp.time_since_epoch()).count();
        txId = CryptoHash::hash(ss.str());
    }
    
    void sign(const DigitalSignature& signer) {
        std::string message = getTxData();
        signature = signer.sign(message);
    }
    
    bool verify() const {
        if (signature.empty()) return false;
        std::string message = getTxData();
        return DigitalSignature::verifySignature(message, signature, sender);
    }
    
    std::string getTxData() const {
        std::stringstream ss;
        ss << txId << sender << receiver << amount << fee;
        return ss.str();
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "TX[" << txId.substr(0, 8) << "...] "
           << sender.substr(0, 8) << "... -> " 
           << receiver.substr(0, 8) << "... "
           << amount << " coins (fee: " << fee << ")";
        return ss.str();
    }
    
    double getTotalAmount() const {
        return amount + fee;
    }
};

// UTXO (Unspent Transaction Output)
class UTXO {
public:
    std::string txId;
    int outputIndex;
    std::string owner;
    double amount;
    bool spent;
    
    UTXO(const std::string& id, int index, const std::string& addr, double amt)
        : txId(id), outputIndex(index), owner(addr), amount(amt), spent(false) {}
    
    std::string getKey() const {
        return txId + ":" + std::to_string(outputIndex);
    }
};

// Block class
class Block {
public:
    int index;
    std::string previousHash;
    std::string merkleRoot;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    std::vector<Transaction> transactions;
    int nonce;
    std::string hash;
    int difficulty;
    std::string minerAddress;
    double blockReward;
    
    Block(int idx, const std::string& prevHash, int diff = 4)
        : index(idx), previousHash(prevHash), difficulty(diff), nonce(0),
          timestamp(std::chrono::system_clock::now()), blockReward(50.0) {}
    
    void addTransaction(const Transaction& tx) {
        transactions.push_back(tx);
        updateMerkleRoot();
    }
    
    void updateMerkleRoot() {
        std::vector<std::string> txHashes;
        for (const auto& tx : transactions) {
            txHashes.push_back(tx.txId);
        }
        merkleRoot = CryptoHash::merkleRoot(txHashes);
    }
    
    std::string calculateHash() const {
        std::stringstream ss;
        ss << index << previousHash << merkleRoot << nonce << difficulty
           << std::chrono::duration_cast<std::chrono::milliseconds>(
               timestamp.time_since_epoch()).count();
        return CryptoHash::hash(ss.str());
    }
    
    bool mine(const std::string& miner) {
        minerAddress = miner;
        std::string target(difficulty, '0');
        
        std::cout << "Mining block " << index << " (difficulty: " << difficulty << ")..." << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        do {
            nonce++;
            hash = calculateHash();
            
            // Show mining progress
            if (nonce % 100000 == 0) {
                std::cout << "Nonce: " << nonce << ", Hash: " << hash.substr(0, 16) << "..." << std::endl;
            }
            
        } while (hash.substr(0, difficulty) != target);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Block mined! Hash: " << hash << std::endl;
        std::cout << "Mining time: " << duration.count() << " ms" << std::endl;
        std::cout << "Nonce: " << nonce << std::endl;
        
        return true;
    }
    
    bool isValid() const {
        if (hash != calculateHash()) return false;
        
        std::string target(difficulty, '0');
        if (hash.substr(0, difficulty) != target) return false;
        
        // Verify all transactions
        for (const auto& tx : transactions) {
            if (!tx.verify()) return false;
        }
        
        return true;
    }
    
    double getTotalFees() const {
        double totalFees = 0.0;
        for (const auto& tx : transactions) {
            totalFees += tx.fee;
        }
        return totalFees;
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "Block #" << index << "\n";
        ss << "Hash: " << hash << "\n";
        ss << "Previous: " << previousHash << "\n";
        ss << "Merkle Root: " << merkleRoot << "\n";
        ss << "Transactions: " << transactions.size() << "\n";
        ss << "Miner: " << minerAddress.substr(0, 16) << "...\n";
        ss << "Reward: " << blockReward + getTotalFees() << " coins\n";
        ss << "Nonce: " << nonce << "\n";
        return ss.str();
    }
};

// Wallet class
class Wallet {
private:
    DigitalSignature keyPair;
    std::string address;
    
public:
    Wallet() {
        address = keyPair.getPublicKey();
    }
    
    std::string getAddress() const { return address; }
    
    Transaction createTransaction(const std::string& to, double amount, double fee = 0.001) {
        Transaction tx(address, to, amount, fee);
        tx.sign(keyPair);
        return tx;
    }
    
    bool verifyOwnership(const std::string& message, const std::string& signature) const {
        return keyPair.verify(message, signature);
    }
};

// Memory pool for pending transactions
class MemoryPool {
private:
    std::vector<Transaction> pendingTransactions;
    std::mutex poolMutex;
    size_t maxSize;
    
public:
    MemoryPool(size_t maxPoolSize = 10000) : maxSize(maxPoolSize) {}
    
    void addTransaction(const Transaction& tx) {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        // Verify transaction before adding
        if (!tx.verify()) {
            std::cout << "Invalid transaction rejected: " << tx.txId << std::endl;
            return;
        }
        
        // Check for duplicate
        for (const auto& existingTx : pendingTransactions) {
            if (existingTx.txId == tx.txId) {
                return; // Already exists
            }
        }
        
        pendingTransactions.push_back(tx);
        
        // Remove oldest transactions if pool is full
        if (pendingTransactions.size() > maxSize) {
            pendingTransactions.erase(pendingTransactions.begin());
        }
        
        std::cout << "Transaction added to mempool: " << tx.toString() << std::endl;
    }
    
    std::vector<Transaction> getTransactions(size_t maxCount = 100) {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        // Sort by fee (highest first) for priority processing
        std::sort(pendingTransactions.begin(), pendingTransactions.end(),
                  [](const Transaction& a, const Transaction& b) {
                      return a.fee > b.fee;
                  });
        
        size_t count = std::min(maxCount, pendingTransactions.size());
        return std::vector<Transaction>(pendingTransactions.begin(), 
                                       pendingTransactions.begin() + count);
    }
    
    void removeTransactions(const std::vector<Transaction>& txs) {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        for (const auto& tx : txs) {
            pendingTransactions.erase(
                std::remove_if(pendingTransactions.begin(), pendingTransactions.end(),
                              [&tx](const Transaction& t) { return t.txId == tx.txId; }),
                pendingTransactions.end());
        }
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(poolMutex);
        return pendingTransactions.size();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(poolMutex);
        pendingTransactions.clear();
    }
};

// Blockchain class
class Blockchain {
private:
    std::vector<Block> chain;
    std::unordered_map<std::string, UTXO> utxoSet;
    std::unordered_map<std::string, double> balances;
    MemoryPool mempool;
    int difficulty;
    double blockReward;
    std::mutex chainMutex;
    
    // Mining and consensus
    std::atomic<bool> miningActive;
    std::thread miningThread;
    std::string minerAddress;
    
    // Network simulation
    std::vector<std::string> networkNodes;
    std::mutex networkMutex;
    
public:
    Blockchain(int initialDifficulty = 4, double reward = 50.0)
        : difficulty(initialDifficulty), blockReward(reward), miningActive(false) {
        createGenesisBlock();
    }
    
    ~Blockchain() {
        stopMining();
    }
    
    void createGenesisBlock() {
        Block genesis(0, "0", difficulty);
        genesis.timestamp = std::chrono::system_clock::now();
        
        // Genesis transaction (coin creation)
        Transaction genesisTx("genesis", "genesis", 1000000, 0);
        genesisTx.generateTxId();
        genesis.addTransaction(genesisTx);
        
        genesis.mine("genesis");
        
        std::lock_guard<std::mutex> lock(chainMutex);
        chain.push_back(genesis);
        
        // Initialize genesis UTXO
        UTXO genesisUtxo(genesisTx.txId, 0, "genesis", 1000000);
        utxoSet[genesisUtxo.getKey()] = genesisUtxo;
        balances["genesis"] = 1000000;
        
        std::cout << "Genesis block created!" << std::endl;
    }
    
    bool isChainValid() const {
        std::lock_guard<std::mutex> lock(chainMutex);
        
        for (size_t i = 1; i < chain.size(); i++) {
            const Block& currentBlock = chain[i];
            const Block& previousBlock = chain[i - 1];
            
            if (!currentBlock.isValid()) {
                return false;
            }
            
            if (currentBlock.previousHash != previousBlock.hash) {
                return false;
            }
        }
        
        return true;
    }
    
    void addTransaction(const Transaction& tx) {
        // Validate transaction
        if (!validateTransaction(tx)) {
            std::cout << "Transaction validation failed: " << tx.toString() << std::endl;
            return;
        }
        
        mempool.addTransaction(tx);
    }
    
    bool validateTransaction(const Transaction& tx) const {
        // Basic validation
        if (tx.amount <= 0 || tx.fee < 0) return false;
        if (tx.sender == tx.receiver) return false;
        if (!tx.verify()) return false;
        
        // Check balance (simplified - in reality would check UTXOs)
        auto it = balances.find(tx.sender);
        if (it == balances.end()) return false;
        
        return it->second >= tx.getTotalAmount();
    }
    
    void startMining(const std::string& miner) {
        if (miningActive) return;
        
        minerAddress = miner;
        miningActive = true;
        miningThread = std::thread(&Blockchain::miningLoop, this);
        
        std::cout << "Mining started by: " << miner.substr(0, 16) << "..." << std::endl;
    }
    
    void stopMining() {
        miningActive = false;
        if (miningThread.joinable()) {
            miningThread.join();
        }
    }
    
    double getBalance(const std::string& address) const {
        auto it = balances.find(address);
        return (it != balances.end()) ? it->second : 0.0;
    }
    
    size_t getChainLength() const {
        std::lock_guard<std::mutex> lock(chainMutex);
        return chain.size();
    }
    
    Block getBlock(size_t index) const {
        std::lock_guard<std::mutex> lock(chainMutex);
        if (index < chain.size()) {
            return chain[index];
        }
        throw std::out_of_range("Block index out of range");
    }
    
    Block getLatestBlock() const {
        std::lock_guard<std::mutex> lock(chainMutex);
        return chain.back();
    }
    
    std::vector<Transaction> getTransactionHistory(const std::string& address) const {
        std::lock_guard<std::mutex> lock(chainMutex);
        std::vector<Transaction> history;
        
        for (const auto& block : chain) {
            for (const auto& tx : block.transactions) {
                if (tx.sender == address || tx.receiver == address) {
                    history.push_back(tx);
                }
            }
        }
        
        return history;
    }
    
    void printBlockchain() const {
        std::lock_guard<std::mutex> lock(chainMutex);
        
        std::cout << "\n=== BLOCKCHAIN STATUS ===" << std::endl;
        std::cout << "Chain length: " << chain.size() << std::endl;
        std::cout << "Current difficulty: " << difficulty << std::endl;
        std::cout << "Mempool size: " << mempool.size() << std::endl;
        std::cout << "Mining active: " << (miningActive ? "Yes" : "No") << std::endl;
        
        std::cout << "\n--- RECENT BLOCKS ---" << std::endl;
        size_t startIdx = (chain.size() > 3) ? chain.size() - 3 : 0;
        for (size_t i = startIdx; i < chain.size(); i++) {
            std::cout << chain[i].toString() << std::endl;
        }
        
        std::cout << "--- BALANCES ---" << std::endl;
        for (const auto& balance : balances) {
            if (balance.second > 0) {
                std::cout << balance.first.substr(0, 16) << "...: " 
                          << balance.second << " coins" << std::endl;
            }
        }
        std::cout << std::endl;
    }
    
    // Network simulation methods
    void addNetworkNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(networkMutex);
        networkNodes.push_back(nodeId);
    }
    
    void broadcastTransaction(const Transaction& tx) {
        // Simulate network broadcast
        std::cout << "Broadcasting transaction: " << tx.toString() << std::endl;
        addTransaction(tx);
    }
    
    void broadcastBlock(const Block& block) {
        // Simulate network broadcast
        std::cout << "Broadcasting new block: " << block.index << std::endl;
    }
    
    // Advanced features
    void adjustDifficulty() {
        std::lock_guard<std::mutex> lock(chainMutex);
        
        if (chain.size() < 10) return;
        
        // Calculate average block time for last 10 blocks
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(
            chain.back().timestamp - chain[chain.size() - 10].timestamp);
        
        double avgBlockTime = totalTime.count() / 10.0;
        double targetTime = 60.0; // 1 minute per block
        
        if (avgBlockTime < targetTime * 0.8) {
            difficulty++;
            std::cout << "Difficulty increased to: " << difficulty << std::endl;
        } else if (avgBlockTime > targetTime * 1.2 && difficulty > 1) {
            difficulty--;
            std::cout << "Difficulty decreased to: " << difficulty << std::endl;
        }
    }
    
    void rewardMiner(const std::string& miner, double amount) {
        balances[miner] += amount;
    }
    
    // Statistics
    struct BlockchainStats {
        size_t totalBlocks;
        size_t totalTransactions;
        double totalCoinsInCirculation;
        double averageBlockTime;
        int currentDifficulty;
        size_t mempoolSize;
        std::map<std::string, double> topBalances;
    };
    
    BlockchainStats getStats() const {
        std::lock_guard<std::mutex> lock(chainMutex);
        
        BlockchainStats stats;
        stats.totalBlocks = chain.size();
        stats.currentDifficulty = difficulty;
        stats.mempoolSize = mempool.size();
        
        stats.totalTransactions = 0;
        for (const auto& block : chain) {
            stats.totalTransactions += block.transactions.size();
        }
        
        stats.totalCoinsInCirculation = 0;
        for (const auto& balance : balances) {
            stats.totalCoinsInCirculation += balance.second;
        }
        
        // Calculate average block time
        if (chain.size() > 1) {
            auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(
                chain.back().timestamp - chain.front().timestamp);
            stats.averageBlockTime = totalTime.count() / (chain.size() - 1.0);
        } else {
            stats.averageBlockTime = 0;
        }
        
        // Top 5 balances
        std::vector<std::pair<std::string, double>> sortedBalances(balances.begin(), balances.end());
        std::sort(sortedBalances.begin(), sortedBalances.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        for (size_t i = 0; i < std::min(size_t(5), sortedBalances.size()); i++) {
            stats.topBalances[sortedBalances[i].first] = sortedBalances[i].second;
        }
        
        return stats;
    }
    
private:
    void miningLoop() {
        while (miningActive) {
            // Get transactions from mempool
            auto transactions = mempool.getTransactions(10);
            
            if (!transactions.empty()) {
                // Create new block
                Block newBlock(getChainLength(), getLatestBlock().hash, difficulty);
                
                // Add coinbase transaction (mining reward)
                Transaction coinbase("coinbase", minerAddress, blockReward, 0);
                coinbase.generateTxId();
                newBlock.addTransaction(coinbase);
                
                // Add transactions from mempool
                for (const auto& tx : transactions) {
                    newBlock.addTransaction(tx);
                }
                
                // Mine the block
                if (newBlock.mine(minerAddress)) {
                    // Add block to chain
                    {
                        std::lock_guard<std::mutex> lock(chainMutex);
                        chain.push_back(newBlock);
                        
                        // Update balances
                        for (const auto& tx : newBlock.transactions) {
                            if (tx.sender != "coinbase") {
                                balances[tx.sender] -= tx.getTotalAmount();
                            }
                            balances[tx.receiver] += tx.amount;
                        }
                        
                        // Reward miner with fees
                        balances[minerAddress] += newBlock.getTotalFees();
                    }
                    
                    // Remove mined transactions from mempool
                    std::vector<Transaction> minedTxs(transactions.begin(), transactions.end());
                    mempool.removeTransactions(minedTxs);
                    
                    // Broadcast new block
                    broadcastBlock(newBlock);
                    
                    // Adjust difficulty periodically
                    if (getChainLength() % 10 == 0) {
                        adjustDifficulty();
                    }
                }
            } else {
                // No transactions to mine, wait a bit
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
};

// Blockchain network simulator
class BlockchainNetwork {
private:
    std::vector<std::shared_ptr<Blockchain>> nodes;
    std::vector<std::shared_ptr<Wallet>> wallets;
    std::mutex networkMutex;
    
public:
    void addNode() {
        auto node = std::make_shared<Blockchain>();
        std::lock_guard<std::mutex> lock(networkMutex);
        nodes.push_back(node);
        std::cout << "Added blockchain node. Total nodes: " << nodes.size() << std::endl;
    }
    
    void addWallet() {
        auto wallet = std::make_shared<Wallet>();
        std::lock_guard<std::mutex> lock(networkMutex);
        wallets.push_back(wallet);
        std::cout << "Created new wallet: " << wallet->getAddress().substr(0, 16) << "..." << std::endl;
    }
    
    std::shared_ptr<Blockchain> getNode(size_t index) {
        std::lock_guard<std::mutex> lock(networkMutex);
        if (index < nodes.size()) {
            return nodes[index];
        }
        return nullptr;
    }
    
    std::shared_ptr<Wallet> getWallet(size_t index) {
        std::lock_guard<std::mutex> lock(networkMutex);
        if (index < wallets.size()) {
            return wallets[index];
        }
        return nullptr;
    }
    
    void simulateTransactions(int count) {
        if (wallets.size() < 2 || nodes.empty()) return;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> walletDist(0, wallets.size() - 1);
        std::uniform_real_distribution<> amountDist(1.0, 100.0);
        
        auto blockchain = nodes[0]; // Use first node
        
        for (int i = 0; i < count; i++) {
            size_t senderIdx = walletDist(gen);
            size_t receiverIdx = walletDist(gen);
            
            if (senderIdx == receiverIdx) continue;
            
            auto sender = wallets[senderIdx];
            auto receiver = wallets[receiverIdx];
            
            double amount = amountDist(gen);
            double senderBalance = blockchain->getBalance(sender->getAddress());
            
            if (senderBalance >= amount + 0.001) { // Check if sender has enough balance
                Transaction tx = sender->createTransaction(receiver->getAddress(), amount);
                blockchain->broadcastTransaction(tx);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    void printNetworkStatus() {
        std::lock_guard<std::mutex> lock(networkMutex);
        
        std::cout << "\n=== BLOCKCHAIN NETWORK STATUS ===" << std::endl;
        std::cout << "Active nodes: " << nodes.size() << std::endl;
        std::cout << "Active wallets: " << wallets.size() << std::endl;
        
        if (!nodes.empty()) {
            auto stats = nodes[0]->getStats();
            std::cout << "\n--- NETWORK STATISTICS ---" << std::endl;
            std::cout << "Total blocks: " << stats.totalBlocks << std::endl;
            std::cout << "Total transactions: " << stats.totalTransactions << std::endl;
            std::cout << "Total coins: " << std::fixed << std::setprecision(2) 
                      << stats.totalCoinsInCirculation << std::endl;
            std::cout << "Average block time: " << stats.averageBlockTime << "s" << std::endl;
            std::cout << "Current difficulty: " << stats.currentDifficulty << std::endl;
            std::cout << "Mempool size: " << stats.mempoolSize << std::endl;
            
            std::cout << "\n--- TOP BALANCES ---" << std::endl;
            for (const auto& balance : stats.topBalances) {
                std::cout << balance.first.substr(0, 16) << "...: " 
                          << std::fixed << std::setprecision(2) 
                          << balance.second << " coins" << std::endl;
            }
        }
        std::cout << std::endl;
    }
};

// Demo function
void runBlockchainDemo() {
    std::cout << "=== BLOCKCHAIN IMPLEMENTATION DEMO ===" << std::endl;
    std::cout << "Initializing blockchain network..." << std::endl;
    
    BlockchainNetwork network;
    
    // Add blockchain node
    network.addNode();
    auto blockchain = network.getNode(0);
    
    // Create wallets
    std::cout << "\nCreating wallets..." << std::endl;
    for (int i = 0; i < 5; i++) {
        network.addWallet();
    }
    
    // Distribute initial coins from genesis
    std::cout << "\nDistributing initial coins..." << std::endl;
    auto genesisWallet = std::make_shared<Wallet>();
    // Simulate genesis wallet having the genesis address
    
    for (int i = 0; i < 5; i++) {
        auto wallet = network.getWallet(i);
        if (wallet) {
            Transaction initialTx("genesis", wallet->getAddress(), 1000.0, 0.1);
            blockchain->addTransaction(initialTx);
        }
    }
    
    // Start mining
    std::cout << "\nStarting mining..." << std::endl;
    auto minerWallet = network.getWallet(0);
    if (minerWallet) {
        blockchain->startMining(minerWallet->getAddress());
    }
    
    // Wait for initial mining
    std::cout << "Waiting for initial blocks to be mined..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Simulate transactions
    std::cout << "\nSimulating transactions..." << std::endl;
    network.simulateTransactions(10);
    
    // Monitor blockchain for a while
    for (int i = 0; i < 8; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        network.printNetworkStatus();
        
        if (i % 3 == 0) {
            std::cout << "Adding more transactions..." << std::endl;
            network.simulateTransactions(5);
        }
    }
    
    // Print final blockchain state
    std::cout << "\nFinal blockchain state:" << std::endl;
    blockchain->printBlockchain();
    
    // Validate blockchain
    std::cout << "Blockchain validation: " 
              << (blockchain->isChainValid() ? "VALID" : "INVALID") << std::endl;
    
    blockchain->stopMining();
    std::cout << "\nBlockchain demo completed!" << std::endl;
}

int main() {
    try {
        runBlockchainDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
