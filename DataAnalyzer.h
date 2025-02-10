#pragma once
#include "DatabaseManager.h"
#include <map>
#include <vector>

struct UniversityStats {
    int totalCandidates;
    double averageGpa;
};

class DataAnalyzer {
public:
    static int getTotalCandidates(const std::vector<Candidate>& candidates);
    static std::map<std::string, UniversityStats> getUniversityStats(const std::vector<Candidate>& candidates);
    static std::vector<std::pair<std::string, int>> getSkillsStats(const std::vector<Candidate>& candidates);
};