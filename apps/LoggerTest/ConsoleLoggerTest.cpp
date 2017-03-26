#include <ANGL/Core/loggers/ConsoleLogger.hpp>

int main() {

    ANGL::ConsoleLogger aLogger(true);

    ANGL::ILogger::getLogger()->logMessage("Test Debug", ANGL::SeverityDebug, "ConsoleLoggerTest.cpp", __LINE__);
    ANGL::ILogger::getLogger()->logMessage("Test Info", ANGL::SeverityInfo, "ConsoleLoggerTest.cpp", __LINE__);
    ANGL::ILogger::getLogger()->logMessage("Test Warning", ANGL::SeverityWarning, "ConsoleLoggerTest.cpp", __LINE__);
    ANGL::ILogger::getLogger()->logMessage("Test Error", ANGL::SeverityError, "ConsoleLoggerTest.cpp", __LINE__);
    ANGL::ILogger::getLogger()->logMessage("Test Fatal", ANGL::SeverityFatal, "ConsoleLoggerTest.cpp", __LINE__);

    return 0;
}

