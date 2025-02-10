#pragma once  
#include <cpprest/http_client.h>
#include <string>  
#include <vector>  

class DataDownloader {
public:
	DataDownloader() = default;
	void downloadAllFeedsAsync(const std::vector<std::string>& urls);
	void downloadFeedWithRetry(const std::string& url, int maxRetries);

	std::string url_to_filename(const std::string& url);
	std::string loadFile(const std::string& filename);
	void saveToFile(const std::string& filename, const std::string& data);
	std::string wstring_to_string(const std::wstring& wstr);
	std::string getFileExtension(const std::string& filename);
	std::string extractUniversityName(const std::string& url);
};