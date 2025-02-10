#include "Logger.h"

namespace util
{
    void Logger::Log(const char *val)
    {
        if (!enabled)
            return;

        if (logLines < 2048)
        {
            time_t timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            strftime(stime, 128, "%c", localtime(&timenow));
            logfile << (std::string(stime) + " | " + val + "\n").c_str();
            logfile.flush();
            logLines++;
        }
        else
        {
            logfile.close();
            logfile.open("SA.GPS.LOG.txt", std::ios::out);
            logLines = 0;
            Log(val);
        }
    }

    Logger::Logger(bool enabled)
    {
        this->enabled = enabled;
        if (enabled)
        {
            logfile.open("SA.GPS.LOG.txt", std::ios::out);
        }
    }
}