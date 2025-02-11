#pragma once
#include "DataParser.h"
#include <string>

struct FilterData {
	std::string date = "";
	bool universityChecked = false;
	bool gpaChecked = false;
	bool skillsChecked = false;
	std::string university = "";
	double minGPA = 0.0f;
	double maxGPA = 0.0f;
	std::vector<std::string> skills;
};

class PersistenceHandler {
public:
    static bool saveToFile(const std::vector<Candidate>& candidates, const std::string& filename, const FilterData& filterData);
    static std::vector<Candidate> loadFromFile(const std::string& filename, FilterData& filterData);
};