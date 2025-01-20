#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <sstream>
#include <exception>
#include "utils.h"

class Config
{
public:
    std::string title;
    size_t windowWidth, windowHeight;
    std::string spirvPath;
    
    Config () = delete;
    Config (Config& rhs) = delete;
    Config (Config&& rhs) = delete;
    Config (std::string configFilepath)
    {
        std::ifstream configFile(configFilepath);
        if (!configFile.is_open()) {
            throw std::runtime_error(std::format("could not open config file: ", configFilepath));
        }
        logInfo(std::format("Found {} as config file", configFilepath));

        std::string line;
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

                    if (key == "title") {
                        title = value;
                    } else if (key == "windowWidth") {
                        windowWidth = std::stoul(value);
                    } else if (key == "windowHeight") {
                        windowHeight = std::stoul(value);
                    } else if (key == "spirvPath") {
                        spirvPath = value;
                    } else {
                        std::cerr << "Warning: Unknown config key: " << key << std::endl;
                    }
                }
            }
        }

        configFile.close();
    } 
};

#endif
