#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <regex>
#include <set>
#include <limits>

using namespace std;

// Class to represent text statistics
class TextStatistics {
private:
    string text;
    vector<string> words;
    vector<string> sentences;
    vector<string> paragraphs;
    map<string, int> wordFrequency;
    
    // Helper function to convert to lowercase
    string toLower(const string& s) const {
        string result = s;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    // Helper function to check if a character is a letter
    bool isLetter(char c) const {
        return isalpha(c) || c == '\'' || c == '-';
    }
    
    // Helper function to clean a word (remove punctuation)
    string cleanWord(const string& word) const {
        string result;
        for (size_t i = 0; i < word.length(); i++) {
            if (isLetter(word[i])) {
                result += word[i];
            }
        }
        return result;
    }
    
    // Parse the text into words, sentences, and paragraphs
    void parseText() {
        // Clear previous data
        words.clear();
        sentences.clear();
        paragraphs.clear();
        wordFrequency.clear();
        
        // Parse paragraphs
        istringstream paragraphStream(text);
        string paragraph;
        while (getline(paragraphStream, paragraph, '\n')) {
            if (!paragraph.empty()) {
                paragraphs.push_back(paragraph);
                
                // Parse sentences
                regex sentenceRegex("[^.!?]+[.!?]+");
                sregex_iterator sentenceIter(paragraph.begin(), paragraph.end(), sentenceRegex);
                sregex_iterator sentenceEnd;
                
                while (sentenceIter != sentenceEnd) {
                    string sentence = (*sentenceIter).str();
                    sentences.push_back(sentence);
                    
                    // Parse words
                    istringstream wordStream(sentence);
                    string word;
                    while (wordStream >> word) {
                        string cleanedWord = cleanWord(word);
                        if (!cleanedWord.empty()) {
                            words.push_back(cleanedWord);
                            wordFrequency[toLower(cleanedWord)]++;
                        }
                    }
                    
                    ++sentenceIter;
                }
            }
        }
    }
    
public:
    TextStatistics() {}
    
    // Load text from a string
    void loadText(const string& textContent) {
        text = textContent;
        parseText();
    }
    
    // Load text from a file
    bool loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file) {
            return false;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        text = buffer.str();
        
        file.close();
        parseText();
        return true;
    }
    
    // Get the total number of characters
    size_t getCharacterCount() const {
        return text.length();
    }
    
    // Get the number of characters excluding whitespace
    size_t getCharacterCountNoSpaces() const {
        size_t count = 0;
        for (char c : text) {
            if (!isspace(c)) {
                count++;
            }
        }
        return count;
    }
    
    // Get the total number of words
    size_t getWordCount() const {
        return words.size();
    }
    
    // Get the total number of sentences
    size_t getSentenceCount() const {
        return sentences.size();
    }
    
    // Get the total number of paragraphs
    size_t getParagraphCount() const {
        return paragraphs.size();
    }
    
    // Get the average word length
    double getAverageWordLength() const {
        if (words.empty()) {
            return 0.0;
        }
        
        size_t totalLength = 0;
        for (const string& word : words) {
            totalLength += word.length();
        }
        
        return static_cast<double>(totalLength) / words.size();
    }
    
    // Get the average sentence length (in words)
    double getAverageSentenceLength() const {
        if (sentences.empty()) {
            return 0.0;
        }
        
        return static_cast<double>(words.size()) / sentences.size();
    }
    
    // Get the average paragraph length (in sentences)
    double getAverageParagraphLength() const {
        if (paragraphs.empty()) {
            return 0.0;
        }
        
        return static_cast<double>(sentences.size()) / paragraphs.size();
    }
    
    // Get the most frequent words
    vector<pair<string, int>> getMostFrequentWords(size_t count = 10) const {
        vector<pair<string, int>> result(wordFrequency.begin(), wordFrequency.end());
        
        // Sort by frequency (descending)
        sort(result.begin(), result.end(), 
             [](const pair<string, int>& a, const pair<string, int>& b) {
                 return a.second > b.second;
             });
        
        // Return only the requested number of words
        if (result.size() > count) {
            result.resize(count);
        }
        
        return result;
    }
    
    // Get the unique word count
    size_t getUniqueWordCount() const {
        return wordFrequency.size();
    }
    
