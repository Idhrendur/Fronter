#include "Configuration.h"
#include "CommonRegexes.h"
#include "FronterMod.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"
#include <filesystem>
#include <fstream>
#include <ranges>



using std::filesystem::exists;
using std::filesystem::path;



Configuration::Configuration()
{
	std::ofstream clearLog("log.txt");
	clearLog.close();
	registerKeys();
	if (exists("Configuration/fronter-configuration.txt"))
	{
		parseFile("Configuration/fronter-configuration.txt");
		Log(LogLevel::Info) << "Frontend configuration loaded.";
	}
	else
	{
		Log(LogLevel::Warning) << "Configuration/fronter-configuration.txt not found!";
	}
	if (exists("Configuration/fronter-options.txt"))
	{
		parseFile("Configuration/fronter-options.txt");
		Log(LogLevel::Info) << "Frontend options loaded.";
	}
	else
	{
		Log(LogLevel::Warning) << "Configuration/fronter-options.txt not found!";
	}
	clearRegisteredKeywords();
	registerPreloadKeys();
	if (!converterFolder.empty() && exists(converterFolder / "configuration.txt"))
	{
		Log(LogLevel::Info) << "Previous configuration located, preloading selections.";
		parseFile(converterFolder / "configuration.txt");
	}
	clearRegisteredKeywords();
}

