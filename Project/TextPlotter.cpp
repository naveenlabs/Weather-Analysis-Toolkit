#include "TextPlotter.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <vector>

// Constructor to initialize the TextPlotter object with a plot height.
TextPlotter::TextPlotter(int) : plotHeight(0) {}

// Plots candlestick data with the default granularity of "yearly".
void TextPlotter::plot(const std::vector<Candlestick>& candlesticks) {
    plot(candlesticks, "yearly");
}

// Plots candlestick data with a specified granularity.
// Handles data processing, temperature range calculation, and text-based plotting.
void TextPlotter::plot(const std::vector<Candlestick>& candlesticks, const std::string& granularity) {
    if (candlesticks.empty()) {
        std::cout << "No data to plot.\n";
        return;
    }

    // Determine the minimum and maximum temperature values from the candlesticks.
    double minTemp = std::min_element(
        candlesticks.begin(), candlesticks.end(),
        [](const Candlestick& a, const Candlestick& b) {
            return a.low < b.low;
        })->low;

    double maxTemp = std::max_element(
        candlesticks.begin(), candlesticks.end(),
        [](const Candlestick& a, const Candlestick& b) {
            return a.high < b.high;
        })->high;

    int minVal = static_cast<int>(std::floor(minTemp));
    int maxVal = static_cast<int>(std::ceil(maxTemp));

    if (minVal == maxVal) {
        std::cout << "All temperatures are the same. Nothing to plot.\n";
        return;
    }

    // Convert candlestick data to integer values for plotting.
    struct IntCandle {
        std::string year;
        int open;
        int high;
        int low;
        int close;
        bool isUp;
    };

    std::vector<IntCandle> intCandles;
    intCandles.reserve(candlesticks.size());

    for (const auto& c : candlesticks) {
        std::string formattedYear = c.year;

        // Format labels for monthly or weekly granularity.
        if (granularity == "monthly" || granularity == "weekly") {
            if (formattedYear.size() >= 4) {
                formattedYear = formattedYear.substr(2);
            }
        }

        intCandles.emplace_back(IntCandle{
            formattedYear,
            static_cast<int>(std::round(c.open)),
            static_cast<int>(std::round(c.high)),
            static_cast<int>(std::round(c.low)),
            static_cast<int>(std::round(c.close)),
            (c.close >= c.open) 
        });
    }

    const std::string wickStr = " ||";
    const std::string upBody = "++++";
    const std::string downBody = "----";

    int columnWidth = 6; 
    if (granularity == "monthly" || granularity == "weekly") {
        columnWidth = 8; 
    }

    const int startPos = 1;
    const int labelWidth = 12; 
    const std::string yAxisLabel = "Temperature";

    int nRows = maxVal - minVal + 1;

    // Prepare Y-axis label characters.
    std::vector<char> yLabelChars(nRows, ' ');
    int labelLength = static_cast<int>(yAxisLabel.size());

    if (labelLength > 0) {
        int startRow = (nRows - labelLength) / 2;
        for (size_t i = 0; i < yAxisLabel.size(); ++i) {
            int pos = startRow + static_cast<int>(i);
            if (pos >= 0 && pos < nRows) {
                yLabelChars[pos] = yAxisLabel[i];
            }
        }
    }

    // Generate the plot row by row, from the maximum temperature to the minimum.
    for (int y = maxVal; y >= minVal; --y) {
        int rowIndex = maxVal - y;
        char yLabelChar = yLabelChars[rowIndex];

        // Print Y-axis label, temperature value, and the separator.
        std::cout << std::setw(labelWidth) << yLabelChar
                  << " " << std::setw(4) << y
                  << " | ";

        for (const auto& ic : intCandles) {
            bool inWick = (y <= ic.high && y >= ic.low);
            bool inBody = (y <= std::max(ic.open, ic.close) &&
                           y >= std::min(ic.open, ic.close));

            std::string cell(columnWidth, ' ');

            // Fill the cell based on whether the position is in the body or wick.
            if (inBody) {
                const std::string& bodyStr = ic.isUp ? upBody : downBody;
                for (size_t i = 0; i < bodyStr.size(); ++i) {
                    int idx = startPos + static_cast<int>(i);
                    if (idx >= 0 && idx < columnWidth) {
                        cell[idx] = bodyStr[i];
                    }
                }
            } else if (inWick) {
                for (size_t i = 0; i < wickStr.size(); ++i) {
                    int idx = startPos + static_cast<int>(i);
                    if (idx >= 0 && idx < columnWidth) {
                        cell[idx] = wickStr[i];
                    }
                }
            }

            std::cout << cell;
        }
        std::cout << "\n";
    }

    // Print X-axis and labels.
    std::cout << std::setw(labelWidth) << ""
              << " " << std::setw(4) << ""
              << " | ";
    for (size_t i = 0; i < intCandles.size(); ++i) {
        std::cout << std::string(columnWidth, '-');
    }
    std::cout << "\n";

    std::cout << std::setw(labelWidth) << ""
              << " " << std::setw(4) << ""
              << "   ";
    for (auto &ic : intCandles) {
        std::string labelToPrint = ic.year;
        int pad = (columnWidth - static_cast<int>(labelToPrint.size())) / 2;
        std::cout << std::setw(pad) << ""
                  << labelToPrint
                  << std::setw(columnWidth - pad - static_cast<int>(labelToPrint.size()))
                  << "";
    }
    std::cout << "\n\n";

    // Print the bottom label indicating the granularity (e.g., Years, Months, Weeks).
    std::string bottomLabel = "Time";
    if (granularity == "yearly") bottomLabel = "Years";
    else if (granularity == "monthly") bottomLabel = "Months";
    else if (granularity == "weekly") bottomLabel = "Weeks";

    int plotWidth = static_cast<int>(intCandles.size()) * columnWidth;
    int labelLen = static_cast<int>(bottomLabel.size());
    int startPosLabel = (plotWidth - labelLen) / 2;

    std::cout << std::setw(labelWidth + 5 + startPosLabel)
              << bottomLabel << "\n";
}
