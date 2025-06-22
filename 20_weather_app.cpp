#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <limits>
#include <algorithm>

using namespace std;

// Weather condition enum
enum class WeatherCondition {
    SUNNY,
    PARTLY_CLOUDY,
    CLOUDY,
    RAINY,
    THUNDERSTORM,
    SNOWY,
    FOGGY
};

// Weather data structure
struct WeatherData {
    double temperature; // in Celsius
    double humidity; // percentage
    double windSpeed; // km/h
    double precipitation; // mm
    WeatherCondition condition;
    time_t timestamp;
};

// City class
class City {
private:
    string name;
    string country;
    double latitude;
    double longitude;
    vector<WeatherData> weatherHistory;
    WeatherData currentWeather;
    vector<WeatherData> forecast;

public:
    // Constructor
    City(const string& n, const string& c, double lat, double lon)
        : name(n), country(c), latitude(lat), longitude(lon) {
        // Initialize with random current weather
        generateRandomWeather(currentWeather);
        
        // Generate forecast for the next 7 days
        generateForecast(7);
    }
    
    // Getters
    string getName() const { return name; }
    string getCountry() const { return country; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }
    const WeatherData& getCurrentWeather() const { return currentWeather; }
    const vector<WeatherData>& getForecast() const { return forecast; }
    const vector<WeatherData>& getWeatherHistory() const { return weatherHistory; }
    
    // Update the current weather
    void updateCurrentWeather() {
        // Save the current weather to history
        weatherHistory.push_back(currentWeather);
        
        // Generate new weather with some continuity from previous
        WeatherData newWeather;
        generateContinuousWeather(currentWeather, newWeather);
        currentWeather = newWeather;
        
        // Update the forecast
        updateForecast();
    }
    
    // Generate a forecast for the specified number of days
    void generateForecast(int days) {
        forecast.clear();
        
        WeatherData prevWeather = currentWeather;
        for (int i = 0; i < days; i++) {
            WeatherData forecastData;
            generateContinuousWeather(prevWeather, forecastData);
            
            // Set timestamp to future date
            forecastData.timestamp = currentWeather.timestamp + (i + 1) * 24 * 3600;
            
            forecast.push_back(forecastData);
            prevWeather = forecastData;
        }
    }
    
    // Update the forecast based on current weather
    void updateForecast() {
        // Shift forecast by one day
        if (!forecast.empty()) {
            forecast.erase(forecast.begin());
        }
        
        // Add a new day at the end
        if (!forecast.empty()) {
            WeatherData newDay;
            generateContinuousWeather(forecast.back(), newDay);
            newDay.timestamp = forecast.back().timestamp + 24 * 3600;
            forecast.push_back(newDay);
        }
    }
    
private:
    // Generate random weather data
    void generateRandomWeather(WeatherData& data) {
        // Random temperature between -20 and 40 degrees Celsius
        data.temperature = -20.0 + (rand() % 600) / 10.0;
        
        // Random humidity between 0% and 100%
        data.humidity = rand() % 101;
        
        // Random wind speed between 0 and 100 km/h
        data.windSpeed = (rand() % 1000) / 10.0;
        
        // Random precipitation between 0 and 50 mm
        data.precipitation = (rand() % 500) / 10.0;
        
        // Random weather condition
        data.condition = static_cast<WeatherCondition>(rand() % 7);
        
        // Current timestamp
        data.timestamp = time(nullptr);
    }
    
