#include "Candlestick.h"
#include "CSVReader.h"
#include <fstream>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <map>
#include <stdexcept>
#include <cmath>

// Represents a record of temperature data with a timestamp.
struct TemperatureData {
    std::string timestamp; 
    double temperature;    
};

// Extracts the year key from a timestamp.
static std::string extractYearlyKey(const std::string& timestamp) {
    if (timestamp.size() < 4) return "";
    return timestamp.substr(0, 4);
}

// Extracts the month key from a timestamp.
static std::string extractMonthlyKey(const std::string& timestamp) {
    if (timestamp.size() < 7) return "";
    std::string year  = timestamp.substr(0, 4);
    std::string month = timestamp.substr(5, 2); 
    return year + "-M" + month;
}

// Extracts the week key from a timestamp.
static std::string extractWeeklyKey(const std::string& timestamp) {
    if (timestamp.size() < 10) return "";

    int year  = std::stoi(timestamp.substr(0, 4));
    int month = std::stoi(timestamp.substr(5, 2));
    int day   = std::stoi(timestamp.substr(8, 2));

    static int daysBeforeMonth[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int dayOfYear = daysBeforeMonth[month - 1] + day;

    bool isLeapYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    if (isLeapYear && month > 2) {
        dayOfYear += 1;
    }

    int weekNumber = (dayOfYear - 1) / 7 + 1;
    std::stringstream ss;
    ss << year << "-W";
    if (weekNumber < 10) ss << '0';
    ss << weekNumber;
    return ss.str();
}

// Generates a grouping key based on the timestamp and granularity.
static std::string getGroupingKey(const std::string& timestamp, const std::string& granularity) {
    if (granularity == "monthly") {
        return extractMonthlyKey(timestamp);
    }
    else if (granularity == "weekly") {
        return extractWeeklyKey(timestamp);
    }
    return extractYearlyKey(timestamp);
}

// Computes candlestick data based on input granularity.
std::vector<Candlestick> WeatherDataProcessor::computeCandlestickData(
    const std::string& filename,
    const std::string& countryCode,
    const std::string& granularity
) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string line;
    std::getline(file, line); 
    int countryIndex = -1;

    // Identify the column index for the specified country.
    {
        std::vector<std::string> headerTokens = CSVReader::tokenise(line, ',');
        for (size_t i = 0; i < headerTokens.size(); ++i) {
            if (headerTokens[i] == countryCode + "_temperature") {
                countryIndex = static_cast<int>(i);
                break;
            }
        }
    }
    if (countryIndex == -1) {
        throw std::runtime_error("Country code " + countryCode + " not found in file header.");
    }

    // Parse temperature records from the file.
    std::vector<TemperatureData> records;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = CSVReader::tokenise(line, ',');
        if ((int)tokens.size() <= countryIndex) continue;
        try {
            double tempVal = std::stod(tokens[countryIndex]);
            records.push_back(TemperatureData{ tokens[0], tempVal });
        } catch (...) {
        }
    }

    // Group temperatures by the specified granularity.
    std::map<std::string, std::vector<double>> groupedData;
    for (const auto& rec : records) {
        std::string key = getGroupingKey(rec.timestamp, granularity);
        if (!key.empty()) {
            groupedData[key].push_back(rec.temperature);
        }
    }

    // Compute candlestick data for each group.
    std::vector<Candlestick> candlesticks;
    std::string prevKey;
    double prevClose = 0.0;

    for (auto it = groupedData.begin(); it != groupedData.end(); ++it) {
        const std::string& currentKey = it->first;
        const std::vector<double>& temps = it->second;

        double openVal  = (prevKey.empty()) ? 0.0 : prevClose;
        double closeVal = !temps.empty() ? std::accumulate(temps.begin(), temps.end(), 0.0) / temps.size() : 0.0;
        double highVal = *std::max_element(temps.begin(), temps.end());
        double lowVal  = *std::min_element(temps.begin(), temps.end());

        candlesticks.emplace_back(currentKey, openVal, highVal, lowVal, closeVal);

        prevKey = currentKey;
        prevClose = closeVal;
    }

    return candlesticks;
}

// Computes yearly candlestick data by default.
std::vector<Candlestick> WeatherDataProcessor::computeCandlestickData(
    const std::string& filename,
    const std::string& countryCode
) {
    return computeCandlestickData(filename, countryCode, "yearly");
}

// Retrieves the list of available countries from the file header.
std::vector<std::string> WeatherDataProcessor::getAvailableCountries(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::vector<std::string> countries;
    std::string headerLine;
    if (std::getline(file, headerLine)) {
        std::vector<std::string> tokens = CSVReader::tokenise(headerLine, ',');
        for (auto& token : tokens) {
            size_t pos = token.find("_temperature");
            if (pos != std::string::npos && pos >= 2) {
                std::string cc = token.substr(0, pos);
                countries.push_back(cc);
            }
        }
    }
    return countries;
}
