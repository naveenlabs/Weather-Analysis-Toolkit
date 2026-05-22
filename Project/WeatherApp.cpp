#include "WeatherApp.h"
#include "Prediction.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <regex>     
#include <cmath>     

// Helper function to find the minimum and maximum labels (years) in the candlestick data.
static std::pair<std::string, std::string> findMinMaxLabels(const std::vector<Candlestick>& data) {
    if (data.empty()) {
        return std::make_pair(std::string(""), std::string(""));
    }
    auto minIt = std::min_element(
        data.begin(), data.end(),
        [](const Candlestick& a, const Candlestick& b) {
            return a.year < b.year;
        }
    );
    auto maxIt = std::max_element(
        data.begin(), data.end(),
        [](const Candlestick& a, const Candlestick& b) {
            return a.year < b.year;
        }
    );

    return std::make_pair(minIt->year, maxIt->year);
}

// Extracts the year as an integer from a given period string.
// Returns -1 if the period format is invalid.
static int extractYearFromPeriod(const std::string& period) {
    if (period.size() < 4) return -1;
    try {
        return std::stoi(period.substr(0, 4));
    } catch (...) {
        return -1;
    }
}

// Constructor to initialize the WeatherApp object with a data file and plot height.
// Initializes country codes to their corresponding country names and sets default time granularity.
WeatherApp::WeatherApp(const std::string& dataFile, int plotHeight)
    : filename(dataFile), plotter(plotHeight), isFiltered(false)
{
    countryCodeToName = {
        {"AT", "Austria"}, {"BE", "Belgium"}, {"BG", "Bulgaria"},
        {"CH", "Switzerland"}, {"CZ", "Czech Republic"}, {"DE", "Germany"},
        {"DK", "Denmark"}, {"EE", "Estonia"}, {"ES", "Spain"},
        {"FI", "Finland"}, {"FR", "France"}, {"GB", "United Kingdom"},
        {"GR", "Greece"}, {"HR", "Croatia"}, {"HU", "Hungary"},
        {"IE", "Ireland"}, {"IT", "Italy"}, {"LT", "Lithuania"},
        {"LU", "Luxembourg"}, {"LV", "Latvia"}, {"NL", "Netherlands"},
        {"NO", "Norway"}, {"PL", "Poland"}, {"PT", "Portugal"},
        {"RO", "Romania"}, {"SE", "Sweden"}, {"SI", "Slovenia"},
        {"SK", "Slovakia"}
    };

    timeGranularity = "yearly";
}

