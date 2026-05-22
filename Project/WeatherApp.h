#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "TextPlotter.h"
#include "Candlestick.h"
#include "CSVReader.h"
#include "FilterData.h"
#include "Prediction.h"

// Main class for managing and running the Weather Application.
class WeatherApp {
public:
    // Constructor to initialize the WeatherApp with a data file and optional plot height.
    WeatherApp(const std::string& dataFile, int plotHeight = 10);

    // Main function to run the WeatherApp.
    void run();

private:
    std::string filename;                            
    TextPlotter plotter;                             
    std::unordered_map<std::string, std::string> countryCodeToName; 
    std::string countryCode;                         
    std::string timeGranularity;                     
    std::vector<Candlestick> currentData;            
    bool isFiltered;                                 

    // Prompts the user to select a country.
    std::string selectCountry();

    // Prompts the user to select a time granularity.
    std::string selectGranularity();

    // Displays the current candlestick data.
    void displayCandlestickData();

    // Plots the current candlestick data.
    void plotCandlestickData();

    // Applies a date range filter to the data.
    bool applyDateRangeFilter();

    // Applies a temperature range filter to the data.
    bool applyTemperatureRangeFilter();

    // Applies both year and temperature range filters to the data.
    bool applyYearAndTemperatureRangeFilter();

    // Applies prediction on the current data.
    bool applyPrediction();

    // Predicts future candlestick data.
    void predictFutureData();

    // Validates a monthly date range.
    bool isValidMonthlyRange(const std::string& start, const std::string& end);

    // Validates a weekly date range.
    bool isValidWeeklyRange(const std::string& start, const std::string& end);

    // Calculates the length of a range based on granularity.
    int calculateRangeLength(const std::string& start, const std::string& end, int granularity);
};