    // Generate weather with some continuity from previous weather
    void generateContinuousWeather(const WeatherData& prev, WeatherData& next) {
        // Temperature changes by -5 to +5 degrees
        next.temperature = prev.temperature + (-5.0 + (rand() % 100) / 10.0);
        // Clamp temperature between -30 and 50 degrees
        next.temperature = max(-30.0, min(50.0, next.temperature));
        
        // Humidity changes by -20% to +20%
        next.humidity = prev.humidity + (-20.0 + (rand() % 40));
        // Clamp humidity between 0% and 100%
        next.humidity = max(0.0, min(100.0, next.humidity));
        
        // Wind speed changes by -10 to +10 km/h
        next.windSpeed = prev.windSpeed + (-10.0 + (rand() % 200) / 10.0);
        // Clamp wind speed between 0 and 150 km/h
        next.windSpeed = max(0.0, min(150.0, next.windSpeed));
        
        // Precipitation depends on the weather condition
        // Weather condition has some continuity
        int conditionChange = rand() % 10;
        if (conditionChange < 7) {
            // 70% chance to stay the same or move one step
            int direction = rand() % 3 - 1; // -1, 0, or 1
            int newCondition = static_cast<int>(prev.condition) + direction;
            newCondition = max(0, min(6, newCondition));
            next.condition = static_cast<WeatherCondition>(newCondition);
        } else {
            // 30% chance to change randomly
            next.condition = static_cast<WeatherCondition>(rand() % 7);
        }
        
        // Set precipitation based on condition
        switch (next.condition) {
            case WeatherCondition::SUNNY:
            case WeatherCondition::PARTLY_CLOUDY:
                next.precipitation = 0.0;
                break;
            case WeatherCondition::CLOUDY:
            case WeatherCondition::FOGGY:
                next.precipitation = (rand() % 20) / 10.0;
                break;
            case WeatherCondition::RAINY:
                next.precipitation = 0.5 + (rand() % 200) / 10.0;
                break;
            case WeatherCondition::THUNDERSTORM:
                next.precipitation = 5.0 + (rand() % 300) / 10.0;
                break;
            case WeatherCondition::SNOWY:
                next.precipitation = 1.0 + (rand() % 150) / 10.0;
                break;
        }
        
        // Current timestamp
        next.timestamp = time(nullptr);
    }
};

// Weather App class
class WeatherApp {
private:
    vector<City> cities;
    
    // Convert weather condition to string
    string conditionToString(WeatherCondition condition) const {
        switch (condition) {
            case WeatherCondition::SUNNY: return "Sunny";
            case WeatherCondition::PARTLY_CLOUDY: return "Partly Cloudy";
            case WeatherCondition::CLOUDY: return "Cloudy";
            case WeatherCondition::RAINY: return "Rainy";
            case WeatherCondition::THUNDERSTORM: return "Thunderstorm";
            case WeatherCondition::SNOWY: return "Snowy";
            case WeatherCondition::FOGGY: return "Foggy";
            default: return "Unknown";
        }
    }
    
    // Convert timestamp to date string
    string timestampToDate(time_t timestamp) const {
        struct tm* timeinfo = localtime(&timestamp);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
        return string(buffer);
    }
    
    // Convert timestamp to time string
    string timestampToTime(time_t timestamp) const {
        struct tm* timeinfo = localtime(&timestamp);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
        return string(buffer);
    }
    
    // Find city by name
    City* findCity(const string& name) {
        for (auto& city : cities) {
            if (city.getName() == name) {
                return &city;
            }
        }
        return nullptr;
    }
    
public:
    // Constructor
    WeatherApp() {
        // Seed the random number generator
        srand(static_cast<unsigned int>(time(nullptr)));
        
        // Initialize with some cities
        initializeCities();
    }
    
    // Initialize with some predefined cities
    void initializeCities() {
        cities.push_back(City("New York", "USA", 40.7128, -74.0060));
        cities.push_back(City("London", "UK", 51.5074, -0.1278));
        cities.push_back(City("Tokyo", "Japan", 35.6762, 139.6503));
        cities.push_back(City("Sydney", "Australia", -33.8688, 151.2093));
        cities.push_back(City("Rio de Janeiro", "Brazil", -22.9068, -43.1729));
        cities.push_back(City("Cairo", "Egypt", 30.0444, 31.2357));
        cities.push_back(City("Moscow", "Russia", 55.7558, 37.6173));
        cities.push_back(City("Beijing", "China", 39.9042, 116.4074));
        cities.push_back(City("Paris", "France", 48.8566, 2.3522));
        cities.push_back(City("Rome", "Italy", 41.9028, 12.4964));
    }
    
    // Add a new city
    void addCity(const string& name, const string& country, double latitude, double longitude) {
        cities.push_back(City(name, country, latitude, longitude));
    }
    
    // Remove a city
    bool removeCity(const string& name) {
        for (auto it = cities.begin(); it != cities.end(); ++it) {
            if (it->getName() == name) {
                cities.erase(it);
                return true;
            }
        }
        return false;
    }
    
