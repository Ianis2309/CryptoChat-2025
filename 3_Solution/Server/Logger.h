#pragma once
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <map>
#include <shared_mutex>

class Logger
{
public:
	enum class Level {
		INFO,
		WARNING,
		ERROR
	};

	static void Info(const std::string& message);
	static void Warning(const std::string& message);
	static void Error(const std::string& message);

	static void SetFileLog(const std::string& filename);
	static void ClsFileLog();

	void writeToFile(const std::string&, const std::string&, const std::string&);
	std::string ReadFromFile(const std::string& filename);


private:
	std::shared_ptr<std::mutex> getMutexForFile(const std::string& filename);
	static void Log(const std::string& message, Level level);
	static std::ofstream logfile;
	static std::mutex mtxlog;

	static std::map<std::string, std::shared_ptr<std::mutex>> fileMutexes;
	static std::shared_mutex mutexMapMutex;
};

