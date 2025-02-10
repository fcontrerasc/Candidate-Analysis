#include "FilterManager.h"
#include <algorithm>
#include <iterator>

std::vector<Candidate> FilterManager::filterByGPA(const std::vector<Candidate>& candidates, double min, double max) {
    std::vector<Candidate> result;
    std::copy_if(candidates.begin(), candidates.end(), std::back_inserter(result),
        [min, max](const Candidate& c) { return c.gpa >= min && c.gpa <= max; });
    return result;
}

std::vector<Candidate> FilterManager::filterByUniversity(const std::vector<Candidate>& candidates, const std::string& university) {
    std::vector<Candidate> result;
    std::copy_if(candidates.begin(), candidates.end(), std::back_inserter(result),
        [university](const Candidate& c) { return c.university == university; });
    return result;
}

std::vector<Candidate> FilterManager::filterBySkills(const std::vector<Candidate>& candidates, const std::vector<std::string>& requiredSkills) {
    std::vector<Candidate> result;
    std::copy_if(candidates.begin(), candidates.end(), std::back_inserter(result),
        [requiredSkills](const Candidate& c) {
            for (const auto& skill : requiredSkills) {
                if (std::find(c.skills.begin(), c.skills.end(), skill) == c.skills.end()) return false;
            }
            return true;
        });
    return result;
}