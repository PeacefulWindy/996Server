#pragma once
#include<unordered_map>
#include<memory>
#include<string>

enum class ConfigValueType
{
	None = 0,
	Int = 1,
	Float = 2,
	Bool = 3,
	String = 4,
	Json = 5,
};

struct Config
{
	ConfigValueType type = ConfigValueType::None;
	int64_t intValue = 0;
	float floatValue = 0;
	std::string stringValue;
};

using ConfigMgrKeyType = std::unordered_map<std::string, std::shared_ptr<Config>>;
using ConfigMgrLineType = std::unordered_map<std::string, std::shared_ptr<ConfigMgrKeyType>>;
using ConfigMgrType = std::unordered_map<std::string, std::shared_ptr<ConfigMgrLineType>>;

class ConfigMgr
{
public:
	static ConfigMgr* getInst();

public:
	ConfigMgrType mConfigs;

public:
	std::shared_ptr<ConfigMgrLineType> getConfig(std::string &configName);
	std::shared_ptr<ConfigMgrKeyType> getLine(std::string &configName, std::string &id);
	std::shared_ptr<Config> getValue(std::string &configName, std::string &id, std::string &key);

private:
	static ConfigMgr* mInst;
};