# Weather Analysis Toolkit

[![Status: Complete](https://img.shields.io/badge/Status-Complete-brightgreen)]() [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE) [![Language: C++](https://img.shields.io/badge/Language-C%2B%2B11-blue)]() 

A command-line weather data analysis toolkit for computing candlestick statistics, generating text-based visualizations, filtering temperature data, and predicting future trends using linear regression on 40 years of European climate data.

## Overview

Weather Analysis Toolkit processes European temperature data (1980-2019) with hourly granularity across 28 countries. The application computes candlestick data for multiple time periods (yearly, monthly, weekly), renders text-based plots, applies intelligent filtering, and generates predictions using linear regression analysis.

## Features

**Candlestick Computation** – Aggregates hourly temperature data into candlestick format with open, high, low, close values across three time granularities.

**Text-Based Visualization** – Renders ASCII art candlestick plots with configurable height and period-aware labeling.

**Data Filtering** – Filters by date range, temperature range, or combined criteria with validation for multiple granularities including leap year handling.

**Prediction Engine** – Implements linear regression to extrapolate temperature trends into future periods with slope and intercept calculations.

**Interactive CLI** – Menu-driven interface with country selection from 28 European nations and granularity options.

## Tech Stack

- **Language:** C++11
- **Compiler:** g++ (GCC)
- **Standard:** ISO/IEC 14882:2011
- **Data Format:** CSV (comma-separated values)
- **Dataset Size:** 74 MB (temperature-only variant)

## Setup & Usage

### Prerequisites

- C++11 compatible compiler (g++, clang, MSVC)
- 100 MB free disk space
- Terminal/Command prompt access

### Installation

1. Clone the repository:
```bash
git clone https://github.com/naveenlabs/Weather-Analysis-Toolkit.git
cd Weather-Analysis-Toolkit
```

2. Download dataset:
   - Navigate to `data/` folder
   - Read `data/README.txt` for download instructions
   - Download pre-processed CSV: [Google Drive Link](https://drive.google.com/file/d/1XK9hOdXs_WsXXDgYMDC2JroZd86JfShM/view?usp=sharing)
   - Place file as: `data/weather_data_EU_1980-2019_temp_only.csv`

3. Compile:
```bash
g++ -std=c++11 -o weather_app *.cpp
```

4. Run:
```bash
./weather_app
```

### Usage Example

```
============================================================
                     Weather Application                    
============================================================

Please choose an option below:

1. Display Candlestick Data
2. Plot Candlestick Data
3. Apply Filters and Plot Data
4. Predict Future Data and Plot
5. Exit Application

Enter your choice (1-5): 1
```

## Data

The toolkit requires European temperature data spanning 1980-2019 with hourly records across 28 countries (AT, BE, BG, CH, CZ, DE, DK, EE, ES, FI, FR, GB, GR, HR, HU, IE, IT, LT, LU, LV, NL, NO, PL, PT, RO, SE, SI, SK).

**Dataset Location:** `data/weather_data_EU_1980-2019_temp_only.csv` (74 MB)

For detailed download and setup instructions, see `data/README.txt`.

## Implementation Details

### Candlestick Computation

Aggregates hourly temperature data into period-based candlestick format:
- **Open:** Average temperature from previous period
- **Close:** Average temperature from current period
- **High:** Maximum temperature in period
- **Low:** Minimum temperature in period

Supports three granularities: yearly (YYYY), monthly (YYYY-Mxx), weekly (YYYY-Wxx with ISO week numbering).

### Linear Regression Prediction

Generates future predictions using least squares linear regression:

```
y = mx + b

where:
  m = slope = Σ(dx*dy) / Σ(dx²)
  b = intercept = mean(y) - slope * mean(x)
  dx = x[i] - mean(x)
  dy = y[i] - mean(y)
```

Computes separate regression lines for close, high, and low values to produce realistic predictions.

### Text-Based Plotting

Renders candlestick data using ASCII characters with configurable plot height. Each candlestick represents a time period with visual representation of open/close/high/low values.

### Filtering Engine

Implements three filter modes:
1. **Period Range Filter** – Validates and filters by start/end dates with granularity-aware comparison
2. **Temperature Range Filter** – Filters by minimum low temperature and maximum high temperature bounds
3. **Combined Filter** – Applies both period and temperature constraints simultaneously

## Classes & Components

**Candlestick** – Data structure representing candlestick format with year/period label and OHLC values.

**WeatherDataProcessor** – Static methods for computing candlestick data from raw CSV with granularity support.

**WeatherApp** – Main application class managing CLI, menu logic, state management, and user interaction.

**TextPlotter** – Renders text-based candlestick visualizations with configurable dimensions.

**FilterData** – Static utility methods for period range and temperature range filtering.

**Prediction** – Linear regression implementation for trend extrapolation into future periods.

**CSVReader** – Utility for tokenizing CSV lines by delimiter.

## Compilation Flags

```bash
g++ -std=c++11 -o weather_app *.cpp
```

- `-std=c++11` – Enables C++11 standard (required for regex, stoi, vector)
- `-o weather_app` – Output executable name

## Performance

- **Data Loading:** ~2-5 seconds (74 MB CSV)
- **Candlestick Computation:** Yearly: <100ms, Monthly: <200ms, Weekly: <300ms
- **Prediction:** Linear regression on 40 years: <50ms
- **Memory Usage:** ~150-200 MB (dataset dependent)

## Supported Countries

Austria, Belgium, Bulgaria, Switzerland, Czech Republic, Germany, Denmark, Estonia, Spain, Finland, France, United Kingdom, Greece, Croatia, Hungary, Ireland, Italy, Lithuania, Luxembourg, Latvia, Netherlands, Norway, Poland, Portugal, Romania, Sweden, Slovenia, Slovakia.

## Documentation

Complete assignment documentation:
- **Report:** [OOP Report.pdf](Documentation/OOP%20-%20Report.pdf)
- **Demo Video:** [OOP Demonstration.mp4](https://drive.google.com/file/d/1Ihh07TTABkmnlbsBE-YA6qkDfFqJW2ZR/view?usp=sharing)

## Author

**Dhanarasu Naveen**  
Computer Science (AI & Machine Learning Specialisation)  
University of London via SIM Singapore  

## License

MIT License – see [LICENSE](LICENSE) file for details.
