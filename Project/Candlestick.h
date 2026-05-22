#pragma once
#include <string>
#include <vector>

// Represents a candlestick data structure for time-series data.
class Candlestick {
public:
    std::string year;   
    double open;       
    double high;        
    double low;         
    double close;       

    // Constructor to initialize a Candlestick object.
    Candlestick(std::string year, double open, double high, double low, double close)
        : year(year), open(open), high(high), low(low), close(close) {}
};

// Provides functionality to process weather data and compute candlestick statistics.
class WeatherDataProcessor {
public:
    // Computes candlestick data for a specific country.
    static std::vector<Candlestick> computeCandlestickData(const std::string& filename, 
                                                           const std::string& countryCode);

    // Computes candlestick data for a specific country with specified granularity.
    static std::vector<Candlestick> computeCandlestickData(const std::string& filename, 
                                                           const std::string& countryCode,
                                                           const std::string& granularity);

    // Retrieves the list of available countries from the dataset.
    static std::vector<std::string> getAvailableCountries(const std::string& filename);
};
