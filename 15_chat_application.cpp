#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <queue>
#include <condition_variable>
#include <atomic>

using namespace std;

// Struct to represent a chat message
struct Message {
    string sender;
    string content;
    string timestamp;
    
    Message(const string& s, const string& c) : sender(s), content(c) {
        // Generate timestamp
        time_t now = time(0);
        tm* ltm = localtime(&now);
        
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", ltm);
        timestamp = string(buffer);
    }
    
    void display() const {
        cout << "[" << timestamp << "] " << sender << ": " << content << endl;
    }
};

// Thread-safe message queue for communication between threads
class MessageQueue {
private:
    queue<Message> messages;
    mutable mutex mtx;
    condition_variable cv;
    
public:
    void push(const Message& msg) {
        unique_lock<mutex> lock(mtx);
        messages.push(msg);
        lock.unlock();
        cv.notify_one();
    }
    
    bool pop(Message& msg) {
        unique_lock<mutex> lock(mtx);
        
        if (messages.empty()) {
            return false;
        }
        
        msg = messages.front();
        messages.pop();
        return true;
    }
    
    bool waitAndPop(Message& msg) {
        unique_lock<mutex> lock(mtx);
        
        cv.wait(lock, [this] { return !messages.empty(); });
        
        msg = messages.front();
        messages.pop();
        return true;
    }
    
    bool empty() const {
        unique_lock<mutex> lock(mtx);
        return messages.empty();
    }
};

// Class to represent a chat room
class ChatRoom {
private:
    vector<Message> history;
    MessageQueue messageQueue;
    mutex historyMutex;
    
public:
    void addMessage(const Message& msg) {
        // Add to history
        unique_lock<mutex> lock(historyMutex);
        history.push_back(msg);
        lock.unlock();
        
        // Add to queue for real-time display
        messageQueue.push(msg);
    }
    
    void displayHistory() const {
        unique_lock<mutex> lock(historyMutex);
        
        cout << "\n===== Chat History =====" << endl;
        
        if (history.empty()) {
            cout << "No messages yet." << endl;
        } else {
            for (const auto& msg : history) {
                msg.display();
            }
        }
        
        cout << "=======================" << endl;
    }
    
    MessageQueue& getMessageQueue() {
        return messageQueue;
    }
};

// Class to represent a chat user
class User {
private:
    string username;
    ChatRoom* chatRoom;
    
public:
    User(const string& name, ChatRoom* room) : username(name), chatRoom(room) {}
    
    void sendMessage(const string& content) {
        Message msg(username, content);
        chatRoom->addMessage(msg);
    }
    
    string getUsername() const {
        return username;
    }
};

// Function to simulate receiving messages from other users
void simulateOtherUsers(ChatRoom& chatRoom, atomic<bool>& running) {
    vector<string> users = {"Alice", "Bob", "Charlie", "David"};
    vector<string> messages = {
        "Hello everyone!",
        "How are you doing today?",
        "I'm working on a new project.",
        "Did you see the news?",
        "Let's meet tomorrow.",
        "I'll be late for the meeting.",
        "Can someone help me with this problem?",
        "I found a solution!",
        "That's interesting.",
        "I disagree with that."
    };
    
    // Random seed
    srand(static_cast<unsigned int>(time(nullptr)));
    
    while (running) {
        // Sleep for a random time between 5-15 seconds
        int sleepTime = 5000 + (rand() % 10000);
        this_thread::sleep_for(chrono::milliseconds(sleepTime));
        
        // Select a random user and message
        string user = users[rand() % users.size()];
        string message = messages[rand() % messages.size()];
        
        // Send the message
        Message msg(user, message);
        chatRoom.addMessage(msg);
    }
}

// Function to display incoming messages in real-time
void displayIncomingMessages(ChatRoom& chatRoom, atomic<bool>& running) {
    MessageQueue& queue = chatRoom.getMessageQueue();
    
    while (running) {
        Message msg("", "");
        
        if (queue.waitAndPop(msg)) {
            // Clear the current line
            cout << "\r                                                                               \r";
            
            // Display the message
            msg.display();
            
            // Redisplay the prompt
            cout << "Enter message (or /exit to quit, /history to view chat history): ";
            cout.flush();
        }
    }
}

int main() {
    ChatRoom chatRoom;
    
    string username;
    cout << "Enter your username: ";
    getline(cin, username);
    
    User currentUser(username, &chatRoom);
    
    cout << "Welcome to the chat room, " << username << "!" << endl;
    cout << "Type your messages and press Enter to send." << endl;
    cout << "Type /exit to quit or /history to view chat history." << endl;
    
    // Flag to control background threads
    atomic<bool> running(true);
    
    // Start the thread to simulate other users
    thread otherUsersThread(simulateOtherUsers, ref(chatRoom), ref(running));
    
    // Start the thread to display incoming messages
    thread displayThread(displayIncomingMessages, ref(chatRoom), ref(running));
    
    // Main loop for user input
    while (true) {
        cout << "Enter message (or /exit to quit, /history to view chat history): ";
        string input;
        getline(cin, input);
        
        if (input == "/exit") {
            break;
        } else if (input == "/history") {
            chatRoom.displayHistory();
        } else if (!input.empty()) {
            currentUser.sendMessage(input);
        }
    }
    
    // Signal threads to stop and wait for them
    running = false;
    
    // Send a dummy message to wake up the display thread
    Message dummyMsg("", "");
    chatRoom.getMessageQueue().push(dummyMsg);
    
    // Join threads
    if (otherUsersThread.joinable()) {
        otherUsersThread.join();
    }
    
    if (displayThread.joinable()) {
        displayThread.join();
    }
    
    cout << "Goodbye!" << endl;
    
    return 0;
} 