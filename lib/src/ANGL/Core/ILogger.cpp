#include <ANGL/Core/ILogger.hpp>

#include <ANGL/Core/IApp.hpp>

#include <ctime>
#include <sstream>

namespace ANGL
{
    ILogger* ILogger::gInstance = NULL;

    // TODO : Implement onnullstream
    //ILogger::_nullStream;

    ILogger::ILogger(bool pMakeDefault, int pExitCode) :
        mActive(false),
        mExitCode(pExitCode)
    {

        if(pMakeDefault) {
            gInstance = this;
        }

        setActive(true);
    }

    ILogger::~ILogger()
    {
        setActive(false);

        if(this == gInstance) {
            gInstance = NULL;
        }
    }

    ILogger *ILogger::getLogger()
    {
        // If there is no logger instantiated, instantiate one at the last moment
        if(NULL == gInstance) {
            ILogger::getLogger()->getStream(SeverityFatal, "ILogger.cpp", __LINE__)
                << "ILogger::getArguments() : no logger has been instantiated" << std::endl;
        }

        return gInstance;
    }

    bool ILogger::isActive()
    {
        return mActive;
    }

    void ILogger::setActive(bool pActive)
    {
        mActive = pActive;
    }


    void ILogger::writeTag(std::ostream &pOstream, SeverityLevel pSeverityLevel, std::string pSourceFile,
                           int pSourceLine)
    {
        std::time_t  aNow;
        std::tm* aTm;

        std::ostringstream aTimestamp, aTag;
        char logLevel;

        // Get current time (seconds since January 1st 1970)
        aNow = std::time(NULL);

        // Convert current time into local time
        aTm = std::localtime(&aNow);


        // Construct timestamp
        // Year
        aTimestamp << aTm->tm_year + 1900 << "-";
        // Month
        aTimestamp.fill('0');
        aTimestamp.width(2);
        aTimestamp << aTm->tm_mon + 1 << "-";
        // Day
        aTimestamp.fill('0');
        aTimestamp.width(2);
        aTimestamp << aTm->tm_mday << " ";

        // Hour
        aTimestamp.fill('0');
        aTimestamp.width(2);
        aTimestamp << aTm->tm_hour + 1 << ":";
        // Minutes
        aTimestamp.fill('0');
        aTimestamp.width(2);
        aTimestamp << aTm->tm_min << ":";
        // Seconds
        aTimestamp.fill('0');
        aTimestamp.width(2);
        aTimestamp << aTm->tm_sec << " ";


        // Construct the File:Line tag
        aTag << pSourceFile << ":" << pSourceLine;


        // Now print the log level as a single character
        switch(pSeverityLevel)
        {
            case SeverityDebug:
                logLevel = 'D';
                break;
            case SeverityInfo:
                logLevel = 'I';
                break;
            case SeverityWarning:
                logLevel = 'W';
                break;
            case SeverityError:
                logLevel = 'E';
                break;
            case SeverityFatal:
                logLevel = 'F';
                break;
            default:
            case SeverityNoLog:
                logLevel = '-';
                break;
        }

        // Output the timestamp and File:Line tag into the ostream
        pOstream << aTimestamp.str() << " " << logLevel << " " << aTag.str() << " ";
    }

    void ILogger::fatalShutDown()
    {
        if(NULL != IApp::getApp())
        {
            IApp::getApp()->quit(mExitCode);
        }
        else
        {
            exit(mExitCode);
        }
    }
} // namespace ANGL
