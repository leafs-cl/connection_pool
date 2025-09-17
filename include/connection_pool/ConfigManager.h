#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include<string>
#include<memory>

class ConfigManager{
    public:
        virtual ~ConfigManager() = default;
        virtual bool loadConfig(const std::string &fileName) = 0;
        virtual std::string getString(const std::string& key, const std::string& defaultValue="") = 0;
        virtual int getInt(const std::string& key, int defaultValue = 0) = 0;
        virtual bool getBool(const std::string& key, bool defaultValue = false) = 0;
};

std::unique_ptr<ConfigManager> createConfigManager(const std::string& filename);

#endif //CONFIG_MANAGER_H