#include "DataAnalyzer.h"
#include <numeric>
#include <algorithm>

int DataAnalyzer::getTotalCandidates(const std::vector<Candidate>& candidates) {
    return candidates.size();
}

std::map<std::string, UniversityStats> DataAnalyzer::getUniversityStats(const std::vector<Candidate>& candidates) {
    std::map<std::string, UniversityStats> stats;
    std::map<std::string, std::vector<double>> gpaMap;

    for (const auto& c : candidates) {
        stats[c.university].totalCandidates++;
        gpaMap[c.university].push_back(c.gpa);
    }

    // Calculate average GPA per university
    for (auto& pair : gpaMap) {
        const std::string& uni = pair.first;
        std::vector<double>& data = pair.second;
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        stats[uni].averageGpa = sum / data.size();
    }

    return stats;
}

std::vector<std::pair<std::string, int>> DataAnalyzer::getSkillsStats(const std::vector<Candidate>& candidates) {
    std::map<std::string, int> skillCounts;
    for (const auto& c : candidates) {
        for (const auto& skill : c.skills) {
            skillCounts[skill]++;
        }
    }

    // Sort skills by frequency
    std::vector<std::pair<std::string, int>> sortedSkills(skillCounts.begin(), skillCounts.end());
    std::sort(sortedSkills.begin(), sortedSkills.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    //if (sortedSkills.size() > topN) sortedSkills.resize(topN);
    return sortedSkills;
}