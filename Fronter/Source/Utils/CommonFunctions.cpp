#include "CommonFunctions.h"
#include "../../../commonItems/CommonFunctions.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
#include <codecvt>
#include <filesystem>
namespace fs = std::filesystem;

LogMessage sliceMessage(const std::string& message)
{
	LogMessage logMessage;
	const auto posOpen = message.find_first_of('[');
	if (!(posOpen >= 20 && posOpen <= 23))
	{
		// This is not a standard message. File as info ad verbatim.
		logMessage.message = message;
		return logMessage;
	}
	const auto posClose = message.find_first_of(']');
	if (posClose == std::string::npos)
	{
		// something's very wrong with this message.
		logMessage.message = message;
		return logMessage;
	}
	const auto logLevel = message.substr(posOpen + 1, posClose - posOpen - 1);
	if (logLevel == "INFO")
		logMessage.logLevel = LogLevel::Info;
	else if (logLevel == "DEBUG")
		logMessage.logLevel = LogLevel::Debug;
	else if (logLevel == "WARNING")
		logMessage.logLevel = LogLevel::Warning;
	else if (logLevel == "ERROR")
		logMessage.logLevel = LogLevel::Error;
	logMessage.timestamp = message.substr(0, 19);
	logMessage.message = message.substr(posClose + 2, message.length());
	return logMessage;
}

std::string normalizeStringPath(const std::string& stringPath)
{
	std::string toReturn = Utils::normalizeUTF8Path(stringPath);
	toReturn = replaceCharacter(toReturn, '-');
	toReturn = replaceCharacter(toReturn, ' ');

	return toReturn;
}
