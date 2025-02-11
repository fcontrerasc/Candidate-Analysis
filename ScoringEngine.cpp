#include "ScoringEngine.h"
#include <algorithm>

float ScoringEngine::calculateScore(Candidate& candidate, const ScoringWeights& weights) {
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

	candidate.score = (candidate.gpa * weights.gpaWeight) + (matchingSkills * weights.skillWeight) + uniWeight;

	return candidate.score;
}

std::vector<Candidate> ScoringEngine::rankCandidates(std::vector<Candidate>& candidates, const ScoringWeights& weights) {
    
	for (auto& c : candidates) {
		calculateScore(c, weights);
	}
	std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
    return candidates;
}