void Configuration::registerPreloadKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& incomingKey, std::istream& theStream) {
		const commonItems::stringOfItem valueStr(theStream);
		std::stringstream ss;
		ss.str(valueStr.getString());
		const commonItems::singleString theString(ss);

		if (incomingKey == "configuration")
		{
			Log(LogLevel::Warning) << "You have an old configuration file. Preload will not be possible.";
			return;
		}
		for (const auto& [requiredFolderName, folderPtr]: requiredFolders)
		{
			if (requiredFolderName == incomingKey)
				folderPtr->setValue(theString.getString());
		}
		for (const auto& [requiredFileName, filePtr]: requiredFiles)
		{
			if (requiredFileName == incomingKey)
				filePtr->setValue(theString.getString());
		}
		for (const auto& option: options)
		{
			if (option->getName() == incomingKey && !option->getCheckBoxSelector().first)
			{
				option->setValue(theString.getString());
			}
			else if (option->getName() == incomingKey && option->getCheckBoxSelector().first)
			{
				commonItems::stringList theList(ss);
				const auto selections = theList.getStrings();
				auto values = std::set(selections.begin(), selections.end());
				option->setValue(values);
				option->setCheckBoxSelectorPreloaded();
			}
		}
		if (incomingKey == "selectedMods")
		{
			commonItems::stringList theList(ss);
			const auto selections = theList.getStrings();
			preloadedModFileNames.insert(selections.begin(), selections.end());
		}
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

Configuration::Configuration(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void Configuration::registerKeys()
{
	registerKeyword("name", [this](std::istream& theStream) {
		const commonItems::singleString nameStr(theStream);
		name = nameStr.getString();
	});
	registerKeyword("converterFolder", [this](std::istream& theStream) {
		converterFolder = commonItems::getString(theStream);
	});
	registerKeyword("backendExePath", [this](std::istream& theStream) {
		backendExePath = commonItems::getString(theStream);
	});
	registerKeyword("requiredFolder", [this](std::istream& theStream) {
		auto newFolder = std::make_shared<RequiredFolder>(theStream);
		if (!newFolder->getName().empty())
			requiredFolders.insert(std::pair(newFolder->getName(), newFolder));
		else
			Log(LogLevel::Error) << "Required Folder has no mandatory field: name!";
	});
	registerKeyword("requiredFile", [this](std::istream& theStream) {
		auto newFile = std::make_shared<RequiredFile>(theStream);
		if (!newFile->getName().empty())
			requiredFiles.insert(std::pair(newFile->getName(), newFile));
		else
			Log(LogLevel::Error) << "Required File has no mandatory field: name!";
	});
	registerKeyword("option", [this](std::istream& theStream) {
		optionCounter++;
		auto newOption = std::make_shared<Option>(theStream, optionCounter);
		options.emplace_back(newOption);
	});
	registerKeyword("displayName", [this](std::istream& theStream) {
		const commonItems::singleString nameStr(theStream);
		displayName = nameStr.getString();
	});
	registerKeyword("sourceGame", [this](std::istream& theStream) {
		const commonItems::singleString gameStr(theStream);
		sourceGame = gameStr.getString();
	});
	registerKeyword("targetGame", [this](std::istream& theStream) {
		const commonItems::singleString gameStr(theStream);
		targetGame = gameStr.getString();
	});
	registerKeyword("autoGenerateModsFrom", [this](std::istream& theStream) {
		const commonItems::singleString modsStr(theStream);
		autoGenerateModsFrom = modsStr.getString();
	});
	registerKeyword("enableUpdateChecker", [this](std::istream& theStream) {
		enableUpdateChecker = commonItems::getString(theStream) == "true";
	});
	registerKeyword("overwritePlayset", [this](std::istream& theStream) {
		overwritePlayset = commonItems::getString(theStream) == "true";
	});
	registerKeyword("checkForUpdatesOnStartup", [this](std::istream& theStream) {
		checkForUpdatesOnStartup = commonItems::getString(theStream) == "true";
	});
	registerKeyword("converterReleaseForumThread", [this](std::istream& theStream) {
		converterReleaseForumThread = commonItems::getString(theStream);
	});
	registerKeyword("latestGitHubConverterReleaseUrl", [this](std::istream& theStream) {
		latestGitHubConverterReleaseUrl = commonItems::getString(theStream);
	});
	registerKeyword("pagesCommitIdUrl", [this](std::istream& theStream) {
		pagesCommitIdUrl = commonItems::getString(theStream);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

bool Configuration::exportConfiguration() const
{
	if (converterFolder.empty())
	{
		Log(LogLevel::Error) << "Converter folder is not set!";
		return false;
	}
	if (!exists(converterFolder))
	{
		Log(LogLevel::Error) << "Could not find converter folder!";
		return false;
	}
	std::ofstream confFile(converterFolder / "configuration.txt");
	if (!confFile.is_open())
	{
		Log(LogLevel::Error) << "Could not open configuration.txt!";
		return false;
	}

	for (const auto& [requiredFolderName, folderPtr]: requiredFolders)
	{
		confFile << requiredFolderName << " = \"" << folderPtr->getValue().string() << "\"\n";
	}
	for (const auto& [requiredFileName, filePtr]: requiredFiles)
	{
		if (!filePtr->isOutputtable())
			continue;

		std::string fileValue;
		try
		{
			fileValue = filePtr->getValue().string();
		}
		catch (...)
		{
			// we ran into a case where C++ can't stringify the path, but can still use it. So copy the file to a safe name
			std::filesystem::path dir = filePtr->getValue().parent_path();
			auto extension = filePtr->getValue().extension();
			auto temp_file = dir / ("temp" + extension.string());
			std::filesystem::copy_file(filePtr->getValue(), temp_file, std::filesystem::copy_options::overwrite_existing);
			confFile << temp_file.string() << "\"\n";
			filePtr->setValue(temp_file);
			fileValue = filePtr->getValue().string();
		}

		confFile << requiredFileName << " = \"" << fileValue << "\"\n";
	}

	if (!autoGenerateModsFrom.empty())
	{
		confFile << "selectedMods = {\n";
		for (const auto& mod: autolocatedMods)
			if (preloadedModFileNames.count(mod.getFileName()))
			{
				confFile << "\t" << mod.getFileName() << "\n";
			}
		confFile << "}\n";
	}

	for (const auto& option: options)
	{
		if (option->getCheckBoxSelector().first)
		{
			confFile << option->getName() << " = { ";
			for (const auto& value: option->getValues())
			{
				confFile << "\"" << value << "\" ";
			}
			confFile << "}\n";
		}
		else
		{
			confFile << option->getName() << " = \"" << option->getValue() << "\"\n";
		}
	}
	confFile.close();
	return true;
}

path Configuration::getSecondTailSource() const
{
	return converterFolder / "log.txt";
}

void Configuration::clearSecondLog() const
{
	std::ofstream clearSecondLog(converterFolder / "log.txt");
	clearSecondLog.close();
}

void Configuration::autoLocateMods()
{
	autolocatedMods.clear();
	// Do we have a mod path?
	path modPath;
	for (const auto& filePtr: requiredFolders | std::views::values)
	{
		if (filePtr->getName() == autoGenerateModsFrom)
		{
			modPath = filePtr->getValue();
		}
	}
	if (modPath.empty())
		return;

	// Does it exist?
	if (!commonItems::DoesFolderExist(modPath))
	{
		Log(LogLevel::Warning) << "Mod path: " << modPath.string() << " does not exist or can not be accessed!";
		return;
	}

	// Are we looking at documents directory?
	if (commonItems::DoesFolderExist(modPath / "mod"))
		modPath /= "mod";

	// Are there mods inside?
	std::vector<path> validModFiles;
	for (const auto& file: commonItems::GetAllFilesInFolder(modPath))
	{
		if (file.extension() != ".mod")
			continue;
		validModFiles.emplace_back(file);
	}

	if (validModFiles.empty())
	{
		Log(LogLevel::Warning) << "No mod files could be found in " << modPath;
		return;
	}

	for (const auto& modFile: validModFiles)
	{
		FronterMod theMod(modPath / modFile);
		if (theMod.getName().empty())
		{
			Log(LogLevel::Warning) << "Mod at " << (modPath / modFile).string() << " has no defined name, skipping.";
			continue;
		}
		autolocatedMods.emplace_back(theMod);
	}

	// filter broken filenames from preloaded list.
	std::set<path> modNames;
	for (const auto& mod: autolocatedMods)
		modNames.insert(mod.getFileName());

	for (auto it = preloadedModFileNames.begin(); it != preloadedModFileNames.end();)
	{
		if (!modNames.count(*it))
		{
			it = preloadedModFileNames.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Configuration::setEnabledMods(const std::set<int>& selection)
{
	preloadedModFileNames.clear();
	for (auto counter = 0; counter < static_cast<int>(autolocatedMods.size()); counter++)
	{
		if (selection.count(counter))
		{
			preloadedModFileNames.insert(autolocatedMods[counter].getFileName());
		}
	}
}