// Main loop of the WeatherApp that displays the menu and handles user choices.
void WeatherApp::run() {
    while (true) {
        // Display the main menu with options.
        std::cout << "\n============================================================\n";
        std::cout << "                     Weather Application                    \n";
        std::cout << "============================================================\n\n";
        std::cout << "Please choose an option below:\n\n";
        std::cout << "1. Display Candlestick Data\n";
        std::cout << "2. Plot Candlestick Data\n";
        std::cout << "3. Apply Filters and Plot Data\n";
        std::cout << "4. Predict Future Data and Plot\n";
        std::cout << "5. Exit Application\n\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "Enter your choice (1-5): ";

        int choice;
        std::cin >> choice;

        // Ignore the remaining input buffer.
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        try {
            if (choice == 1) {
                // Option 1: Display Candlestick Data
                countryCode = selectCountry();
                if (countryCode.empty()) {
                    continue;
                }

                timeGranularity = selectGranularity();
                currentData = WeatherDataProcessor::computeCandlestickData(
                    filename, countryCode, timeGranularity
                );
                isFiltered = false;
                displayCandlestickData();

            } else if (choice == 2) {
                // Option 2: Plot Candlestick Data
                countryCode = selectCountry();
                if (countryCode.empty()) {
                    continue;
                }

                timeGranularity = "yearly";
                currentData = WeatherDataProcessor::computeCandlestickData(
                    filename, countryCode, timeGranularity
                );
                isFiltered = false;
                plotCandlestickData();

            } else if (choice == 3) {
                // Option 3: Apply Filters and Plot Data
                countryCode = selectCountry();
                if (countryCode.empty()) {
                    continue;
                }

                timeGranularity = selectGranularity();

                // Display filter options to the user.
                std::cout << "\n------------------ Filter Options ------------------\n";
                std::cout << "1. Filter by " << timeGranularity << " Range\n";
                std::cout << "2. Filter by Temperature Range\n";
                std::cout << "3. Filter by Both " << timeGranularity << " and Temperature Range\n";
                std::cout << "-----------------------------------------------------\n";
                std::cout << "Enter your choice (1-3): ";

                int filterChoice;
                std::cin >> filterChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                bool filterSuccess = false;

                // Apply the selected filter based on user choice.
                if (filterChoice == 1) {
                    filterSuccess = applyDateRangeFilter();
                }
                else if (filterChoice == 2) {
                    filterSuccess = applyTemperatureRangeFilter();
                }
                else if (filterChoice == 3) {
                    filterSuccess = applyYearAndTemperatureRangeFilter();
                }
                else {
                    std::cerr << "\n[Error] Invalid filter choice. Returning to main menu.\n";
                }

                // If filter was successful, display and plot the filtered data.
                if (filterSuccess) {
                    if (!currentData.empty()) {
                        displayCandlestickData();
                        plotCandlestickData();
                    } else {
                        std::cout << "\n[Info] No data available for the selected filters.\n";
                    }
                }
                else {
                    std::cout << "\n[Info] Filter was not applied.\n";
                }

            } else if (choice == 4) {
                // Option 4: Predict Future Data and Plot
                std::cout << "\n============================================================\n";
                std::cout << "             Future Data Prediction and Plotting             \n";
                std::cout << "============================================================\n";

                std::string predictionCountry = selectCountry();
                if (predictionCountry.empty()) {
                    continue;
                }

                std::string predictionGranularity = selectGranularity();
                if (predictionGranularity.empty()) {
                    continue;
                }

                // Compute candlestick data for prediction.
                std::vector<Candlestick> fullData = WeatherDataProcessor::computeCandlestickData(
                    filename, predictionCountry, predictionGranularity
                );
                if (fullData.empty()) {
                    std::cerr << "\n[Error] No data available for the selected country and granularity.\n";
                    continue;
                }

                std::string startPeriod, endPeriod;
                while (true) {
                    // Prompt user for start and end periods based on granularity.
                    if (predictionGranularity == "yearly") {
                        std::cout << "\nEnter the start year (1980..2019): ";
                        std::cin >> startPeriod;
                        std::cout << "Enter the end year (1980..2019): ";
                        std::cin >> endPeriod;
                    }
                    else if (predictionGranularity == "monthly") {
                        std::cout << "\nEnter the start month (format YYYY-MNN, e.g., 1980-M01): ";
                        std::cin >> startPeriod;
                        std::cout << "Enter the end month (format YYYY-MNN, e.g., 2019-M12): ";
                        std::cin >> endPeriod;
                    }
                    else { 
                        std::cout << "\nEnter the start week (format YYYY-WNN, e.g., 1980-W01): ";
                        std::cin >> startPeriod;
                        std::cout << "Enter the end week (format YYYY-WNN, e.g., 2019-W53): ";
                        std::cin >> endPeriod;
                    }
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    bool formatOK = false;
                    // Validate the input format and range based on granularity.
                    if (predictionGranularity == "yearly") {
                        try {
                            int sYear = std::stoi(startPeriod);
                            int eYear = std::stoi(endPeriod);
                            if ((sYear >= 1980 && sYear <= 2019) &&
                                (eYear >= 1980 && eYear <= 2019) &&
                                sYear <= eYear)
                            {
                                formatOK = true;
                            }
                        } catch (...) {}
                    }
                    else if (predictionGranularity == "monthly") {
                        static std::regex monthlyRegex("^\\d{4}-M(0[1-9]|1[0-2])$");
                        if (std::regex_match(startPeriod, monthlyRegex) &&
                            std::regex_match(endPeriod, monthlyRegex) &&
                            startPeriod <= endPeriod)
                        {
                            auto extractYear = [](const std::string& p) {
                                return std::stoi(p.substr(0, 4));
                            };
                            int sYear = -1, eYear = -1;
                            try {
                                sYear = extractYear(startPeriod);
                                eYear = extractYear(endPeriod);
                            } catch (...) {}

                            if (sYear >= 1980 && sYear <= 2019 &&
                                eYear >= 1980 && eYear <= 2019)
                            {
                                formatOK = true;
                            }
                        }
                    }
                    else { 
                        static std::regex weeklyRegex("^\\d{4}-W(0[1-9]|[1-4]\\d|5[0-3])$");
                        if (std::regex_match(startPeriod, weeklyRegex) &&
                            std::regex_match(endPeriod, weeklyRegex) &&
                            startPeriod <= endPeriod)
                        {
                            auto extractYear = [](const std::string& p) {
                                return std::stoi(p.substr(0, 4));
                            };
                            int sYear = -1, eYear = -1;
                            try {
                                sYear = extractYear(startPeriod);
                                eYear = extractYear(endPeriod);
                            } catch (...) {}

                            if (sYear >= 1980 && sYear <= 2019 &&
                                eYear >= 1980 && eYear <= 2019)
                            {
                                formatOK = true;
                            }
                        }
                    }

                    // If input is invalid, prompt the user again.
                    if (!formatOK) {
                        std::cerr << "\n[Error] Invalid input or out-of-range period.\n"
                                  << "Ensure [1980..2019], correct format, and start <= end.\n";
                        std::cin.clear();
                        continue;
                    }
                    break;
                }

                // Filter the data based on the selected period range.
                std::vector<Candlestick> selectedData =
                    FilterData::filterByPeriodRange(fullData, startPeriod, endPeriod);
                if (selectedData.empty()) {
                    std::cerr << "\n[Error] No data found for the selected start/end period.\n";
                    continue;
                }

                // Determine the granularity code for range length calculation.
                int g = 0;
                if (predictionGranularity == "yearly") {
                    g = 1;
                } else if (predictionGranularity == "monthly") {
                    g = 2;
                } else if (predictionGranularity == "weekly") {
                    g = 3;
                }
                int rangeLen = calculateRangeLength(startPeriod, endPeriod, g);
                int predictionPeriods = static_cast<int>(std::lround(rangeLen * 1.5));

                std::vector<Candlestick> predictedCandles;
                try {
                    // Perform prediction based on the filtered data and desired periods.
                    predictedCandles = Prediction::predict(
                        selectedData, predictionGranularity, predictionPeriods
                    );
                } catch (const std::exception& e) {
                    std::cerr << "\n[Error] During prediction: " << e.what() << "\n";
                    continue;
                }

                // Inform the user about the prediction details.
                std::cout << "\n[Info] Prediction range length: " << rangeLen
                          << " => Predicting " << predictionPeriods << " future "
                          << (predictionGranularity == "yearly"  ? "years" :
                              predictionGranularity == "monthly" ? "months" : "weeks")
                          << ".\n";

                // Display the predicted candlestick data in a tabular format.
                std::cout << "\n================= Predicted Candlestick Data =================\n\n";
                {
                    const int periodWidth = 20;
                    const int openWidth   = 12;
                    const int highWidth   = 12;
                    const int lowWidth    = 12;
                    const int closeWidth  = 12;

                    std::cout << std::left << std::setw(periodWidth) << "Predicted Period"
                              << std::right << std::setw(openWidth)  << "Open"
                              << std::setw(highWidth)               << "High"
                              << std::setw(lowWidth)                << "Low"
                              << std::setw(closeWidth)              << "Close"
                              << "\n";

                    std::cout << std::string(periodWidth + openWidth + highWidth + lowWidth + closeWidth, '-') 
                              << "\n";

                    for (const auto& candle : predictedCandles) {
                        std::cout << std::left << std::setw(periodWidth) << candle.year
                                  << std::right << std::setw(openWidth)   << candle.open
                                  << std::setw(highWidth)                 << candle.high
                                  << std::setw(lowWidth)                  << candle.low
                                  << std::setw(closeWidth)                << candle.close
                                  << "\n";
                    }
                }
                std::cout << "\n============================================================\n";

                std::cout << "\n[Info] Now plotting only the predicted data...\n";

                if (predictedCandles.empty()) {
                    std::cout << "\n[Info] No predicted data to plot.\n";
                }
                else {
                    int totalPoints = static_cast<int>(predictedCandles.size());

                    // Handle large datasets by prompting the user to select a subrange.
                    if ((predictionGranularity == "monthly" || predictionGranularity == "weekly")
                        && totalPoints > 150)
                    {
                        std::cout << "\n[Warning] You have " << totalPoints
                                  << " predicted data points, exceeding 150.\n";
                        std::cout << "All predicted data is displayed above.\n"
                                  << "Please select a subrange to plot.\n";

                        auto mm = findMinMaxLabels(predictedCandles);
                        std::string minLabel = mm.first;
                        std::string maxLabel = mm.second;

                        std::cout << "Valid range: [" << minLabel << " .. " << maxLabel << "]\n";
                        std::vector<Candlestick> subset;

                        while (true) {
                            std::string userStart;
                            std::cout << "\nEnter the start "
                                      << (predictionGranularity == "monthly"
                                          ? "month (e.g. " + minLabel + ")"
                                          : "week (e.g. " + minLabel + ")")
                                      << " within [" << minLabel << " .. " << maxLabel << "]: ";
                            std::cin >> userStart;

                            std::string userEnd;
                            std::cout << "Enter the end "
                                      << (predictionGranularity == "monthly"
                                          ? "month (e.g. " + maxLabel + ")"
                                          : "week (e.g. " + maxLabel + ")")
                                      << " within [" << userStart << " .. " << maxLabel << "]: ";
                            std::cin >> userEnd;
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                            bool valid = false;
                            // Validate the user-selected subrange.
                            if (predictionGranularity == "monthly") {
                                static std::regex monthlyRegex("^\\d{4}-M(0[1-9]|1[0-2])$");
                                if (std::regex_match(userStart, monthlyRegex) &&
                                    std::regex_match(userEnd, monthlyRegex) &&
                                    userStart <= userEnd)
                                {
                                    if (userStart >= minLabel && userEnd <= maxLabel) {
                                        valid = true;
                                    }
                                    else {
                                        std::cerr << "\n[Error] Start and end periods must be within the valid range ["
                                                  << minLabel << " .. " << maxLabel << "].\n";
                                    }
                                }
                            }
                            else { 
                                static std::regex weeklyRegex("^\\d{4}-W(0[1-9]|[1-4]\\d|5[0-3])$");
                                if (std::regex_match(userStart, weeklyRegex) &&
                                    std::regex_match(userEnd, weeklyRegex) &&
                                    userStart <= userEnd)
                                {
                                    if (userStart >= minLabel && userEnd <= maxLabel) {
                                        valid = true;
                                    }
                                    else {
                                        std::cerr << "\n[Error] Start and end periods must be within the valid range ["
                                                  << minLabel << " .. " << maxLabel << "].\n";
                                    }
                                }
                            }

                            // If the subrange is invalid, prompt the user again.
                            if (!valid) {
                                std::cerr << "\n[Error] Invalid input or out-of-range period.\n"
                                          << "Ensure both start and end are within ["
                                          << minLabel << " .. " << maxLabel << "] and correctly formatted.\n";
                                std::cin.clear();
                                continue;
                            }

                            // Filter the predicted data based on the user-selected subrange.
                            subset = FilterData::filterByPeriodRange(predictedCandles, userStart, userEnd);

                            if (subset.empty()) {
                                std::cerr << "\n[Error] No data in that subrange. Please try again.\n";
                                continue;
                            }
                            if ((int)subset.size() > 150) {
                                std::cerr << "\n[Error] Selected subrange has " << subset.size()
                                          << " points, exceeding 150.\n";
                                continue;
                            }
                            break;
                        }

                        int subSize = static_cast<int>(subset.size());
                        std::cout << "\n[Info] Plotting " << subSize << " predicted data points ["
                                  << subset.front().year << " .. " << subset.back().year << "]...\n";
                        int indexStart = 0;
                        int chunkNumber = 1;
                        // Plot the subset in chunks to manage large datasets.
                        while (indexStart < subSize && chunkNumber <= 5) {
                            int chunkSize = std::min(30, subSize - indexStart);
                            std::vector<Candlestick> chunk(
                                subset.begin() + indexStart,
                                subset.begin() + indexStart + chunkSize
                            );

                            std::cout << "\n--- Plot " << chunkNumber << ": " << chunkSize << " Data Points ---\n";
                            TextPlotter predictionPlotter(10);
                            predictionPlotter.plot(chunk, predictionGranularity);

                            indexStart += chunkSize;
                            chunkNumber++;
                        }
                    }
                    else if (predictionGranularity == "yearly" && totalPoints > 150) {
                        // Handle excessive data points for yearly granularity.
                        std::cout << "\n[Error] " << totalPoints
                                  << " predicted data points exceed the 150 limit.\n";
                    }
                    else {
                        // Plot the predicted data in manageable chunks.
                        int indexStart = 0;
                        int chunkNumber = 1;
                        while (indexStart < totalPoints && chunkNumber <= 5) {
                            int chunkSize = std::min(30, totalPoints - indexStart);
                            std::vector<Candlestick> chunk(
                                predictedCandles.begin() + indexStart,
                                predictedCandles.begin() + indexStart + chunkSize
                            );

                            std::cout << "\n--- Plot " << chunkNumber << ": " << chunkSize << " Data Points ---\n";
                            TextPlotter predictionPlotter(10);
                            predictionPlotter.plot(chunk, predictionGranularity);

                            indexStart += chunkSize;
                            chunkNumber++;
                        }
                    }
                }

            } 
            else if (choice == 5) {
                // Option 5: Exit Application
                std::cout << "\n============================================================\n";
                std::cout << "                     Exiting Application                   \n";
                std::cout << "============================================================\n";
                std::cout << "Thank you for using the Weather Application. Goodbye!\n";
                break;

            } else {
                // Handle invalid menu choices.
                std::cerr << "\n[Error] Invalid choice. Please select a number between 1 and 5.\n";
            }
        } catch (const std::exception& e) {
            // Catch and display any exceptions that occur during execution.
            std::cerr << "\n[Exception] " << e.what() << "\n";
        }
    }
}

