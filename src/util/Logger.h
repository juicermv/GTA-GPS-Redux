#pragma once
#include <fstream>
#include <chrono>

namespace util
{
	class Logger
	{
	private:
		bool enabled = false;
		char stime[128] = "";
		unsigned short logLines = 0;
		std::ofstream logfile;

	public:
		void Log(const char *val);
		Logger(bool enabled);
	};
}