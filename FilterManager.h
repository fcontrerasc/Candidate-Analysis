#pragma once
#include "DataParser.h"
#include <functional>

class FilterManager {
public:
    static std::vector<Candidate> filterByGPA(const std::vector<Candidate>& candidates, double min, double max);
    static std::vector<Candidate> filterByUniversity(const std::vector<Candidate>& candidates, const std::string& university);
    static std::vector<Candidate> filterBySkills(const std::vector<Candidate>& candidates, const std::vector<std::string>& requiredSkills);
};