// Prompts the user to select a country from the available list.
// Returns the selected country's code or an empty string if canceled.
std::string WeatherApp::selectCountry() {
    std::vector<std::string> countries = WeatherDataProcessor::getAvailableCountries(filename);
    if (countries.empty()) {
        std::cerr << "\n[Error] No countries found in the data file.\n";
        return "";
    }

    // Display the list of available countries with their codes and names.
    std::cout << "\n------------------ Available Countries ------------------\n";
    for (size_t i = 0; i < countries.size(); ++i) {
        std::string code = countries[i];
        std::string name = "Unknown";
        auto it = countryCodeToName.find(code);
        if (it != countryCodeToName.end()) {
            name = it->second;
        }
        std::cout << std::setw(3) << (i + 1) << ". " << code << " (" << name << ")\n";
    }
    std::cout << "------------------------------------------------------------\n";

    while (true) {
        // Prompt the user to select a country by number.
        std::cout << "Enter the number of the country you want to select (or 0 to cancel): ";
        int selection;
        std::cin >> selection;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!std::cin.fail()) {
            if (selection == 0) {
                // User chose to cancel the selection.
                std::cout << "\n[Info] Country selection canceled.\n";
                return "";
            }
            if (selection >= 1 && static_cast<size_t>(selection) <= countries.size()) {
                // Valid selection; return the corresponding country code.
                std::string selectedCode = countries[selection - 1];
                std::cout << "\n[Selected] " << selectedCode;
                auto it = countryCodeToName.find(selectedCode);
                if (it != countryCodeToName.end()) {
                    std::cout << " (" << it->second << ")";
                }
                std::cout << "\n";
                return selectedCode;
            }
        }

        // Handle invalid selections.
        std::cerr << "\n[Error] Invalid country selection. Please try again.\n";
        std::cin.clear();
    }
}

