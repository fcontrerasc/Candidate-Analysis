#pragma once  
#include <sqlite3.h>
#include <vector>  
#include "DataParser.h"

class DatabaseManager {
private:
    sqlite3* db;
    static DatabaseManager* instance;
    DatabaseManager() : db(nullptr) {}  // Private constructor

public:
    static DatabaseManager& getInstance();
    bool open(const std::string& dbPath);
    void close();
    bool createTables();
    bool insertCandidate(const Candidate& candidate, const std::string& downloadDate);
    std::vector<Candidate> getCandidatesByDate(const std::string& date);
    bool beginTransaction();
    bool commitTransaction();
    bool insertCandidates(const std::vector<Candidate>& candidates, const std::string& date);
};