    // Display current weather for a city
    void displayCurrentWeather(const string& cityName) const {
        // Find the city
        const City* city = nullptr;
        for (const auto& c : cities) {
            if (c.getName() == cityName) {
                city = &c;
                break;
            }
        }
        
        if (!city) {
            cout << "City not found: " << cityName << endl;
            return;
        }
        
        // Get the current weather
        const WeatherData& weather = city->getCurrentWeather();
        
        // Display the weather information
        cout << "Current Weather for " << city->getName() << ", " << city->getCountry() << endl;
        cout << "Date: " << timestampToDate(weather.timestamp) << endl;
        cout << "Time: " << timestampToTime(weather.timestamp) << endl;
        cout << "Condition: " << conditionToString(weather.condition) << endl;
        cout << "Temperature: " << fixed << setprecision(1) << weather.temperature << "°C" << endl;
        cout << "Humidity: " << fixed << setprecision(1) << weather.humidity << "%" << endl;
        cout << "Wind Speed: " << fixed << setprecision(1) << weather.windSpeed << " km/h" << endl;
        cout << "Precipitation: " << fixed << setprecision(1) << weather.precipitation << " mm" << endl;
    }
    
    // Display weather forecast for a city
    void displayForecast(const string& cityName) const {
        // Find the city
        const City* city = nullptr;
        for (const auto& c : cities) {
            if (c.getName() == cityName) {
                city = &c;
                break;
            }
        }
        
        if (!city) {
            cout << "City not found: " << cityName << endl;
            return;
        }
        
        // Get the forecast
        const vector<WeatherData>& forecast = city->getForecast();
        
        // Display the forecast
        cout << "Weather Forecast for " << city->getName() << ", " << city->getCountry() << endl;
        cout << left << setw(12) << "Date"
             << left << setw(15) << "Condition"
             << right << setw(10) << "Temp (°C)"
             << right << setw(10) << "Humidity"
             << right << setw(12) << "Wind (km/h)"
             << right << setw(12) << "Precip (mm)" << endl;
        cout << string(71, '-') << endl;
        
        for (const auto& weather : forecast) {
            cout << left << setw(12) << timestampToDate(weather.timestamp)
                 << left << setw(15) << conditionToString(weather.condition)
                 << right << setw(10) << fixed << setprecision(1) << weather.temperature
                 << right << setw(10) << fixed << setprecision(1) << weather.humidity << "%"
                 << right << setw(12) << fixed << setprecision(1) << weather.windSpeed
                 << right << setw(12) << fixed << setprecision(1) << weather.precipitation << endl;
        }
    }
    
    // List all available cities
    void listCities() const {
        cout << "Available Cities:" << endl;
        cout << left << setw(20) << "Name"
             << left << setw(15) << "Country"
             << right << setw(10) << "Latitude"
             << right << setw(10) << "Longitude" << endl;
        cout << string(55, '-') << endl;
        
        for (const auto& city : cities) {
            cout << left << setw(20) << city.getName()
                 << left << setw(15) << city.getCountry()
                 << right << setw(10) << fixed << setprecision(2) << city.getLatitude()
                 << right << setw(10) << fixed << setprecision(2) << city.getLongitude() << endl;
        }
    }
    
    // Update weather for all cities
    void updateAllWeather() {
        for (auto& city : cities) {
            city.updateCurrentWeather();
        }
    }
    
    // Save weather data to file
    bool saveWeatherData(const string& filename) const {
        ofstream file(filename);
        if (!file) {
            return false;
        }
        
        // Save the number of cities
        file << cities.size() << endl;
        
        // Save each city
        for (const auto& city : cities) {
            file << city.getName() << endl;
            file << city.getCountry() << endl;
            file << city.getLatitude() << endl;
            file << city.getLongitude() << endl;
            
            // Save current weather
            const WeatherData& currentWeather = city.getCurrentWeather();
            file << currentWeather.temperature << endl;
            file << currentWeather.humidity << endl;
            file << currentWeather.windSpeed << endl;
            file << currentWeather.precipitation << endl;
            file << static_cast<int>(currentWeather.condition) << endl;
            file << currentWeather.timestamp << endl;
            
            // Save forecast
            const vector<WeatherData>& forecast = city.getForecast();
            file << forecast.size() << endl;
            
            for (const auto& weather : forecast) {
                file << weather.temperature << endl;
                file << weather.humidity << endl;
                file << weather.windSpeed << endl;
                file << weather.precipitation << endl;
                file << static_cast<int>(weather.condition) << endl;
                file << weather.timestamp << endl;
            }
        }
        
        file.close();
        return true;
    }
    
