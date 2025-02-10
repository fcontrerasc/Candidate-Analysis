#include "ScoringEngine.h"
#include <algorithm>

float ScoringEngine::calculateScore(const Candidate& candidate, const ScoringWeights& weights) {
    // University weight
    float uniWeight = 0.0f;
    auto it = weights.universityWeights.find(candidate.university);
    if (it != weights.universityWeights.end()) {
        uniWeight = it->second;
    }

    // Skill matches
    int matchingSkills = 0;
    for (const auto& skill : candidate.skills) {
        if (std::find(weights.requiredSkills.begin(), weights.requiredSkills.end(), skill) != weights.requiredSkills.end()) {
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