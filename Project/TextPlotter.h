#pragma once
#include "Candlestick.h"
#include <vector>
#include <string>

// Class for creating text-based visualizations of candlestick data.
class TextPlotter {
public:
    // Constructor to initialize the plotter with a specified plot height.
    TextPlotter(int plotHeight);

    // Plots candlestick data with a specified granularity.
    void plot(const std::vector<Candlestick>& candlesticks, const std::string& granularity);

    // Plots candlestick data without specifying granularity.
    void plot(const std::vector<Candlestick>& candlesticks);

private:
    int plotHeight; // Height of the text-based plot.
};
