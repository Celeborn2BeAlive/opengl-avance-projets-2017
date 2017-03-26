/**
 * @file Provides the implementation for a console logger.
 * A console logger responsibility is to output log messages to the console.
 * The ConsoleLogger implements the Singleton pattern.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Core/CoreTypes.hpp>
#include <ANGL/Core/ILogger.hpp>

namespace ANGL {

    class ConsoleLogger : public ILogger {

    public:
        enum TextColor {
            Default = 39,
            black = 30,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            lightGray,
            darkGray = 90,
            lightRed,
            lightGreen,
            lightYellow,
            lightBlue,
            lightMagenta,
            lightCyan,
            white
        };


        /**
         *  ConsoleLogger constructor
         *
         * @param {bool} pMakeDefault True if the new logger must become the default logger, false if not
         * @param {int} pExitCode The exit code to return if the application has to shutdown
         */
        ConsoleLogger(bool pMakeDefault, int pExitCode = StatusError);

        /**
         * ConsoleLogger deconstructor
         */
        ~ConsoleLogger();


        /**
         *  @return {std::ostream&} a reference to the ostream where the logger sends the messages that are given to him
         */
        virtual std::ostream& getStream();

        /**
         * Returns a reference to the stream and inserts a timestamp and File:Line tag inside
         *
         * @return {std::ostream&} a reference to the ostream where the logger sends the messages that are given to him
         */
        virtual std::ostream& getStream(SeverityLevel pSeverityLevel,
                                        std::string pSourceFile, int pSourceLine,
                                        int pExitCode = StatusError);


        /**
         * Logs the provided message
         *
         * @param {std::string} pMessage The message to log
         */
        virtual void logMessage(const std::string &pMessage);

        /**
         * Logs the provided message with a timestamp and File:Line tag in front
         *
         * @param {std::string} pMessage The message to log
         * @param {SeverityLevel} pSeverityLevel The severity level for the message to log
         * @param {std::string} pSourceFile The source file where the logger has been called from
         * @param {int} pSourceLine The line number where the logger has been called from
         */
        virtual void logMessage(const std::string &pMessage, SeverityLevel pSeverityLevel,
                                std::string pSourceFile, int pSourceLine);

    protected:

    private:
        /**
         * ConsoleLogger copy constructor
         * is private because we do not allow copies of a Singleton
         */
        ConsoleLogger(const ConsoleLogger&);  // Intentionally undefined

        /**
         * ConsoleLogger assignment operator
         * is private because we do not allow copies of a Singleton
         */
        ConsoleLogger& operator=(const ConsoleLogger&); // Intentionally undefined

    }; // class ConsoleLogger

} // namespace ANGL
