#pragma once
#include <vector>
#include <string>
#include "Candlestick.h"

// Class for filtering candlestick data based on specific criteria.
class FilterData {
public:
    // Filters candlesticks within a given period range.
    static std::vector<Candlestick> filterByPeriodRange(
        const std::vector<Candlestick>& candlesticks,
        const std::string& startPeriod,
        const std::string& endPeriod
    );

    // Filters candlesticks within a specified temperature range.
    static std::vector<Candlestick> filterByTemperatureRange(
        const std::vector<Candlestick>& candlesticks,
        double minLowTemp,
        double maxHighTemp
    );
};
