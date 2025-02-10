#pragma once
#include "DataParser.h"
#include <string>

class PersistenceHandler {
public:
    static bool saveToFile(const std::vector<Candidate>& candidates, const std::string& filename);
    static std::vector<Candidate> loadFromFile(const std::string& filename);
};