// Prompts the user to select the time granularity for data analysis.
// Returns the selected granularity as a string.
std::string WeatherApp::selectGranularity() {
    while (true) {
        // Display granularity options to the user.
        std::cout << "\n================= Select Time Granularity =================\n";
        std::cout << "1. Yearly (e.g., 1980)\n";
        std::cout << "2. Monthly (e.g., 1980-M01)\n";
        std::cout << "3. Weekly (e.g., 1980-W01)\n";
        std::cout << "============================================================\n";
        std::cout << "Enter your choice (1-3): ";

        int gChoice;
        std::cin >> gChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!std::cin.fail()) {
            // Return the corresponding granularity based on user choice.
            if (gChoice == 1) {
                return "yearly";
            } else if (gChoice == 2) {
                return "monthly";
            } else if (gChoice == 3) {
                return "weekly";
            }
        }

        // Handle invalid granularity choices.
        std::cerr << "\n[Error] Invalid choice. Please select a number between 1 and 3.\n";
        std::cin.clear();
    }
}

// Displays the current candlestick data in a tabular format.
void WeatherApp::displayCandlestickData() {
    if (currentData.empty()) {
        std::cout << "\n[Info] No candlestick data available to display.\n";
        return;
    }

    // Retrieve the country name based on the country code.
    std::string countryName = "Unknown";
    auto itC = countryCodeToName.find(countryCode);
    if (itC != countryCodeToName.end()) {
        countryName = itC->second;
    }

    // Display header information.
    std::cout << "\n============================================================\n";
    std::cout << "                Candlestick Data Display                    \n";
    std::cout << "============================================================\n";
    std::cout << "Country: " << countryCode;
    if (countryName != "Unknown") {
        std::cout << " (" << countryName << ")";
    }
    std::cout << "\nTime Granularity: " << timeGranularity << "\n\n";

    // Define column widths for the table.
    const int periodWidth = 20;
    const int openWidth   = 12;
    const int highWidth   = 12;
    const int lowWidth    = 12;
    const int closeWidth  = 12;

    // Print table headers.
    std::cout << std::left << std::setw(periodWidth)
              << ((timeGranularity == "yearly") ? "Year" : "Period")
              << std::right << std::setw(openWidth)  << "Open"
              << std::setw(highWidth)               << "High"
              << std::setw(lowWidth)                << "Low"
              << std::setw(closeWidth)              << "Close"
              << "\n";

    std::cout << std::string(periodWidth + openWidth + highWidth + lowWidth + closeWidth, '-')
              << "\n";

    // Iterate through the candlestick data and display each entry.
    for (size_t i = 0; i < currentData.size(); ++i) {
        const Candlestick& candle = currentData[i];
        std::cout << std::left << std::setw(periodWidth) << candle.year
                  << std::right << std::setw(openWidth)   << candle.open
                  << std::setw(highWidth)                 << candle.high
                  << std::setw(lowWidth)                  << candle.low
                  << std::setw(closeWidth)                << candle.close
                  << "\n";

        // Add extra spacing for monthly or weekly granularity at the end of each year/month/week.
        if (!isFiltered) {
            if (timeGranularity == "monthly") {
                if (candle.year.rfind("-M12") != std::string::npos) {
                    std::cout << "\n";
                }
            }
            else if (timeGranularity == "weekly") {
                if (candle.year.rfind("-W53") != std::string::npos) {
                    std::cout << "\n";
                }
            }
        }
    }
    std::cout << "\n============================================================\n";
}

