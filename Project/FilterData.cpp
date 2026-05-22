#include "FilterData.h"
#include <algorithm>

// Filters candlesticks within a specific period range.
std::vector<Candlestick> FilterData::filterByPeriodRange(
    const std::vector<Candlestick>& candlesticks,
    const std::string& startPeriod,
    const std::string& endPeriod
) {
    std::vector<Candlestick> filtered;

    // Iterates through candlesticks and adds those within the period range to the result.
    for (const auto& candle : candlesticks) {
        if (candle.year >= startPeriod && candle.year <= endPeriod) {
            filtered.push_back(candle);
        }
    }

    return filtered;
}

// Filters candlesticks based on temperature range constraints.
std::vector<Candlestick> FilterData::filterByTemperatureRange(
    const std::vector<Candlestick>& candlesticks,
    double minLowTemp,
    double maxHighTemp
) {
    std::vector<Candlestick> filtered;

    // Iterates through candlesticks and adds those within the temperature range to the result.
    for (const auto& candle : candlesticks) {
        if (candle.low >= minLowTemp && candle.high <= maxHighTemp) {
            filtered.push_back(candle);
        }
    }

    return filtered;
}
