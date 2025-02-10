#include "PersistenceHandler.h"
#include "Logger.h"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

bool PersistenceHandler::saveToFile(const std::vector<Candidate>& candidates, const std::string& filename) {
    rapidjson::Document doc;
    doc.SetArray();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    for (const auto& c : candidates) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("name", rapidjson::Value(c.name.c_str(), allocator), allocator);
        obj.AddMember("gpa", c.gpa, allocator);
        obj.AddMember("university", rapidjson::Value(c.university.c_str(), allocator), allocator);

        rapidjson::Value skills(rapidjson::kArrayType);
        for (const auto& skill : c.skills) {
            skills.PushBack(rapidjson::Value(skill.c_str(), allocator), allocator);
        }
        obj.AddMember("skills", skills, allocator);

        doc.PushBack(obj, allocator);
    }

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

std::vector<Candidate> PersistenceHandler::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Logger::getInstance().log("Failed to load file: " + filename);
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    JSONParser parser;
    return parser.parse(content, "default university");
}