// Plots the current candlestick data using the TextPlotter class.
void WeatherApp::plotCandlestickData() {
    if (currentData.empty()) {
        std::cout << "\n[Info] No candlestick data available to plot.\n";
        return;
    }

    int totalPoints = static_cast<int>(currentData.size());

    // Handle scenarios where data points exceed the maximum allowed for plotting.
    if ((timeGranularity == "monthly" || timeGranularity == "weekly") && totalPoints > 150) {
        std::cout << "\n[Warning] You have " << totalPoints
                  << " data points for " << timeGranularity
                  << ", which exceeds the maximum (150) allowed for a single plot.\n";
        std::cout << "All data has been displayed above. Please select a subrange to plot.\n";

        // Find the minimum and maximum labels for valid range selection.
        std::pair<std::string, std::string> minMax = findMinMaxLabels(currentData);
        std::string minLabel = minMax.first;
        std::string maxLabel = minMax.second;

        std::cout << "Valid range: [" << minLabel << " .. " << maxLabel << "]\n";
        std::vector<Candlestick> subset;

        while (true) {
            std::string startPeriod;
            std::cout << "\nEnter the start "
                      << (timeGranularity == "monthly" ? "month (e.g., 1980-M01)" : "week (e.g., 1980-W01)")
                      << " within [" << minLabel << " .. " << maxLabel << "]: ";
            std::cin >> startPeriod;

            std::string endPeriod;
            std::cout << "Enter the end "
                      << (timeGranularity == "monthly" ? "month (e.g., 1980-M12)" : "week (e.g., 1980-W53)")
                      << " within [" << startPeriod << " .. " << maxLabel << "]: ";
            std::cin >> endPeriod;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            bool valid = false;
            // Validate the user-selected subrange based on granularity.
            if (timeGranularity == "monthly") {
                static std::regex monthlyRegex("^\\d{4}-M(0[1-9]|1[0-2])$");
                if (std::regex_match(startPeriod, monthlyRegex) &&
                    std::regex_match(endPeriod, monthlyRegex) &&
                    startPeriod <= endPeriod)
                {
                    if (startPeriod >= minLabel && endPeriod <= maxLabel) {
                        valid = true;
                    }
                    else {
                        std::cerr << "\n[Error] Start and end periods must be within the valid range ["
                                  << minLabel << " .. " << maxLabel << "].\n";
                    }
                }
            }
            else { 
                static std::regex weeklyRegex("^\\d{4}-W(0[1-9]|[1-4]\\d|5[0-3])$");
                if (std::regex_match(startPeriod, weeklyRegex) &&
                    std::regex_match(endPeriod, weeklyRegex) &&
                    startPeriod <= endPeriod)
                {
                    if (startPeriod >= minLabel && endPeriod <= maxLabel) {
                        valid = true;
                    }
                    else {
                        std::cerr << "\n[Error] Start and end periods must be within the valid range ["
                                  << minLabel << " .. " << maxLabel << "].\n";
                    }
                }
            }

            // If the subrange is invalid, prompt the user again.
            if (!valid) {
                std::cerr << "\n[Error] Invalid input or out-of-range period.\n"
                          << "Ensure both start and end are within ["
                          << minLabel << " .. " << maxLabel << "] and correctly formatted.\n";
                std::cin.clear();
                continue;
            }

            // Filter the current data based on the selected subrange.
            subset = FilterData::filterByPeriodRange(currentData, startPeriod, endPeriod);

            if (subset.empty()) {
                std::cerr << "\n[Error] No data in that subrange. Please try again.\n";
                continue;
            }

            if ((int)subset.size() > 150) {
                std::cerr << "\n[Error] Selected subrange has " << subset.size()
                          << " data points, exceeding 150.\n";
                continue;
            }
            break;
        }

        int subSize = static_cast<int>(subset.size());
        std::cout << "\n[Info] Plotting " << subSize << " data points ["
                  << subset.front().year << " .. " << subset.back().year << "]...\n";
        int indexStart = 0;
        int chunkNumber = 1;
        // Plot the subset in chunks to manage large datasets.
        while (indexStart < subSize && chunkNumber <= 5) {
            int chunkSize = std::min(30, subSize - indexStart);
            std::vector<Candlestick> chunk(
                subset.begin() + indexStart,
                subset.begin() + indexStart + chunkSize
            );

            std::cout << "\n--- Plot " << chunkNumber << ": " << chunkSize << " Data Points ---\n";
            plotter.plot(chunk, timeGranularity);

            indexStart += chunkSize;
            chunkNumber++;
        }
        return;
    }

    if (timeGranularity == "yearly" && totalPoints > 150) {
        // Handle excessive data points for yearly granularity.
        std::cout << "\n[Error] You have " << totalPoints
                  << " data points, which exceeds the 150 limit for plotting.\n"
                  << "Please refine your filters or choose a smaller range.\n";
        return;
    }

    // Retrieve the country name based on the country code.
    std::string countryName = "Unknown";
    auto it = countryCodeToName.find(countryCode);
    if (it != countryCodeToName.end()) {
        countryName = it->second;
    }

    // Display header information for plotting.
    std::cout << "\n============================================================\n";
    std::cout << "                  Candlestick Data Plotting                  \n";
    std::cout << "============================================================\n";
    std::cout << "Country: " << countryCode;
    if (countryName != "Unknown") {
        std::cout << " (" << countryName << ")";
    }
    std::cout << "\nTime Granularity: " << timeGranularity << "\n\n";

    int indexStart = 0;
    int chunkNumber = 1;
    // Plot the data in chunks to manage large datasets.
    while (indexStart < totalPoints && chunkNumber <= 5) {
        int chunkSize = std::min(30, totalPoints - indexStart);
        std::vector<Candlestick> chunk(
            currentData.begin() + indexStart,
            currentData.begin() + indexStart + chunkSize
        );

        std::cout << "\n--- Plot " << chunkNumber << ": " << chunkSize << " Data Points ---\n";
        plotter.plot(chunk, timeGranularity);

        indexStart += chunkSize;
        chunkNumber++;
    }
}

