#include <iostream>
#include <fstream>
#include <array>
#include <memory>

class FileUtility
{
public:
	static std::string ReadFromFile(const std::string& fileName);
	static bool WriteToFile(const std::string& fileName, const std::string dataForWrite);
};

std::string FileUtility::ReadFromFile(const std::string& fileName)
{
	std::ifstream file;
	std::string result = "";
	try
	{
		file.open(fileName.c_str());
		if (file.is_open())
		{
			file.seekg(0, std::ios::end);
			const auto size = file.tellg();
			std::unique_ptr<char[]> buffer(new char[size]);
			file.seekg(0, std::ios::beg);
			file.read(buffer.get(), size);
			result.assign(buffer.get());
			file.close();
		}
		else
		{
			LOG_ERROR("Error open file: " + fileName + " for read.");
		}
	}
	catch (std::exception &e)
	{
		LOG_ERROR("ReadFile error: " + static_cast<std::string>(e.what()));
	}
	catch (...)
	{
		LOG_ERROR("Read file error: Unknown error");
	}
	return result;
}


bool FileUtility::WriteToFile(const std::string& fileName, const std::string dataForWrite)
{
	std::ofstream file;
	bool result = false;
	try
	{
		file.open(fileName.c_str());
		if (file.is_open())
		{
			file << dataForWrite;
			file.close();
			result = true;
		}
		else
		{
			LOG_ERROR("Error open file: " + fileName + " for write.");
		}
	}
	catch (std::exception &e)
	{
		LOG_ERROR("WriteFile error: " + static_cast<std::string>(e.what()));
	}
	catch (...)
	{
		LOG_ERROR("Write file error: Unknown error");
	}

	return result;
}


class ProcessManager
{
public:
	static std::string Execute(const std::string& program/*, const std::string& args = ""*/);
	static void Restart();
	static void PowerOff();
};

std::string ProcessManager::Execute(const std::string& command)
{
	std::string result("");
	std::array<char, 256> buffer;
	try
	{
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
		if (pipe)
		{
			while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
			{
				result += buffer.data();
			}
		}
		else
		{
			throw std::system_error(errno, std::generic_category());
		}
	}
	catch (std::exception &e)
	{
		LOG_ERROR("exec error: " + static_cast<std::string>(e.what()));
	}
	catch (...)
	{
		LOG_ERROR("exec error: Unknown error");
	}
	return result;
}

void ProcessManager::Restart()
{
	std::stringstream settingsStream;
	const std::string settings{ "/sbin/reboot" };
	settingsStream << settings;
	const std::string result = Execute(settingsStream.str());
}

void ProcessManager::PowerOff()
{
	std::stringstream settingsStream;
	const std::string settings{ "/sbin/poweroff -p" };
	settingsStream << settings;
	const std::string result = Execute(settingsStream.str());
}
