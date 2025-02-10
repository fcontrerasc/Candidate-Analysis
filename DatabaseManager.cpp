#include "DatabaseManager.h"  
#include "Logger.h"  
#include <mutex>  
#include <iostream>
#include <sstream>

// Mutex for thread-safe database access  
static std::mutex dbMutex;

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager& DatabaseManager::getInstance() {
    if (!instance) {
        instance = new DatabaseManager();
    }
    return *instance;
}

bool DatabaseManager::open(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        Logger::getInstance().log("Failed to open database: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    return true;
}

void DatabaseManager::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool DatabaseManager::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS candidates (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            gpa REAL CHECK (gpa BETWEEN 0 AND 4),
            university TEXT NOT NULL,
            skills TEXT,
            download_date TEXT NOT NULL
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        Logger::getInstance().log("Failed to create tables: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::insertCandidate(const Candidate& candidate, const std::string& downloadDate) {
    Logger::getInstance().log("Inserting candidate: " + candidate.name);
    
    // Serialize skills into a comma-separated string
    std::stringstream skillsStream;
    for (size_t i = 0; i < candidate.skills.size(); ++i) {
        skillsStream << candidate.skills[i];
        if (i < candidate.skills.size() - 1) skillsStream << ",";
    }
    std::string skillsStr = skillsStream.str();

    // Prepare SQL statement
    const char* sql = "INSERT INTO candidates (name, gpa, university, skills, download_date) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().log("Failed to prepare insert statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, candidate.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, candidate.gpa);
    sqlite3_bind_text(stmt, 3, candidate.university.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, skillsStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, downloadDate.c_str(), -1, SQLITE_STATIC);

    // Execute
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        Logger::getInstance().log("Failed to insert candidate: " + std::string(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<Candidate> DatabaseManager::getCandidatesByDate(const std::string& date) {
    std::vector<Candidate> candidates;
    const char* sql = "SELECT name, gpa, university, skills FROM candidates WHERE download_date = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance().log("Failed to prepare query: " + std::string(sqlite3_errmsg(db)));
        return candidates;
    }

    sqlite3_bind_text(stmt, 1, date.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Candidate c;
        c.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        c.gpa = sqlite3_column_double(stmt, 1);
        c.university = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        // Deserialize skills
        std::string skillsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::stringstream ss(skillsStr);
        std::string skill;
        while (std::getline(ss, skill, ',')) {
            c.skills.push_back(skill);
        }

        candidates.push_back(c);
    }

    sqlite3_finalize(stmt);
    return candidates;
}

bool DatabaseManager::beginTransaction() {
    std::lock_guard<std::mutex> lock(dbMutex);
    char* errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        Logger::getInstance().log("Failed to begin transaction: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::commitTransaction() {
    std::lock_guard<std::mutex> lock(dbMutex);
    char* errMsg = nullptr;
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        Logger::getInstance().log("Failed to commit transaction: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// Batch insert candidates  
bool DatabaseManager::insertCandidates(const std::vector<Candidate>& candidates, const std::string& date) {
    if (!beginTransaction()) return false;

    for (const auto& c : candidates) {
        if (!insertCandidate(c, date)) {
			continue;
        }
    }

    return commitTransaction();
}