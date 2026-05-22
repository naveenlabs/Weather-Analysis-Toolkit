#include "WeatherApp.h"

int main() {
    // File containing weather data for processing.
    const std::string filename = "weather_data_EU_1980-2019_temp_only.csv";

    // Initialize the WeatherApp with the data file.
    WeatherApp app(filename);

    // Run the application.
    app.run();

    return 0;
}
