#include "PersistenceHandler.h"
#include "Logger.h"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

bool PersistenceHandler::saveToFile(const std::vector<Candidate>& candidates, const std::string& filename, const FilterData& filterData) {
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    // Add filterData to the document
    rapidjson::Value filterObj(rapidjson::kObjectType);
	filterObj.AddMember("date", rapidjson::Value(filterData.date.c_str(), allocator), allocator);
    filterObj.AddMember("universityChecked", filterData.universityChecked, allocator);
    filterObj.AddMember("gpaChecked", filterData.gpaChecked, allocator);
    filterObj.AddMember("skillsChecked", filterData.skillsChecked, allocator);
    filterObj.AddMember("university", rapidjson::Value(filterData.university.c_str(), allocator), allocator);
    filterObj.AddMember("minGPA", filterData.minGPA, allocator);
    filterObj.AddMember("maxGPA", filterData.maxGPA, allocator);

    rapidjson::Value skills(rapidjson::kArrayType);
    for (const auto& skill : filterData.skills) {
        skills.PushBack(rapidjson::Value(skill.c_str(), allocator), allocator);
    }
    filterObj.AddMember("skills", skills, allocator);

    doc.AddMember("filterData", filterObj, allocator);

    // Add candidates to the document
    rapidjson::Value candidatesArray(rapidjson::kArrayType);
    for (const auto& c : candidates) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("name", rapidjson::Value(c.name.c_str(), allocator), allocator);
        obj.AddMember("GPA", c.gpa, allocator);
        obj.AddMember("university", rapidjson::Value(c.university.c_str(), allocator), allocator);

        rapidjson::Value skills(rapidjson::kArrayType);
        for (const auto& skill : c.skills) {
            skills.PushBack(rapidjson::Value(skill.c_str(), allocator), allocator);
        }
        obj.AddMember("skills", skills, allocator);

        candidatesArray.PushBack(obj, allocator);
    }
    doc.AddMember("candidates", candidatesArray, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream file(filename);
    if (!file.is_open()) {
        Logger::getInstance().log("Failed to save file: " + filename);
        return false;
    }
    file << buffer.GetString();
    return true;
}

std::vector<Candidate> PersistenceHandler::loadFromFile(const std::string& filename, FilterData& filterData) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Logger::getInstance().log("Failed to load file: " + filename);
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    if (doc.Parse(content.c_str()).HasParseError()) {
        Logger::getInstance().log("Failed to parse JSON file: " + filename);
        return {};
    }

    Logger::getInstance().log("Parsing JSON file Filter Data: " + filename);

    // Retrieve filterData
    if (doc.HasMember("filterData") && doc["filterData"].IsObject()) {
        const rapidjson::Value& filterObj = doc["filterData"];
		if (filterObj.HasMember("date") && filterObj["date"].IsString()) {
			filterData.date = filterObj["date"].GetString();
		}
		else {
			Logger::getInstance().log("Failed to parse date data");
		}
        if (filterObj.HasMember("universityChecked") && filterObj["universityChecked"].IsBool()) {
            filterData.universityChecked = filterObj["universityChecked"].GetBool();
		}
		else {
            Logger::getInstance().log("Failed to parse university filter checkBox data");
		}
        if (filterObj.HasMember("gpaChecked") && filterObj["gpaChecked"].IsBool()) {
            filterData.gpaChecked = filterObj["gpaChecked"].GetBool();
        }
		else {
			Logger::getInstance().log("Failed to parse gpa filter checkBox data");
		}
        if (filterObj.HasMember("skillsChecked") && filterObj["skillsChecked"].IsBool()) {
            filterData.skillsChecked = filterObj["skillsChecked"].GetBool();
        }
		else {
			Logger::getInstance().log("Failed to parse skills filter checkBox data");
		}
        if (filterObj.HasMember("university") && filterObj["university"].IsString()) {
            filterData.university = filterObj["university"].GetString();
        }
		else {
			Logger::getInstance().log("Failed to parse university filter value");
		}
        if (filterObj.HasMember("minGPA") && filterObj["minGPA"].IsDouble()) {
            filterData.minGPA = filterObj["minGPA"].GetDouble();
        }
		else {
			Logger::getInstance().log("Failed to parse minGPA filter value");
		}
        if (filterObj.HasMember("maxGPA") && filterObj["maxGPA"].IsDouble()) {
            filterData.maxGPA = filterObj["maxGPA"].GetDouble();
        }
		else {
			Logger::getInstance().log("Failed to parse maxGPA filter value");
		}
        if (filterObj.HasMember("skills") && filterObj["skills"].IsArray()) {
            for (const auto& skill : filterObj["skills"].GetArray()) {
                if (skill.IsString()) {
                    filterData.skills.push_back(skill.GetString());
                }
            }
        }
        else {
            Logger::getInstance().log("Failed to parse skills filter value");
        }
    }

    Logger::getInstance().log("Parsing JSON file Filter Candidates: " + filename);

    // Retrieve candidates
    std::vector<Candidate> candidates;
    if (doc.HasMember("candidates") && doc["candidates"].IsArray()) {
        for (const auto& candidateObj : doc["candidates"].GetArray()) {
            Candidate candidate;
            if (candidateObj.HasMember("name") && candidateObj["name"].IsString()) {
                candidate.name = candidateObj["name"].GetString();
            }
			else {
				Logger::getInstance().log("Failed to parse name data");
			}
            if (candidateObj.HasMember("GPA") && candidateObj["GPA"].IsDouble()) {
                candidate.gpa = candidateObj["GPA"].GetDouble();
            } 
			else {
				Logger::getInstance().log("Failed to parse GPA data");
			}
            if (candidateObj.HasMember("university") && candidateObj["university"].IsString()) {
                candidate.university = candidateObj["university"].GetString();
            }
			else {
				Logger::getInstance().log("Failed to parse university data");
			}
            if (candidateObj.HasMember("skills") && candidateObj["skills"].IsArray()) {
                for (const auto& skill : candidateObj["skills"].GetArray()) {
                    if (skill.IsString()) {
                        candidate.skills.push_back(skill.GetString());
                    }
                }
            }
			else {
				Logger::getInstance().log("Failed to parse skills data");
			}
            candidates.push_back(candidate);
        }
    }
	else {
		Logger::getInstance().log("Failed to parse candidates data");
	}

    return candidates;
}