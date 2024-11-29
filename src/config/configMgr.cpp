#include "configMgr.hpp"
ConfigMgr* ConfigMgr::mInst = nullptr;

ConfigMgr* ConfigMgr::getInst()
{
    if (!ConfigMgr::mInst)
    {
        ConfigMgr::mInst = new ConfigMgr();
    }

    return ConfigMgr::mInst;
}

std::shared_ptr<ConfigMgrLineType> ConfigMgr::getConfig(std::string& configName)
{
    auto iter = this->mConfigs.find(configName);
    if (iter == this->mConfigs.end())
    {
        return nullptr;
    }

    return iter->second;
}

std::shared_ptr<ConfigMgrKeyType> ConfigMgr::getLine(std::string &configName, std::string &id)
{
    auto config = this->getConfig(configName);
    if (!config)
    {
        return nullptr;
    }

    auto configPtr = config.get();

    auto iter = configPtr->find(id);
    if (iter == configPtr->end())
    {
        return nullptr;
    }

    return iter->second;
}

std::shared_ptr<Config> ConfigMgr::getValue(std::string &configName, std::string &id, std::string &key)
{
    auto line = this->getLine(configName, id);
    if (!line)
    {
        return nullptr;
    }

    auto linePtr = line.get();

    auto iter = linePtr->find(key);
    if (iter == linePtr->end())
    {
        return nullptr;
    }

    return iter->second;
}
