#pragma once
#include <vector>
#include <string>

// Class for handling CSV-related operations.
class CSVReader {
public:
    // Tokenizes a given string into a vector of substrings based on the specified delimiter.
    static std::vector<std::string> tokenise(const std::string& line, char delimiter);
};
