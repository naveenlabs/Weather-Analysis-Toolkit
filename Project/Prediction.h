#pragma once
#include "Candlestick.h"
#include <vector>
#include <string>

// Class for generating predictions based on candlestick data.
class Prediction {
public:
    // Predicts future candlestick data based on historical input, granularity, and number of periods.
    static std::vector<Candlestick> predict(
        const std::vector<Candlestick>& inputCandlesticks,
        const std::string& granularity,
        int predictionPeriods
    );
};
