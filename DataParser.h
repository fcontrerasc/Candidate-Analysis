#pragma once
#include <string>
#include <vector>

struct Candidate {
    std::string name;
    double gpa;
    std::vector<std::string> skills;
    std::string university;
};

class Parser {
public:
    virtual std::vector<Candidate> parse(const std::string& rawData, const std::string& university) = 0;
};

class JSONParser : public Parser {
public:
    std::vector<Candidate> parse(const std::string& rawData, const std::string& university) override;
};

class XMLParser : public Parser {
public:
    std::vector<Candidate> parse(const std::string& rawData, const std::string& university) override;
};

class ParserFactory {
public:
    static Parser* createParser(const std::string& fileType);
};