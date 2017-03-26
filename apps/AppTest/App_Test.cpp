#include "TestApp.hpp"
#include <ANGL/Core/loggers/ConsoleLogger.hpp>

int main(int argc, char** argv) {


    ANGL::ConsoleLogger aLogger(true);
    ANGL::IApp* aApp = new TestApp();

    aApp->processArguments(argc, argv);
    int aExitCode = aApp->run();

    delete aApp;

    return aExitCode;
}
