#pragma once
#include <fstream>
#include <mutex>
#include <string>

class Logger {
private:
    std::mutex mtx;
    std::ofstream logFile;
    Logger(); // Private constructor

public:
    static Logger& getInstance();
    void log(const std::string& message);
    ~Logger();
};