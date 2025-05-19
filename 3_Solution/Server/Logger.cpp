#include "Logger.h"

#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

ofstream Logger::logfile;
mutex Logger::mtxlog;
map<string, shared_ptr<mutex>> Logger::fileMutexes;
shared_mutex Logger::mutexMapMutex;

void Logger::Info(const std::string& message)
{
	Log(message, Level::INFO);
}

void Logger::Warning(const std::string& message)
{
	Log(message, Level::WARNING);
}

void Logger::Error(const std::string& message)
{
	Log(message, Level::ERROR);
}

void Logger::SetFileLog(const std::string& filename)
{
	std::lock_guard<std::mutex> lock(mtxlog);
	if (logfile.is_open() == false) {
		logfile.open(filename, std::ios::out | std::ios::app);
	}
}

void Logger::ClsFileLog()
{
	std::lock_guard<std::mutex> lock(mtxlog);
	if (logfile.is_open()) {
		logfile.close();
	}
}

void Logger::writeToFile(const string& filename, const string& message, const string& src)
{
	shared_ptr<mutex> fileMutex = getMutexForFile(filename);
	lock_guard<mutex> lock(*fileMutex);

	ostringstream ss;
	time_t t = std::time(nullptr);
	tm tm;
	localtime_s(&tm, &t);
	ss << "[" << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "] ";
	ss << src << " " << message << static_cast<char>(0xff) << "\n";

	std::ofstream out(filename, std::ios::app);
	out << ss.str();
	out.flush();
}

std::string Logger::ReadFromFile(const std::string& filename)
{
	std::shared_ptr<std::mutex> fileMutex = getMutexForFile(filename);
	std::lock_guard<std::mutex> lock(*fileMutex);

	std::ifstream in(filename);

	std::ostringstream contents;
	contents << in.rdbuf();
	return contents.str();
}


shared_ptr<mutex> Logger::getMutexForFile(const string& filename)
{
	std::shared_lock<std::shared_mutex> readLock(mutexMapMutex);
	auto it = fileMutexes.find(filename);
	if (it != fileMutexes.end())
		return it->second;
	readLock.unlock();

	std::lock_guard<std::shared_mutex> writeLock(mutexMapMutex);
	auto& mutexPtr = fileMutexes[filename];
	if (!mutexPtr)
		mutexPtr = std::make_shared<std::mutex>();
	return mutexPtr;
}

void Logger::Log(const std::string& message, Level level)
{
	std::lock_guard<std::mutex> lock(mtxlog);

	std::ostringstream ss;

	std::time_t t = std::time(nullptr);
	std::tm tm;
	localtime_s(&tm, &t);

	ss << "[" << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "] ";

	switch (level) {
	case Level::INFO:    ss << "[INFO] "; break;
	case Level::WARNING: ss << "[WARN] "; break;
	case Level::ERROR:   ss << "[ERROR] "; break;
	}

	ss << message << "\n";

	// Consola
	std::cerr << ss.str();

	// File
	if (logfile.is_open()) {
		logfile << ss.str();
		logfile.flush();
	}
}
