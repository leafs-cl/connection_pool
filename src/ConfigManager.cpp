#include "ConfigManager.h"
#include <stdexcept>
#include <algorithm>
#include "Logger.hpp"
#ifdef USE_SIMPLE_INI
#include "SimpleIni.h"
class IniConfigManager : public ConfigManager
{
public:
    bool loadConfig(const std::string &filename) override
    {
        _ini.SetUnicode();
        SI_ERROR rc = _ini.LoadFile(filename.c_str());
        return (rc >= 0)
    }
    std::string getString(const std::string &key, const std::string &defaultValue) override
    {
        const char *value = _ini.GetValue("", key.c_str(), defaultValue.c_str());
        return value ? std::string(value) : defaultValue;
    }
    int getInt(const std::string &key, int defaultValue) override
    {
        return static_cast<int>(_ini.GetLongValue("", key.c_str(), defaultValue));
    }
    bool getBool(const std::string &key, bool defaultValue) override
    {
        return _ini.GetBoolValue("", key.c_str(), defaultValue);
    }

private:
    CSimpleIniA _ini;
};
#endif

#ifdef USE_YAML_CPP
#include <yaml-cpp/yaml.h>
class YamlConfigManager : public ConfigManager
{
public:
    bool loadConfig(const std::string &filename) override
    {
        try
        {
            _config = YAML::LoadFile(filename);
            return true;
        }
        catch (const YAML::Exception &e)
        {
            ERROR_LOG("Load config failed:" + e.msg);
            return false;
        }
    }
    std::string getString(const std::string &key, const std::string &defaultValue) override
    {
        try
        {
            return _config[key].as<std::string>();
        }
        catch (const YAML::Exception &e)
        {
            return defaultValue;
        }
    }
    int getInt(const std::string &key, int defaultValue) override
    {
        try
        {
            return _config[key].as<int>();
        }
        catch (const YAML::Exception &e)
        {
            return defaultValue;
        }
    }
    bool getBool(const std::string &key, bool defaultValue) override
    {
        try
        {
            return _config[key].as<bool>();
        }
        catch (const YAML::Exception &e)
        {
            return defaultValue;
        }
    }

private:
    YAML::Node _config;
};
#endif

std::unique_ptr<ConfigManager> createConfigManager(const std::string &filename)
{
    // Determine file loader according to file extension
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        std::string extension = filename.substr(dotPos + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if ("ini" == extension)
        {
#ifdef USE_SIMPLE_INI
            return std::make_unique<IniConfigManager>();
#else
            throw new std::runtime_error("INI support not compiled or not found!");
#endif
        }
        else if ("yaml" == extension || "yml" == extension)
        {
#ifdef USE_YAML_CPP
            return std::make_unique<YamlConfigManager>();
#else
            throw new std::runtime_error("YAML support not compiled or not found!");
#endif
        }
    }

#ifdef USE_SIMPLE_INI
    return std::make_unique<IniConfigManager>();
#else
    throw new std::runtime_error("Not implemented");
#endif
}