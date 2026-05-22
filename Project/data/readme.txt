================================================================================
                              DATA DIRECTORY
================================================================================

DATASET INFORMATION
-------------------
File: weather_data_EU_1980-2019_temp_only.csv (74 MB)
Content: European temperature data (hourly) from 1980-2019
Coverage: 28 European countries (AT, BE, BG, CH, CZ, DE, DK, EE, ES, FI, FR, 
          GB, GR, HR, HU, IE, IT, LT, LU, LV, NL, NO, PL, PT, RO, SE, SI, SK)

DOWNLOAD
--------
Pre-processed Temperature-Only Version:
https://drive.google.com/file/d/1XK9hOdXs_WsXXDgYMDC2JroZd86JfShM/view?usp=sharing

Original Source (Full Dataset):
https://data.open-power-system-data.org/weather_data/2020-09-16

SETUP
-----
1. Download the CSV file from the Google Drive link above
2. Place it in this directory: data/weather_data_EU_1980-2019_temp_only.csv
3. Run the application from the project root

FILE FORMAT
-----------
timestamp,AT_temperature,BE_temperature,...,SK_temperature
2019-01-01 00:00,5.2,3.1,...,2.8
2019-01-01 01:00,4.9,2.8,...,2.5
...

Columns: Timestamp + 28 country temperature columns
Format: CSV with hourly records
Range: 1980-01-01 to 2019-12-31

================================================================================ 
