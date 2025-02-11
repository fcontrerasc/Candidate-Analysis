#pragma once
#include "DataParser.h"
#include <map>

struct ScoringWeights {
    float gpaWeight = 0.0f;
    float skillWeight = 0.0f;
	std::vector<std::string> requiredSkills;
    std::map<std::string, float> universityWeights;
};

class ScoringEngine {
public:
    static float calculateScore(Candidate& candidate, const ScoringWeights& weights);
    static std::vector<Candidate> rankCandidates(std::vector<Candidate>& candidates, const ScoringWeights& weights);
};