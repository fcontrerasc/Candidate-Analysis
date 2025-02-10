#include "DataDownloader.h"
#include "DataParser.h"
#include "DatabaseManager.h"
#include "DataAnalyzer.h"
#include "FilterManager.h"
#include "ScoringEngine.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");  // Format: YYYY-MM-DD
    return ss.str();
}

int main() {

    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.open("candidates.db")) {
        // Print error message
        std::cout << "Failed to open database" << std::endl;
        // return 1; // Exit on failure
    }
    if (!db.createTables()) {
        // Print error message
        std::cout << "Failed to create tables" << std::endl;
        // return 1;
    }

    // URLs to download
    std::vector<std::string> urls = {
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/Polytechnic-University-of-Bucharest.xml",
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/University-of-S%C3%A3o-Paulo.xml",
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/University-of-Florida.json",
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/University-of-Havana.json"
    };

    // Download data
    DataDownloader downloader;
    downloader.downloadAllFeedsAsync(urls);

    // Batch insert with transaction  
    std::string today = getCurrentDate();
    // db.beginTransaction();
    for (const auto& url : urls) {
        // Determine file type from URL
        std::string fileType = downloader.getFileExtension(url);
        // Create the appropriate parser
        Parser* parser = ParserFactory::createParser(fileType);
        if (parser) {
            // Load parsed candidates from file  
            std::string rawData = downloader.loadFile(downloader.url_to_filename(url));
            std::string university = downloader.extractUniversityName(url);
			auto candidates = parser->parse(rawData, university);
            if (!db.insertCandidates(candidates, today)) {
                std::cout << "Failed to insert candidates for URL: " << url << std::endl;
                delete parser; // Clean up the parser
                // return 1;
            }
        }
        else {
            std::cout << "Unsupported file type: " << fileType << std::endl;
        }
    }
    // db.commitTransaction();
    // while (1);

    // return 0;
}