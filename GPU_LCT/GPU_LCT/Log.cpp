#include "Log.hpp"

std::string Log::noticeLog = "";
std::string Log::warningLog = "";
std::string Log::criticalLog = "";

std::string Log::to_string(int type)
{
	switch (type)
		{
		case NOTICE   : return "Notice";
		case WARNING  : return "Warning";
		case CRITICAL : return "Critical";
		default       : return "";
		}
}