// Applies a date range filter to the current data based on user input.
// Returns true if the filter was successfully applied, false otherwise.
bool WeatherApp::applyDateRangeFilter() {
    // Compute the full candlestick data without any filters.
    std::vector<Candlestick> fullData =
        WeatherDataProcessor::computeCandlestickData(filename, countryCode, timeGranularity);

    if (fullData.empty()) {
        std::cerr << "\n[Error] No data available for the selected country.\n";
        return false;
    }

    std::string startPeriod, endPeriod;
    while (true) {
        // Prompt the user to enter start and end periods based on granularity.
        if (timeGranularity == "yearly") {
            std::cout << "\nEnter the start year (e.g., 1980..2019): ";
        }
        else if (timeGranularity == "monthly") {
            std::cout << "\nEnter the start month in format YYYY-MNN (e.g., 1980-M01): ";
        }
        else if (timeGranularity == "weekly") {
            std::cout << "\nEnter the start week in format YYYY-WNN (e.g., 1980-W01): ";
        }
        std::cin >> startPeriod;

        if (timeGranularity == "yearly") {
            std::cout << "Enter the end year (e.g., 1980..2019): ";
        }
        else if (timeGranularity == "monthly") {
            std::cout << "Enter the end month in format YYYY-MNN (e.g., 1980-M12): ";
        }
        else if (timeGranularity == "weekly") {
            std::cout << "Enter the end week in format YYYY-WNN (e.g., 1980-W53): ";
        }
        std::cin >> endPeriod;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        bool formatOK = false;
        // Validate the input format and range based on granularity.
        if (timeGranularity == "yearly") {
            try {
                int sYear = std::stoi(startPeriod);
                int eYear = std::stoi(endPeriod);

                if ((sYear >= 1980 && sYear <= 2019) &&
                    (eYear >= 1980 && eYear <= 2019) &&
                    sYear <= eYear)
                {
                    formatOK = true;
                }
            } catch (...) {}
        }
        else if (timeGranularity == "monthly") {
            static std::regex monthlyRegex("^\\d{4}-M(0[1-9]|1[0-2])$");
            if (std::regex_match(startPeriod, monthlyRegex) &&
                std::regex_match(endPeriod, monthlyRegex) &&
                startPeriod <= endPeriod)
            {
                auto extractYear = [](const std::string& p) {
                    return std::stoi(p.substr(0, 4));
                };
                int sYear = -1, eYear = -1;
                try {
                    sYear = extractYear(startPeriod);
                    eYear = extractYear(endPeriod);
                } catch (...) {}

                if (sYear >= 1980 && sYear <= 2019 &&
                    eYear >= 1980 && eYear <= 2019)
                {
                    formatOK = true;
                }
            }
        }
        else {
            static std::regex weeklyRegex("^\\d{4}-W(0[1-9]|[1-4]\\d|5[0-3])$");
            if (std::regex_match(startPeriod, weeklyRegex) &&
                std::regex_match(endPeriod, weeklyRegex) &&
                startPeriod <= endPeriod)
            {
                auto extractYear = [](const std::string& p) {
                    return std::stoi(p.substr(0, 4));
                };
                int sYear = -1, eYear = -1;
                try {
                    sYear = extractYear(startPeriod);
                    eYear = extractYear(endPeriod);
                } catch (...) {}

                if (sYear >= 1980 && sYear <= 2019 &&
                    eYear >= 1980 && eYear <= 2019)
                {
                    formatOK = true;
                }
            }
        }

        // If input is invalid, prompt the user again.
        if (!formatOK) {
            std::cerr << "\n[Error] Invalid input or out-of-range period.\n"
                      << "Please ensure both start and end periods are within [1980..2019] and correctly formatted.\n";
            std::cin.clear();
            continue;
        }

        break;
    }

    // Apply the date range filter to the full data.
    currentData = FilterData::filterByPeriodRange(fullData, startPeriod, endPeriod);
    isFiltered = true;

    if (currentData.empty()) {
        std::cout << "\n[Info] No data available for the selected period range.\n";
        return false;
    }

    // Inform the user that the filter was successfully applied.
    std::cout << "\n[Success] Filter applied: " << timeGranularity << " range "
              << startPeriod << " to " << endPeriod << "\n";

    return true;
}

