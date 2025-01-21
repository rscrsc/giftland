#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <sstream>
#include <exception>
#include <format>
#include <vector>
#include <unistd.h>
#include "utils.h"

class Config
{
public:
    std::string rootDir;
    std::string title;
    size_t windowWidth, windowHeight;
    std::string spirvPath;
    std::vector<const std::string> validKeys = {
        "rootDir", "title", "windowWidth", "windowHeight", "spirvPath" };
    size_t validOptionCnt = 0;
    
    Config () = delete;
    Config (Config& rhs) = delete;
    Config (Config&& rhs) = delete;
    Config (std::string configFilepath)
    {
        std::ifstream configFile(configFilepath);
        if (!configFile.is_open()) {
            char buffer[4096];
            if (getcwd(buffer, sizeof(buffer)) != nullptr) {
                throw std::runtime_error(std::format("could not open config file at {}/{}", std::string(buffer), configFilepath));
            } else {
                throw std::runtime_error(std::format("could not open config file at {}", configFilepath));
            }
        }
        logInfo(std::format("Found {} as config file", configFilepath));

        std::string line;
        try {
        while (std::getline(configFile, line)) {
            std::istringstream iss(line);
            std::string key;
            if (std::getline(iss, key, '=')) {
                std::string value;
                if (std::getline(iss, value)) {
                    // Trim leading and trailing whitespace from key and value
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    logInfo(std::format("- Read {} = {}", key, value));
                    if (key == "rootDir") {
                        if (value.empty()) throw std::runtime_error("empty value");
                        if (value.back() == '/') {
                            value.pop_back();
                        }
                        rootDir = value;
                        ++validOptionCnt;
                    } else if (key == "title") {
                        if (value.empty()) throw std::runtime_error("empty value");
                        title = value;
                        ++validOptionCnt;
                    } else if (key == "windowWidth") {
                        windowWidth = std::stoul(value);
                        ++validOptionCnt;
                    } else if (key == "windowHeight") {
                        windowHeight = std::stoul(value);
                        ++validOptionCnt;
                    } else if (key == "spirvPath") {
                        if (value.empty()) throw std::runtime_error("empty value");
                        if (value.back() == '/') {
                            value.pop_back();
                        }
                        spirvPath = value;
                        ++validOptionCnt;
                    } else {
                        std::cerr << "[Warning] Unknown config key: " << key << std::endl;
                    }
                }
            }
        }
        if (validOptionCnt < validKeys.size()) {
            throw std::runtime_error("config options not enough");
        }
        } catch (std::exception& e) {
            throw std::runtime_error(std::format("parsing config file failed: {}", e.what()));
        }

        configFile.close();
    } 
};

#endif
