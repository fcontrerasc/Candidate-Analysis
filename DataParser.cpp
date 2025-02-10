#include "DataParser.h"
#include "Logger.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <tinyxml2.h>
#include <sstream>

// JSON Parser (using RapidJSON)
std::vector<Candidate> JSONParser::parse(const std::string& rawData, const std::string& university) {

    Logger::getInstance().log("Parsing JSON file: " + university);

    std::vector<Candidate> candidates;
    rapidjson::Document doc;
    doc.Parse(rawData.c_str());

    if (doc.HasParseError()) {
        Logger::getInstance().log("JSON parse error: " + std::string(rapidjson::GetParseError_En(doc.GetParseError())));
        Logger::getInstance().log("Error offset: " + std::to_string(doc.GetErrorOffset()));
        return candidates;
    }

    for (const auto& entry : doc.GetArray()) {
        try {
            Candidate c;
			if (!entry.HasMember("name") || !entry.HasMember("GPA") || !entry.HasMember("skills")) {
				throw std::runtime_error("Missing mandatory fields");
			}
            c.name = entry["name"].GetString();
            c.gpa = entry["GPA"].GetDouble();
            for (const auto& skill : entry["skills"].GetArray()) {
                c.skills.push_back(skill.GetString());
            }
            c.university = university;
            candidates.push_back(c);
        }
        catch (const std::exception& e) {
            Logger::getInstance().log(std::string("Error parsing candidate: ") + e.what());
        }
    }
    return candidates;
}

// XML Parser (using TinyXML-2)
std::vector<Candidate> XMLParser::parse(const std::string& rawData, const std::string& university) {
    
	Logger::getInstance().log("Parsing XML file: " + university);
    
    std::vector<Candidate> candidates;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.Parse(rawData.c_str());

    if (eResult != tinyxml2::XML_SUCCESS) {
        Logger::getInstance().log("XML parse error: " + std::string(doc.ErrorStr()));
		Logger::getInstance().log("Error ID: " + std::to_string(doc.ErrorID()));
        return candidates;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("root");
    if (!root) {
        Logger::getInstance().log("XML parse error: No root node");
        return candidates;
    }

    for (tinyxml2::XMLElement* node = root->FirstChildElement("candidate"); node; node = node->NextSiblingElement("candidate")) {
        try {
            Candidate c;
            const char* name = node->FirstChildElement("name")->GetText();
            const char* gpa = node->FirstChildElement("GPA")->GetText();
			const char* skills = node->FirstChildElement("skills")->GetText(); // Check for at least one skill

		    if (!name || !gpa || !skills) {
				throw std::runtime_error("Missing mandatory fields");
		    }
            c.name = name;
            c.gpa = std::stod(gpa);

            for (tinyxml2::XMLElement* skillNode = node->FirstChildElement("skills"); skillNode; skillNode = skillNode->NextSiblingElement("skills")) {
                const char* skill = skillNode->GetText();
                if (skill) {
                    c.skills.push_back(skill);
                }
            }
            c.university = university;

            candidates.push_back(c);
		}
        catch (const std::exception& e) {
			Logger::getInstance().log(std::string("Error parsing candidate: ") + e.what());
		}
    }
    return candidates;
}

// Parser Factory
Parser* ParserFactory::createParser(const std::string& fileType) {
    if (fileType == "json") return new JSONParser();
    if (fileType == "xml") return new XMLParser();
    Logger::getInstance().log("Unsupported file type: " + fileType);
    return nullptr;
}