    // Calculate the lexical diversity (unique words / total words)
    double getLexicalDiversity() const {
        if (words.empty()) {
            return 0.0;
        }
        
        return static_cast<double>(wordFrequency.size()) / words.size();
    }
    
    // Get the word frequency
    const map<string, int>& getWordFrequency() const {
        return wordFrequency;
    }
    
    // Get the sentences
    const vector<string>& getSentences() const {
        return sentences;
    }
    
    // Get the paragraphs
    const vector<string>& getParagraphs() const {
        return paragraphs;
    }
    
    // Get the words
    const vector<string>& getWords() const {
        return words;
    }
    
    // Calculate the readability score (Flesch-Kincaid Grade Level)
    double getFleschKincaidGradeLevel() const {
        if (words.empty() || sentences.empty()) {
            return 0.0;
        }
        
        // Count syllables
        int totalSyllables = 0;
        for (const string& word : words) {
            totalSyllables += countSyllables(word);
        }
        
        // Calculate the Flesch-Kincaid Grade Level
        double wordsPerSentence = static_cast<double>(words.size()) / sentences.size();
        double syllablesPerWord = static_cast<double>(totalSyllables) / words.size();
        
        return 0.39 * wordsPerSentence + 11.8 * syllablesPerWord - 15.59;
    }
    
    // Count syllables in a word (simplified algorithm)
    int countSyllables(const string& word) const {
        string lowercaseWord = toLower(word);
        int count = 0;
        bool prevIsVowel = false;
        
        // Define vowels
        const string vowels = "aeiouy";
        
        // Count syllables
        for (size_t i = 0; i < lowercaseWord.length(); i++) {
            bool isVowel = vowels.find(lowercaseWord[i]) != string::npos;
            
            if (isVowel && !prevIsVowel) {
                count++;
            }
            
            prevIsVowel = isVowel;
        }
        
        // Handle special cases
        if (count == 0) {
            count = 1; // Every word has at least one syllable
        }
        
        // Handle silent 'e' at the end
        if (lowercaseWord.length() > 2 && 
            lowercaseWord[lowercaseWord.length() - 1] == 'e' && 
            !prevIsVowel) {
            count--;
        }
        
        return count;
    }
    
    // Find the most common n-grams (sequences of n words)
    vector<pair<string, int>> getMostCommonNGrams(size_t n, size_t count = 10) const {
        if (words.size() < n) {
            return {};
        }
        
        map<string, int> ngramFrequency;
        
        for (size_t i = 0; i <= words.size() - n; i++) {
            string ngram;
            for (size_t j = 0; j < n; j++) {
                if (j > 0) {
                    ngram += " ";
                }
                ngram += toLower(words[i + j]);
            }
            ngramFrequency[ngram]++;
        }
        
        vector<pair<string, int>> result(ngramFrequency.begin(), ngramFrequency.end());
        
        // Sort by frequency (descending)
        sort(result.begin(), result.end(), 
             [](const pair<string, int>& a, const pair<string, int>& b) {
                 return a.second > b.second;
             });
        
        // Return only the requested number of n-grams
        if (result.size() > count) {
            result.resize(count);
        }
        
        return result;
    }
    
    // Find the longest word
    string getLongestWord() const {
        if (words.empty()) {
            return "";
        }
        
        return *max_element(words.begin(), words.end(), 
                           [](const string& a, const string& b) {
                               return a.length() < b.length();
                           });
    }
    
    // Find the shortest word
    string getShortestWord() const {
        if (words.empty()) {
            return "";
        }
        
        return *min_element(words.begin(), words.end(), 
                           [](const string& a, const string& b) {
                               return a.length() < b.length();
                           });
    }
    
    // Search for a word or phrase
    vector<size_t> searchText(const string& searchTerm) const {
        vector<size_t> positions;
        string lowercaseText = toLower(text);
        string lowercaseSearchTerm = toLower(searchTerm);
        
        size_t pos = 0;
        while ((pos = lowercaseText.find(lowercaseSearchTerm, pos)) != string::npos) {
            positions.push_back(pos);
            pos += lowercaseSearchTerm.length();
        }
        
        return positions;
    }
    
    // Get the context around a position
    string getContext(size_t position, size_t contextSize = 50) const {
        if (position >= text.length()) {
            return "";
        }
        
        size_t start = (position > contextSize) ? position - contextSize : 0;
        size_t end = min(position + contextSize, text.length());
        
        return text.substr(start, end - start);
    }
};