// Applies a temperature range filter to the current data based on user input.
// Returns true if the filter was successfully applied, false otherwise.
bool WeatherApp::applyTemperatureRangeFilter() {
    // Compute the full candlestick data without any filters.
    std::vector<Candlestick> fullData =
        WeatherDataProcessor::computeCandlestickData(filename, countryCode, timeGranularity);

    if (fullData.empty()) {
        std::cerr << "\n[Error] No data available for the selected country.\n";
        return false;
    }

    while (true) {
        // Display temperature filter options to the user.
        std::cout << "\n================= Temperature Filter Options =================\n";
        std::cout << "1. Sort by Low Temperature\n";
        std::cout << "2. Sort by High Temperature\n";
        std::cout << "3. Filter by Minimum Low and Maximum High Temperatures\n";
        std::cout << "================================================================\n";
        std::cout << "Enter your choice (1-3): ";
        int tempFilterChoice;
        std::cin >> tempFilterChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (std::cin.fail()) {
            std::cin.clear();
            std::cerr << "\n[Error] Invalid input. Please try again.\n";
            continue;
        }

        if (tempFilterChoice == 1) {
            // Option 1: Sort data by low temperature in ascending order.
            currentData = fullData;
            std::sort(currentData.begin(), currentData.end(),
                      [](const Candlestick& a, const Candlestick& b) {
                          return a.low < b.low;
                      });
            isFiltered = true;
            std::cout << "\n[Success] Data sorted by Low temperature.\n";
            return true;

        } else if (tempFilterChoice == 2) {
            // Option 2: Sort data by high temperature in descending order.
            currentData = fullData;
            std::sort(currentData.begin(), currentData.end(),
                      [](const Candlestick& a, const Candlestick& b) {
                          return a.high > b.high;
                      });
            isFiltered = true;
            std::cout << "\n[Success] Data sorted by High temperature.\n";
            return true;

        } else if (tempFilterChoice == 3) {
            // Option 3: Filter data based on a minimum low and maximum high temperature.
            double minLowTemp = fullData[0].low;
            double maxHighTemp = fullData[0].high;

            // Determine the available temperature range.
            for (size_t i = 0; i < fullData.size(); ++i) {
                if (fullData[i].low < minLowTemp)   minLowTemp = fullData[i].low;
                if (fullData[i].high > maxHighTemp) maxHighTemp = fullData[i].high;
            }

            std::cout << "\n[Info] Available Low Temperature Range: " << minLowTemp << "\n";
            std::cout << "[Info] Available High Temperature Range: " << maxHighTemp << "\n";

            while (true) {
                double userMinLowTemp, userMaxHighTemp;
                // Prompt the user to enter minimum low and maximum high temperatures.
                std::cout << "\nEnter the Minimum Low Temperature: ";
                std::cin >> userMinLowTemp;
                std::cout << "Enter the Maximum High Temperature: ";
                std::cin >> userMaxHighTemp;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                // Validate the temperature inputs.
                if (std::cin.fail()) {
                    std::cin.clear();
                    std::cerr << "\n[Error] Invalid numeric input. Please try again.\n";
                    continue;
                }
                if (userMinLowTemp > userMaxHighTemp) {
                    std::cerr << "\n[Error] Minimum temperature cannot be greater than maximum temperature.\n";
                    continue;
                }
                if (userMinLowTemp < minLowTemp || userMaxHighTemp > maxHighTemp) {
                    std::cerr << "\n[Error] Temperature range exceeds available data range.\n"
                              << "Please ensure Minimum Low >= " << minLowTemp
                              << " and Maximum High <= " << maxHighTemp << ".\n";
                    continue;
                }

                // Apply the temperature range filter to the full data.
                currentData = FilterData::filterByTemperatureRange(
                    fullData, userMinLowTemp, userMaxHighTemp
                );
                isFiltered = true;

                if (currentData.empty()) {
                    std::cout << "\n[Info] No data available after applying temperature filters.\n";
                    return false;
                }

                // Inform the user that the temperature filter was successfully applied.
                std::cout << "\n[Success] Filter applied: Low >= " << userMinLowTemp
                          << ", High <= " << userMaxHighTemp << "\n";

                // Prompt the user to sort the filtered data.
                std::cout << "\nWould you like to sort the filtered data?\n";
                std::cout << "1. Sort by Low Temperature\n";
                std::cout << "2. Sort by High Temperature\n";
                std::cout << "3. No Sorting\n";
                std::cout << "Enter your choice (1-3): ";
                int postFilterSortChoice;
                std::cin >> postFilterSortChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (!std::cin.fail()) {
                    if (postFilterSortChoice == 1) {
                        // Sort by low temperature in ascending order.
                        std::sort(currentData.begin(), currentData.end(),
                                  [](const Candlestick& a, const Candlestick& b) {
                                      return a.low < b.low;
                                  });
                        std::cout << "\n[Success] Data sorted by Low temperature.\n";
                    }
                    else if (postFilterSortChoice == 2) {
                        // Sort by high temperature in descending order.
                        std::sort(currentData.begin(), currentData.end(),
                                  [](const Candlestick& a, const Candlestick& b) {
                                      return a.high > b.high;
                                  });
                        std::cout << "\n[Success] Data sorted by High temperature.\n";
                    }
                    else {
                        // No additional sorting applied.
                        std::cout << "\n[Info] Data will be plotted without additional sorting.\n";
                    }
                }

                return true;
            }
        } else {
            // Handle invalid temperature filter choices.
            std::cerr << "\n[Error] Invalid temperature filter choice. Please select 1, 2, or 3.\n";
        }
    }
}

