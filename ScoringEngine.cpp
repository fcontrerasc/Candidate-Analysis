#include "ScoringEngine.h"
#include <algorithm>

float ScoringEngine::calculateScore(const Candidate& candidate, const ScoringWeights& weights) {
    // University weight (default to 0 if not found)
    float uniWeight = 0.0f;
    auto it = weights.universityWeights.find(candidate.university);
    if (it != weights.universityWeights.end()) {
        uniWeight = it->second;
    }

    // Skill matches (assume requiredSkills are predefined)
    std::vector<std::string> requiredSkills = { "C++", "Python", "Data Analysis" };
    int matchingSkills = 0;
    for (const auto& skill : candidate.skills) {
        if (std::find(requiredSkills.begin(), requiredSkills.end(), skill) != requiredSkills.end()) {
            matchingSkills++;
        }
    }

    return (candidate.gpa * weights.gpaWeight) + (matchingSkills * weights.skillWeight) + uniWeight;
}

std::vector<Candidate> ScoringEngine::rankCandidates(std::vector<Candidate>& candidates, const ScoringWeights& weights) {
    std::sort(candidates.begin(), candidates.end(),
        [weights](const Candidate& a, const Candidate& b) {
            return calculateScore(a, weights) > calculateScore(b, weights);
        });
    return candidates;
}