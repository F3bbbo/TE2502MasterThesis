#pragma once
#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <iostream>

#define NOTICE 0
#define WARNING 1
#define CRITICAL 2

class Log
{
public:
	template <typename T>
	static void log(int type, const char* file, long line , T message)
	{
		std::string str = std::string(file);
		str = str.substr(str.rfind('\\') + 1);
		str += ": (" + to_string(type) + ") on line " + std::to_string(line) + ": " + message + "\n \n";
		std::cout << str;
		switch (type)
		{
			case NOTICE :
			{
				noticeLog += str;
			}
			case WARNING :
			{
				warningLog += str;
			}
			case CRITICAL :
			{
				criticalLog += str;
			}
		}
	}
private:
	static std::string to_string(int type);
	static std::string noticeLog;
	static std::string warningLog;
	static std::string criticalLog;
};

#define LOG_T(TYPE, STRING) ( (TYPE >= NOTICE && TYPE <= CRITICAL) ? Log::log(TYPE, __FILE__, __LINE__, STRING) : void(0))
#define LOG(STRING) (Log::log(NOTICE, __FILE__, __LINE__, STRING))

#endif