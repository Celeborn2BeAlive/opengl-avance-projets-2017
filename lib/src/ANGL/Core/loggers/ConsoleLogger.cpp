#include <ANGL/Core/loggers/ConsoleLogger.hpp>

#include <iostream>

namespace ANGL {

    void setSeverityColor(SeverityLevel pSeverityLevel)
    {
        switch (pSeverityLevel) {

            case SeverityDebug :
                std::cout << "\033[" << ConsoleLogger::TextColor::lightBlue << "m";
                break;
            case SeverityInfo :
                std::cout << "\033[" << ConsoleLogger::TextColor::lightGreen << "m";
                break;
            case SeverityWarning :
                std::cout << "\033[" << ConsoleLogger::TextColor::yellow << "m";
                break;
            case SeverityError :
                std::cout << "\033[" << ConsoleLogger::TextColor::lightRed << "m";
                break;
            case SeverityFatal :
                std::cout << "\033[" << ConsoleLogger::TextColor::lightRed << "m";
                break;
            default:
            case SeverityNoLog :
                break;
        }
    }

    void resetSeverityColor()
    {
        std::cout << "\033[" << ConsoleLogger::TextColor::Default << "m";
    }


    ConsoleLogger::ConsoleLogger(bool pMakeDefault, int pExitCode) :
        ILogger(pMakeDefault, pExitCode)
    {
        logMessage( "ConsoleLogger::ctor()", SeverityInfo, "ConsoleLogger.cpp", __LINE__);
    }

    ConsoleLogger::~ConsoleLogger()
    {
        logMessage( "ConsoleLogger::dtor()", SeverityInfo, "ConsoleLogger.cpp", __LINE__);
        resetSeverityColor();
    }


    std::ostream &ConsoleLogger::getStream()
    {
        return std::cout;
    }

    std::ostream &ConsoleLogger::getStream(SeverityLevel pSeverityLevel, std::string pSourceFile, int pSourceLine,
                                           int pExitCode)
    {
        setSeverityColor(pSeverityLevel);
        writeTag(std::cout, pSeverityLevel, pSourceFile, pSourceLine);
        resetSeverityColor();

        return std::cout;
    }

    void ConsoleLogger::logMessage(const std::string &pMessage)
    {
        if(isActive()) {
            std::cout << pMessage << std::endl;
        }
    }

    void ConsoleLogger::logMessage(const std::string &pMessage, SeverityLevel pSeverityLevel, std::string pSourceFile,
                                   int pSourceLine)
    {
        if(isActive()) {

            setSeverityColor(pSeverityLevel);
            writeTag(std::cout, pSeverityLevel, pSourceFile, pSourceLine);
            resetSeverityColor();

            std::cout << pMessage << std::endl;

            // Exit the application if we encountered a fatal error
            if(SeverityFatal == pSeverityLevel) {
                fatalShutDown();
            }
        }
    }

} // namespace ANGL
