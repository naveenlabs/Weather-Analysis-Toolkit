#include "Prediction.h"
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cstdio>

// Increments the period label based on the specified granularity (yearly, monthly, weekly).
static std::string incrementPeriodLabel(const std::string& currentLabel, 
                                        const std::string& granularity)
{
    if (granularity == "yearly") {
        int year = std::stoi(currentLabel);
        year += 1;
        return std::to_string(year);
    }
    else if (granularity == "monthly") {
        int year  = std::stoi(currentLabel.substr(0, 4));
        int month = std::stoi(currentLabel.substr(6, 2));

        month++;
        if (month > 12) {
            month = 1;
            year++;
        }

        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%04d-M%02d", year, month);
        return std::string(buffer);
    }
    else if (granularity == "weekly") {
        int year = std::stoi(currentLabel.substr(0, 4));
        int week = std::stoi(currentLabel.substr(6, 2));

        week++;
        if (week > 53) {
            week = 1;
            year++;
        }

        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%04d-W%02d", year, week);
        return std::string(buffer);
    }

    return "UNKNOWN";
}

// Computes the slope and intercept of a linear regression line for the given data.
static std::pair<double, double> computeLinearRegression(
    const std::vector<int>& x,
    const std::vector<double>& y
)
{
    int n = static_cast<int>(x.size());
    double xMean = 0.0, yMean = 0.0;

    // Calculate means of x and y.
    for (int i = 0; i < n; ++i) {
        xMean += x[i];
        yMean += y[i];
    }
    xMean /= n;
    yMean /= n;

    double numerator = 0.0, denominator = 0.0;
    for (int i = 0; i < n; ++i) {
        double dx = x[i] - xMean;
        double dy = y[i] - yMean;
        numerator   += (dx * dy);
        denominator += (dx * dx);
    }

    double slope = numerator / denominator; 
    double intercept = yMean - slope * xMean; 
    return std::make_pair(slope, intercept);
}

// Generates predictions for future candlesticks based on historical data.
std::vector<Candlestick> Prediction::predict(
    const std::vector<Candlestick>& inputCandlesticks,
    const std::string& granularity,
    int predictionPeriods
)
{
    if (inputCandlesticks.empty()) {
        throw std::invalid_argument("Input candlesticks cannot be empty for prediction.");
    }

    // Prepare data for regression calculations.
    std::vector<int> timeIndices;
    std::vector<double> closes, highs, lows;
    timeIndices.reserve(inputCandlesticks.size());
    closes.reserve(inputCandlesticks.size());
    highs.reserve(inputCandlesticks.size());
    lows.reserve(inputCandlesticks.size());

    for (size_t i = 0; i < inputCandlesticks.size(); ++i) {
        timeIndices.push_back(static_cast<int>(i));
        closes.push_back(inputCandlesticks[i].close);
        highs.push_back(inputCandlesticks[i].high);
        lows.push_back(inputCandlesticks[i].low);
    }

    // Compute linear regression for close, high, and low values.
    auto resultClose = computeLinearRegression(timeIndices, closes);
    auto resultHigh  = computeLinearRegression(timeIndices, highs);
    auto resultLow   = computeLinearRegression(timeIndices, lows);

    double slopeClose = resultClose.first, interceptClose = resultClose.second;
    double slopeHigh = resultHigh.first, interceptHigh = resultHigh.second;
    double slopeLow = resultLow.first, interceptLow = resultLow.second;

    // Generate predictions for the specified number of periods.
    std::string currentLabel = inputCandlesticks.back().year;
    double previousClose = inputCandlesticks.back().close;

    std::vector<Candlestick> predictions;
    predictions.reserve(predictionPeriods);

    for (int i = 0; i < predictionPeriods; ++i) {
        int currentIndex = static_cast<int>(inputCandlesticks.size()) + i;

        double close = slopeClose * currentIndex + interceptClose;
        double high  = slopeHigh * currentIndex + interceptHigh;
        double low   = slopeLow * currentIndex + interceptLow;

        double open = previousClose;

        currentLabel = incrementPeriodLabel(currentLabel, granularity);

        predictions.emplace_back(currentLabel, open, high, low, close);

        previousClose = close;
    }

    return predictions;
}