// Function to display the main menu
void displayMenu() {
    cout << "\n===== Text Analyzer =====" << endl;
    cout << "1. Load text from file" << endl;
    cout << "2. Enter text manually" << endl;
    cout << "3. Display basic statistics" << endl;
    cout << "4. Display word frequency" << endl;
    cout << "5. Display most frequent words" << endl;
    cout << "6. Display readability metrics" << endl;
    cout << "7. Search for word/phrase" << endl;
    cout << "8. Display common n-grams" << endl;
    cout << "9. Exit" << endl;
    cout << "=========================" << endl;
    cout << "Enter your choice: ";
}

// Function to display basic statistics
void displayBasicStatistics(const TextStatistics& stats) {
    cout << "\n===== Basic Statistics =====" << endl;
    cout << "Character count: " << stats.getCharacterCount() << endl;
    cout << "Character count (no spaces): " << stats.getCharacterCountNoSpaces() << endl;
    cout << "Word count: " << stats.getWordCount() << endl;
    cout << "Sentence count: " << stats.getSentenceCount() << endl;
    cout << "Paragraph count: " << stats.getParagraphCount() << endl;
    cout << "Average word length: " << fixed << setprecision(2) << stats.getAverageWordLength() << " characters" << endl;
    cout << "Average sentence length: " << fixed << setprecision(2) << stats.getAverageSentenceLength() << " words" << endl;
    cout << "Average paragraph length: " << fixed << setprecision(2) << stats.getAverageParagraphLength() << " sentences" << endl;
    cout << "Unique word count: " << stats.getUniqueWordCount() << endl;
    cout << "Lexical diversity: " << fixed << setprecision(2) << stats.getLexicalDiversity() << endl;
    cout << "Longest word: \"" << stats.getLongestWord() << "\" (" << stats.getLongestWord().length() << " characters)" << endl;
    cout << "Shortest word: \"" << stats.getShortestWord() << "\" (" << stats.getShortestWord().length() << " characters)" << endl;
}

// Function to display word frequency
void displayWordFrequency(const TextStatistics& stats) {
    cout << "\n===== Word Frequency =====" << endl;
    
    const map<string, int>& frequency = stats.getWordFrequency();
    vector<pair<string, int>> sortedFrequency(frequency.begin(), frequency.end());
    
    // Sort alphabetically
    sort(sortedFrequency.begin(), sortedFrequency.end(), 
         [](const pair<string, int>& a, const pair<string, int>& b) {
             return a.first < b.first;
         });
    
    cout << left << setw(20) << "Word" << right << setw(10) << "Frequency" << endl;
    cout << string(30, '-') << endl;
    
    for (const auto& pair : sortedFrequency) {
        cout << left << setw(20) << pair.first << right << setw(10) << pair.second << endl;
    }
}

// Function to display most frequent words
void displayMostFrequentWords(const TextStatistics& stats) {
    cout << "\n===== Most Frequent Words =====" << endl;
    
    size_t count;
    cout << "Enter the number of words to display: ";
    cin >> count;
    
    vector<pair<string, int>> mostFrequent = stats.getMostFrequentWords(count);
    
    cout << left << setw(20) << "Word" << right << setw(10) << "Frequency" << endl;
    cout << string(30, '-') << endl;
    
    for (const auto& pair : mostFrequent) {
        cout << left << setw(20) << pair.first << right << setw(10) << pair.second << endl;
    }
}

// Function to display readability metrics
void displayReadabilityMetrics(const TextStatistics& stats) {
    cout << "\n===== Readability Metrics =====" << endl;
    
    double fkgl = stats.getFleschKincaidGradeLevel();
    cout << "Flesch-Kincaid Grade Level: " << fixed << setprecision(1) << fkgl << endl;
    
    cout << "This text is suitable for readers at approximately ";
    if (fkgl < 1.0) {
        cout << "kindergarten level.";
    } else if (fkgl < 13.0) {
        cout << "grade " << static_cast<int>(fkgl) << " level.";
    } else if (fkgl < 16.0) {
        cout << "college level.";
    } else {
        cout << "graduate level.";
    }
    cout << endl;
}