    // Load weather data from file
    bool loadWeatherData(const string& filename) {
        ifstream file(filename);
        if (!file) {
            return false;
        }
        
        // Clear existing cities
        cities.clear();
        
        // Read the number of cities
        size_t numCities;
        file >> numCities;
        file.ignore(numeric_limits<streamsize>::max(), '\n');
        
        // Read each city
        for (size_t i = 0; i < numCities; i++) {
            string name, country;
            double latitude, longitude;
            
            getline(file, name);
            getline(file, country);
            file >> latitude;
            file >> longitude;
            file.ignore(numeric_limits<streamsize>::max(), '\n');
            
            // Create the city
            cities.push_back(City(name, country, latitude, longitude));
            
            // Read current weather
            double temperature, humidity, windSpeed, precipitation;
            int conditionInt;
            time_t timestamp;
            
            file >> temperature;
            file >> humidity;
            file >> windSpeed;
            file >> precipitation;
            file >> conditionInt;
            file >> timestamp;
            file.ignore(numeric_limits<streamsize>::max(), '\n');
            
            // Set the current weather
            WeatherData& currentWeather = const_cast<WeatherData&>(cities.back().getCurrentWeather());
            currentWeather.temperature = temperature;
            currentWeather.humidity = humidity;
            currentWeather.windSpeed = windSpeed;
            currentWeather.precipitation = precipitation;
            currentWeather.condition = static_cast<WeatherCondition>(conditionInt);
            currentWeather.timestamp = timestamp;
            
            // Read forecast
            size_t forecastSize;
            file >> forecastSize;
            file.ignore(numeric_limits<streamsize>::max(), '\n');
            
            // Clear the forecast
            vector<WeatherData>& forecast = const_cast<vector<WeatherData>&>(cities.back().getForecast());
            forecast.clear();
            
            // Read each forecast day
            for (size_t j = 0; j < forecastSize; j++) {
                WeatherData weather;
                
                file >> weather.temperature;
                file >> weather.humidity;
                file >> weather.windSpeed;
                file >> weather.precipitation;
                file >> conditionInt;
                file >> weather.timestamp;
                file.ignore(numeric_limits<streamsize>::max(), '\n');
                
                weather.condition = static_cast<WeatherCondition>(conditionInt);
                forecast.push_back(weather);
            }
        }
        
        file.close();
        return true;
    }
    
    // Search for cities by name
    vector<string> searchCities(const string& query) const {
        vector<string> results;
        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        
        for (const auto& city : cities) {
            string lowerName = city.getName();
            transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            
            if (lowerName.find(lowerQuery) != string::npos) {
                results.push_back(city.getName());
            }
        }
        
        return results;
    }
    
    // Compare weather between two cities
    void compareWeather(const string& city1Name, const string& city2Name) const {
        // Find the cities
        const City* city1 = nullptr;
        const City* city2 = nullptr;
        
        for (const auto& c : cities) {
            if (c.getName() == city1Name) {
                city1 = &c;
            } else if (c.getName() == city2Name) {
                city2 = &c;
            }
        }
        
        if (!city1) {
            cout << "City not found: " << city1Name << endl;
            return;
        }
        
        if (!city2) {
            cout << "City not found: " << city2Name << endl;
            return;
        }
        
        // Get the current weather
        const WeatherData& weather1 = city1->getCurrentWeather();
        const WeatherData& weather2 = city2->getCurrentWeather();
        
        // Display comparison
        cout << "Weather Comparison: " << city1->getName() << " vs " << city2->getName() << endl;
        cout << string(50, '-') << endl;
        
        cout << left << setw(20) << "Metric"
             << right << setw(15) << city1->getName()
             << right << setw(15) << city2->getName()
             << right << setw(15) << "Difference" << endl;
        cout << string(65, '-') << endl;
        
        // Temperature
        double tempDiff = weather1.temperature - weather2.temperature;
        cout << left << setw(20) << "Temperature (°C)"
             << right << setw(15) << fixed << setprecision(1) << weather1.temperature
             << right << setw(15) << fixed << setprecision(1) << weather2.temperature
             << right << setw(15) << fixed << setprecision(1) << tempDiff << endl;
        
        // Humidity
        double humidityDiff = weather1.humidity - weather2.humidity;
        cout << left << setw(20) << "Humidity (%)"
             << right << setw(15) << fixed << setprecision(1) << weather1.humidity
             << right << setw(15) << fixed << setprecision(1) << weather2.humidity
             << right << setw(15) << fixed << setprecision(1) << humidityDiff << endl;
        
        // Wind Speed
        double windDiff = weather1.windSpeed - weather2.windSpeed;
        cout << left << setw(20) << "Wind Speed (km/h)"
             << right << setw(15) << fixed << setprecision(1) << weather1.windSpeed
             << right << setw(15) << fixed << setprecision(1) << weather2.windSpeed
             << right << setw(15) << fixed << setprecision(1) << windDiff << endl;
        
        // Precipitation
        double precipDiff = weather1.precipitation - weather2.precipitation;
        cout << left << setw(20) << "Precipitation (mm)"
             << right << setw(15) << fixed << setprecision(1) << weather1.precipitation
             << right << setw(15) << fixed << setprecision(1) << weather2.precipitation
             << right << setw(15) << fixed << setprecision(1) << precipDiff << endl;
        
        // Condition
        cout << left << setw(20) << "Condition"
             << right << setw(15) << conditionToString(weather1.condition)
             << right << setw(15) << conditionToString(weather2.condition)
             << right << setw(15) << "-" << endl;
    }
};