// Applies both date range and temperature range filters to the current data based on user input.
// Returns true if the filters were successfully applied, false otherwise.
bool WeatherApp::applyYearAndTemperatureRangeFilter() {
    // Compute the full candlestick data without any filters.
    std::vector<Candlestick> fullData =
        WeatherDataProcessor::computeCandlestickData(filename, countryCode, timeGranularity);
    if (fullData.empty()) {
        std::cerr << "\n[Error] No data available.\n";
        return false;
    }

    std::string startPeriod, endPeriod;
    while (true) {
        // Prompt the user to enter start and end periods based on granularity.
        if (timeGranularity == "yearly") {
            std::cout << "\nEnter the start year (1980..2019): ";
        }
        else if (timeGranularity == "monthly") {
            std::cout << "\nEnter the start month (format YYYY-MNN, e.g., 1980-M01): ";
        }
        else if (timeGranularity == "weekly") {
            std::cout << "\nEnter the start week (format YYYY-WNN, e.g., 1980-W01): ";
        }
        std::cin >> startPeriod;

        if (timeGranularity == "yearly") {
            std::cout << "Enter the end year (1980..2019): ";
        }
        else if (timeGranularity == "monthly") {
            std::cout << "Enter the end month (format YYYY-MNN, e.g., 2019-M12): ";
        }
        else if (timeGranularity == "weekly") {
            std::cout << "Enter the end week (format YYYY-WNN, e.g., 2019-W53): ";
        }
        std::cin >> endPeriod;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        bool formatOK = false;
        // Validate the input format and range based on granularity.
        if (timeGranularity == "yearly") {
            try {
                int sYear = std::stoi(startPeriod);
                int eYear = std::stoi(endPeriod);
                if ((sYear >= 1980 && sYear <= 2019) &&
                    (eYear >= 1980 && eYear <= 2019) &&
                     sYear <= eYear)
                {
                    formatOK = true;
                }
            } catch (...) {}
        }
        else if (timeGranularity == "monthly") {
            static std::regex monthlyRegex("^\\d{4}-M(0[1-9]|1[0-2])$");
            if (std::regex_match(startPeriod, monthlyRegex) &&
                std::regex_match(endPeriod, monthlyRegex) &&
                startPeriod <= endPeriod)
            {
                auto extractYear = [](const std::string& p) {
                    return std::stoi(p.substr(0, 4));
                };
                int sYear = -1, eYear = -1;
                try {
                    sYear = extractYear(startPeriod);
                    eYear = extractYear(endPeriod);
                } catch (...) {}

                if (sYear >= 1980 && sYear <= 2019 &&
                    eYear >= 1980 && eYear <= 2019)
                {
                    formatOK = true;
                }
            }
        }
        else { 
            static std::regex weeklyRegex("^\\d{4}-W(0[1-9]|[1-4]\\d|5[0-3])$");
            if (std::regex_match(startPeriod, weeklyRegex) &&
                std::regex_match(endPeriod, weeklyRegex) &&
                startPeriod <= endPeriod)
            {
                auto extractYear = [](const std::string& p) {
                    return std::stoi(p.substr(0, 4));
                };
                int sYear = -1, eYear = -1;
                try {
                    sYear = extractYear(startPeriod);
                    eYear = extractYear(endPeriod);
                } catch (...) {}

                if (sYear >= 1980 && sYear <= 2019 &&
                    eYear >= 1980 && eYear <= 2019)
                {
                    formatOK = true;
                }
            }
        }

        // If input is invalid, prompt the user again.
        if (!formatOK) {
            std::cerr << "\n[Error] Invalid period input or out-of-range year.\n"
                      << "Please ensure both start and end periods are within [1980..2019] and correctly formatted.\n";
            std::cin.clear();
            continue;
        }

        break;  
    }

    // Apply the date range filter to the full data.
    std::vector<Candlestick> periodFiltered =
        FilterData::filterByPeriodRange(fullData, startPeriod, endPeriod);
    if (periodFiltered.empty()) {
        std::cerr << "\n[Error] No data available for the selected period range.\n";
        return false;
    }

    // Determine the available temperature range within the filtered period.
    double minLowTemp = periodFiltered[0].low;
    double maxHighTemp = periodFiltered[0].high;
    for (size_t i = 0; i < periodFiltered.size(); ++i) {
        if (periodFiltered[i].low < minLowTemp)   minLowTemp = periodFiltered[i].low;
        if (periodFiltered[i].high > maxHighTemp) maxHighTemp = periodFiltered[i].high;
    }

    std::cout << "\n[Info] Available Low Temperature in this period: " << minLowTemp << "\n";
    std::cout << "[Info] Available High Temperature in this period: " << maxHighTemp << "\n";

    while (true) {
        double userMinLowTemp, userMaxHighTemp;
        // Prompt the user to enter minimum low and maximum high temperatures.
        std::cout << "\nEnter the Minimum Low Temperature: ";
        std::cin >> userMinLowTemp;
        std::cout << "Enter the Maximum High Temperature: ";
        std::cin >> userMaxHighTemp;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Validate the temperature inputs.
        if (std::cin.fail()) {
            std::cin.clear();
            std::cerr << "\n[Error] Invalid numeric input. Please try again.\n";
            continue;
        }
        if (userMinLowTemp > userMaxHighTemp) {
            std::cerr << "\n[Error] Minimum temperature cannot be greater than maximum temperature.\n";
            continue;
        }
        if (userMinLowTemp < minLowTemp || userMaxHighTemp > maxHighTemp) {
            std::cerr << "\n[Error] Temperature range exceeds available data range.\n"
                      << "Please ensure Minimum Low >= " << minLowTemp
                      << " and Maximum High <= " << maxHighTemp << ".\n";
            continue;
        }

        // Apply the temperature range filter to the period-filtered data.
        std::vector<Candlestick> finalFiltered =
            FilterData::filterByTemperatureRange(periodFiltered, userMinLowTemp, userMaxHighTemp);
        if (finalFiltered.empty()) {
            std::cerr << "\n[Error] No data available after applying temperature filters.\n";
            return false;
        }

        currentData = finalFiltered;
        isFiltered = true;

        // Inform the user that the filters were successfully applied.
        std::cout << "\n[Success] Filters applied: " << timeGranularity << " range "
                  << startPeriod << " to " << endPeriod
                  << " and Low >= " << userMinLowTemp
                  << ", High <= " << userMaxHighTemp << "\n";

        // Prompt the user to sort the filtered data.
        std::cout << "\nWould you like to sort the filtered data?\n";
        std::cout << "1. Sort by Highest Temperature\n";
        std::cout << "2. Sort by Lowest Temperature\n";
        std::cout << "3. No Sorting\n";
        std::cout << "Enter your choice (1-3): ";
        int sortChoice;
        std::cin >> sortChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!std::cin.fail()) {
            if (sortChoice == 1) {
                // Sort by highest temperature in descending order.
                std::sort(currentData.begin(), currentData.end(),
                          [](const Candlestick& a, const Candlestick& b) {
                              return a.high > b.high;
                          });
                std::cout << "\n[Success] Data sorted by Highest Temperature.\n";
            }
            else if (sortChoice == 2) {
                // Sort by lowest temperature in ascending order.
                std::sort(currentData.begin(), currentData.end(),
                          [](const Candlestick& a, const Candlestick& b) {
                              return a.low < b.low;
                          });
                std::cout << "\n[Success] Data sorted by Lowest Temperature.\n";
            }
            else {
                // No additional sorting applied.
                std::cout << "\n[Info] Data will be plotted without additional sorting.\n";
            }
        }

        return true;
    }
}

// Validates if the provided monthly range is valid based on the start and end periods.
// Returns true if valid, false otherwise.
bool WeatherApp::isValidMonthlyRange(const std::string& start, const std::string& end) {
    return start < end && start.substr(0, 4) >= "1980" && end.substr(0, 4) <= "2019";
}

// Validates if the provided weekly range is valid based on the start and end periods.
// Returns true if valid, false otherwise.
bool WeatherApp::isValidWeeklyRange(const std::string& start, const std::string& end) {
    return start < end && start.substr(0, 4) >= "1980" && end.substr(0, 4) <= "2019";
}

// Calculates the length of the range between start and end periods based on granularity.
// Returns the calculated range length.
int WeatherApp::calculateRangeLength(const std::string& start, const std::string& end, int granularity) {
    if (granularity == 1) { 
        // Yearly granularity: number of years.
        return std::stoi(end) - std::stoi(start) + 1;
    } else if (granularity == 2) { 
        // Monthly granularity: number of months.
        int startY = std::stoi(start.substr(0, 4));
        int endY   = std::stoi(end.substr(0, 4));
        int startM = std::stoi(start.substr(6, 2));
        int endM   = std::stoi(end.substr(6, 2));
        return (endY - startY) * 12 + (endM - startM) + 1;
    } else if (granularity == 3) { 
        // Weekly granularity: number of weeks.
        int startY = std::stoi(start.substr(0, 4));
        int endY   = std::stoi(end.substr(0, 4));
        int startW = std::stoi(start.substr(6, 2));
        int endW   = std::stoi(end.substr(6, 2));
        return (endY - startY) * 53 + (endW - startW) + 1;
    }
    return 0;
}
