#include "Localization.h"
#include "OSCompatibilityLayer.h"
#include <filesystem>
#include <fstream>
#include <regex>
namespace fs = std::filesystem;

Localization::Localization()
{
	if (!fs::exists("Configuration/converter_languages.yml"))
	{
		Log(LogLevel::Error) << "No localization found!";
		return;
	}

	std::ifstream langfile("Configuration/converter_languages.yml");
	std::string line;
	while (std::getline(langfile, line))
	{
		auto pos = line.find_first_of(':');
		auto language = line.substr(2, pos - 2);
		pos = line.find_first_of('\"');
		const auto secpos = line.find_last_of('\"');
		auto langtext = line.substr(pos + 1, secpos - pos - 1);
		languages.insert(std::pair(language, langtext));
		loadedLangauges.emplace_back(language);
	}
	langfile.close();
	loadLanguages();

	if (fs::exists("Configuration/fronter-language.txt"))
	{
		std::ifstream userFile("Configuration/fronter-language.txt");
		std::getline(userFile, line);
		if (line.find("language=") == 0)
		{
			setLanguage = line.substr(9, line.length());
		}
		userFile.close();
	}
}

void Localization::loadLanguages()
{
	std::set<std::string> fileNames;
	Utils::GetAllFilesInFolder("Configuration/", fileNames);

	for (const auto& fileName: fileNames)
	{
		if (fileName.find(".yml") == std::string::npos)
			continue;
		std::ifstream langfile("Configuration/" + fileName);
		std::string line;
		std::getline(langfile, line);
		if (line.find("l_") != 0)
		{
			Log(LogLevel::Error) << "Configuration/" << fileName << " is not a localization file!";
			langfile.close();
			continue;
		}
		auto pos = line.find(':');
		const auto language = line.substr(2, pos - 2);
		while (std::getline(langfile, line))
		{
			pos = line.find_first_of(':');
			auto key = line.substr(1, pos - 1);
			pos = line.find_first_of('\"');
			const auto secpos = line.find_last_of('\"');
			auto text = line.substr(pos + 1, secpos - pos - 1);
			translations[key].insert(std::pair(language, text));
		}
		langfile.close();
	}
}

std::wstring Localization::translate(const std::string& key)
{
	std::string toReturn;
	if (!translations.count(key))
	{
		return std::wstring();
	}
	if (translations[key].count(setLanguage))
		toReturn = translations[key][setLanguage];
	else if (translations[key].count("english"))
		toReturn = translations[key]["english"];
	else
		return std::wstring();

	toReturn = std::regex_replace(toReturn, std::regex(R"(\\n)"), "\n");
	return Utils::convertUTF8ToUTF16(toReturn);
}

std::wstring Localization::translateLanguage(const std::string& language)
{
	if (!languages.count(language))
	{
		return std::wstring();
	}
	return Utils::convertUTF8ToUTF16(languages[language]);
}

void Localization::saveLanguage(int ID)
{
	if (ID > static_cast<int>(loadedLangauges.size()) + 1)
		return;
	setLanguage = loadedLangauges[ID];
	std::ofstream langfile("Configuration/fronter-language.txt");
	if (langfile.is_open())
	{
		langfile << "language=" << loadedLangauges[ID];
	}
	langfile.close();
}