// Function to display the main menu
void displayMenu() {
    cout << "\n===== Weather App =====" << endl;
    cout << "1. List Available Cities" << endl;
    cout << "2. View Current Weather" << endl;
    cout << "3. View Weather Forecast" << endl;
    cout << "4. Add New City" << endl;
    cout << "5. Remove City" << endl;
    cout << "6. Update Weather Data" << endl;
    cout << "7. Compare Weather Between Cities" << endl;
    cout << "8. Search for City" << endl;
    cout << "9. Save Weather Data" << endl;
    cout << "10. Load Weather Data" << endl;
    cout << "0. Exit" << endl;
    cout << "======================" << endl;
    cout << "Enter your choice: ";
}

// Main function
int main() {
    WeatherApp app;
    int choice;
    string cityName, country, filename;
    double latitude, longitude;
    
    cout << "Welcome to Weather App!" << endl;
    
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
            case 1: // List available cities
                app.listCities();
                break;
                
            case 2: { // View current weather
                cout << "Enter city name: ";
                getline(cin, cityName);
                app.displayCurrentWeather(cityName);
                break;
            }
                
            case 3: { // View weather forecast
                cout << "Enter city name: ";
                getline(cin, cityName);
                app.displayForecast(cityName);
                break;
            }
                
            case 4: { // Add new city
                cout << "Enter city name: ";
                getline(cin, cityName);
                cout << "Enter country: ";
                getline(cin, country);
                cout << "Enter latitude: ";
                cin >> latitude;
                cout << "Enter longitude: ";
                cin >> longitude;
                
                app.addCity(cityName, country, latitude, longitude);
                cout << "City added successfully." << endl;
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 5: { // Remove city
                cout << "Enter city name to remove: ";
                getline(cin, cityName);
                
                if (app.removeCity(cityName)) {
                    cout << "City removed successfully." << endl;
                } else {
                    cout << "City not found." << endl;
                }
                break;
            }
                
            case 6: // Update weather data
                app.updateAllWeather();
                cout << "Weather data updated for all cities." << endl;
                break;
                
            case 7: { // Compare weather between cities
                string city1, city2;
                cout << "Enter first city name: ";
                getline(cin, city1);
                cout << "Enter second city name: ";
                getline(cin, city2);
                
                app.compareWeather(city1, city2);
                break;
            }
                
            case 8: { // Search for city
                string query;
                cout << "Enter search query: ";
                getline(cin, query);
                
                vector<string> results = app.searchCities(query);
                
                if (results.empty()) {
                    cout << "No cities found matching '" << query << "'." << endl;
                } else {
                    cout << "Cities matching '" << query << "':" << endl;
                    for (const auto& city : results) {
                        cout << "- " << city << endl;
                    }
                }
                break;
            }
                
            case 9: { // Save weather data
                cout << "Enter filename to save: ";
                getline(cin, filename);
                
                if (app.saveWeatherData(filename)) {
                    cout << "Weather data saved successfully." << endl;
                } else {
                    cout << "Error saving weather data." << endl;
                }
                break;
            }
                
            case 10: { // Load weather data
                cout << "Enter filename to load: ";
                getline(cin, filename);
                
                if (app.loadWeatherData(filename)) {
                    cout << "Weather data loaded successfully." << endl;
                } else {
                    cout << "Error loading weather data." << endl;
                }
                break;
            }
                
            case 0: // Exit
                cout << "Thank you for using Weather App!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 