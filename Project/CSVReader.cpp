#include "CSVReader.h"
#include <sstream>

// Tokenizes a string into substrings based on the specified delimiter.
std::vector<std::string> CSVReader::tokenise(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;  
    std::istringstream lineStream(line);  
    std::string token;

    // Extract substrings separated by the delimiter and add them to the tokens vector.
    while (std::getline(lineStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens; 
}
