#ifndef CONFIGURATION
#define CONFIGURATION
#include "Log.h"
#include "Options/Option.h"
#include "RequiredFile.h"
#include "RequiredFolder.h"
#include "Parser.h"

class Configuration: commonItems::parser
{
  public:
	Configuration();
	explicit Configuration(std::istream& theStream);

	[[nodiscard]] const auto& getSourceGame() const { return sourceGame; }
	[[nodiscard]] const auto& getTargetGame() const { return targetGame; }
	[[nodiscard]] const auto& getName() const { return name; }
	[[nodiscard]] const auto& getDisplayName() const { return displayName; }
	[[nodiscard]] const auto& getRequiredFiles() const { return requiredFiles; }
	[[nodiscard]] const auto& getRequiredFolders() const { return requiredFolders; }
	[[nodiscard]] const auto& getConverterFolder() const { return converterFolder; }
	[[nodiscard]] const auto& getOptions() const { return options; }

	[[nodiscard]] std::string getSecondTailSource() const;
	[[nodiscard]] bool exportConfiguration() const;
	[[nodiscard]] bool copyMod() const;

  private:
	void registerKeys();
	void registerPreloadKeys();
	std::string name;
	std::string converterFolder;
	std::string displayName;
	std::string sourceGame;
	std::string targetGame;
	std::map<std::string, std::shared_ptr<RequiredFile>> requiredFiles;
	std::map<std::string, std::shared_ptr<RequiredFolder>> requiredFolders;
	std::vector<std::shared_ptr<Option>> options;
	int optionCounter = 0;
};

#endif // CONFIGURATION