// Function to search for a word or phrase
void searchForWordOrPhrase(const TextStatistics& stats) {
    cout << "\n===== Search Text =====" << endl;
    
    string searchTerm;
    cout << "Enter word or phrase to search: ";
    cin.ignore();
    getline(cin, searchTerm);
    
    vector<size_t> positions = stats.searchText(searchTerm);
    
    if (positions.empty()) {
        cout << "The search term \"" << searchTerm << "\" was not found in the text." << endl;
    } else {
        cout << "The search term \"" << searchTerm << "\" was found " << positions.size() << " time(s)." << endl;
        
        // Display context for the first few occurrences
        const size_t maxContexts = 5;
        for (size_t i = 0; i < min(positions.size(), maxContexts); i++) {
            string context = stats.getContext(positions[i]);
            
            // Highlight the search term in the context
            size_t termPos = context.find(searchTerm, 0);
            if (termPos != string::npos) {
                cout << "\nOccurrence " << (i + 1) << ":" << endl;
                cout << "..." << context.substr(0, termPos);
                cout << "[" << searchTerm << "]";
                cout << context.substr(termPos + searchTerm.length()) << "..." << endl;
            }
        }
        
        if (positions.size() > maxContexts) {
            cout << "\n(Showing " << maxContexts << " out of " << positions.size() << " occurrences)" << endl;
        }
    }
}

// Function to display common n-grams
void displayCommonNGrams(const TextStatistics& stats) {
    cout << "\n===== Common N-Grams =====" << endl;
    
    size_t n;
    cout << "Enter the value of n (2 for bigrams, 3 for trigrams, etc.): ";
    cin >> n;
    
    size_t count;
    cout << "Enter the number of n-grams to display: ";
    cin >> count;
    
    vector<pair<string, int>> commonNGrams = stats.getMostCommonNGrams(n, count);
    
    if (commonNGrams.empty()) {
        cout << "No " << n << "-grams found in the text." << endl;
        return;
    }
    
    cout << left << setw(30) << n << "-gram" << right << setw(10) << "Frequency" << endl;
    cout << string(40, '-') << endl;
    
    for (const auto& pair : commonNGrams) {
        cout << left << setw(30) << pair.first << right << setw(10) << pair.second << endl;
    }
}

int main() {
    TextStatistics textStats;
    bool textLoaded = false;
    int choice;
    
    cout << "Welcome to Text Analyzer!" << endl;
    
    while (true) {
        displayMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: { // Load text from file
                string filename;
                cout << "Enter the filename: ";
                getline(cin, filename);
                
                if (textStats.loadFromFile(filename)) {
                    cout << "Text loaded successfully from " << filename << endl;
                    textLoaded = true;
                } else {
                    cout << "Error: Could not load text from " << filename << endl;
                }
                break;
            }
                
            case 2: { // Enter text manually
                string text;
                cout << "Enter your text (end with an empty line):" << endl;
                
                string line;
                while (getline(cin, line) && !line.empty()) {
                    text += line + "\n";
                }
                
                textStats.loadText(text);
                cout << "Text loaded successfully." << endl;
                textLoaded = true;
                break;
            }
                
            case 3: // Display basic statistics
                if (!textLoaded) {
                    cout << "Please load text first." << endl;
                    break;
                }
                displayBasicStatistics(textStats);
                break;
                
            case 4: // Display word frequency
                if (!textLoaded) {
                    cout << "Please load text first." << endl;
                    break;
                }
                displayWordFrequency(textStats);
                break;
                
            case 5: // Display most frequent words
                if (!textLoaded) {
                    cout << "Please load text first." << endl;
                    break;
                }
                displayMostFrequentWords(textStats);
                break;
                
            case 6: // Display readability metrics
                if (!textLoaded) {
                    cout << "Please load text first." << endl;
                    break;
                }
                displayReadabilityMetrics(textStats);
                break;
                
            case 7: // Search for word/phrase
                if (!textLoaded) {
                    cout << "Please load text first." << endl;
                    break;
                }
                searchForWordOrPhrase(textStats);
                break;
                
            case 8: // Display common n-grams
                if (!textLoaded) {
                    cout << "Please load text first." << endl;
                    break;
                }
                displayCommonNGrams(textStats);
                break;
                
            case 9: // Exit
                cout << "Thank you for using Text Analyzer!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 