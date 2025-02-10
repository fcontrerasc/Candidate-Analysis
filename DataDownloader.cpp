#include "DataDownloader.h"  
#include "Logger.h"  
#include <cpprest/http_client.h>  
#include <fstream>
#include <future>  
#include <vector>  
#include <locale>
#include <codecvt>

using namespace web;
using namespace web::http;
using namespace web::http::client;

// Function to get file extension from filename
std::string DataDownloader::getFileExtension(const std::string& filename) {
    size_t dotPosition = filename.find_last_of('.');
    if (dotPosition != std::string::npos && dotPosition != filename.length() - 1) {
        return filename.substr(dotPosition + 1);
    }
    return ""; // Return an empty string if no extension is 
}

// Function to save data to a file
void DataDownloader::saveToFile(const std::string& filename, const std::string& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << data;
        file.close();
    }
    else {
        Logger::getInstance().log("Failed to open file: " + filename);
    }
}

// Function to load file content
std::string DataDownloader::loadFile(const std::string& filename) {
    std::ifstream file(filename);
	std::vector <char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	buffer.push_back('\0');
	return buffer.data();
}

// Function to convert URL to filename (example implementation)
std::string DataDownloader::url_to_filename(const std::string& url) {
    // Replace special characters in the URL to create a valid filename
    std::string filename = url;
    std::replace(filename.begin(), filename.end(), '/', '_');
    std::replace(filename.begin(), filename.end(), ':', '_');
    return filename;
}

// Function to convert std::wstring to std::string
std::string DataDownloader::wstring_to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Function to extract university name from URL
std::string DataDownloader::extractUniversityName(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    size_t dot = url.find_last_of('.');
    if (lastSlash != std::string::npos && dot != std::string::npos && lastSlash < dot) {
        return url.substr(lastSlash + 1, dot - lastSlash - 1);
    }
    return "";
}

// Thread-safe download with retries
void DataDownloader::downloadFeedWithRetry(const std::string& url, int maxRetries) {
    for (int i = 0; i < maxRetries; ++i) {
        try {
            http_client client(utility::conversions::to_string_t(url));
            auto response = client.request(methods::GET).get();
            if (response.status_code() == status_codes::OK) {
                auto wdata = response.extract_string().get();
                std::string data = wstring_to_string(wdata);
                // Save data to file
                saveToFile(url_to_filename(url), data);
                Logger::getInstance().log("Downloaded: " + url);
                return;
            }
        }
        catch (const std::exception& e) {
            Logger::getInstance().log("Attempt " + std::to_string(i + 1) + " failed: " + url);
        }
    }
    Logger::getInstance().log("Download failed permanently: " + url);
}

// Parallel downloads using std::async
void DataDownloader::downloadAllFeedsAsync(const std::vector<std::string>& urls) {
    std::vector<std::future<void>> futures;
    for (const auto& url : urls) {
        futures.push_back(std::async(std::launch::async,
            &DataDownloader::downloadFeedWithRetry, this, url, 3));
    }
    // Wait for all downloads to complete  
    for (auto& fut : futures) fut.wait();
}