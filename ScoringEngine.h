#pragma once
#include "DataParser.h"
#include <map>

struct ScoringWeights {
    float gpaWeight = 0.4f;
    float skillWeight = 0.4f;
    std::map<std::string, float> universityWeights = {
        {"University of Florida", 0.2f},
        {"Polytechnic University of Bucharest", 0.1f},
        // Add other universities
    };
};

class ScoringEngine {
public:
    static float calculateScore(const Candidate& candidate, const ScoringWeights& weights);
    static std::vector<Candidate> rankCandidates(std::vector<Candidate>& candidates, const ScoringWeights& weights);
};