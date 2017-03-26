/**
 * @file Provides the interface for a logger.
 * A logger is responsible for handling, storing or displaying logs messages returned by an application.
 *
 * ILogger implements the Singleton pattern.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core/CoreTypes.hpp>

#include <string>
#include <ostream>

namespace ANGL {

    class ILogger {

    public:
        // TODO : Implement onnullstream
        //static std::onnullstream _nullStream;

        /**
         *  ILogger deconstructor
         */
        ~ILogger();


        /**
         *  @return {ILogger*} a pointer to the most recently instantiated logger
         */
        static ILogger* getLogger();

        /**
         *  @return {bool} true if the logger is active, false if not
         */
        bool isActive();

        /**
         * Sets if the logger is active or not
         *
         *  @param {bool} pActive Set to true to activate the logger, false to deactivate
         */
        void setActive(bool pActive);

        /**
         *  @return {std::ostream&} a reference to the ostream where the logger sends the messages that are given to him
         */
        virtual std::ostream& getStream() = 0;

        /**
         * Returns a reference to the stream and inserts a timestamp and File:Line tag inside
         *
         * @return {std::ostream&} a reference to the ostream where the logger sends the messages that are given to him
         */
        virtual std::ostream& getStream(SeverityLevel pSeverityLevel,
                                        std::string pSourceFile, int pSourceLine,
                                        int pExitCode = StatusError) = 0;


        /**
         * Logs the provided message
         *
         * @param {std::string} pMessage The message to log
         */
        virtual void logMessage(const std::string &pMessage) =0;

        /**
         * Logs the provided message with a timestamp and File:Line tag in front
         *
         * @param {std::string} pMessage The message to log
         * @param {SeverityLevel} pSeverityLevel The severity level for the message to log
         * @param {std::string} pSourceFile The source file where the logger has been called from
         * @param {int} pSourceLine The line number where the logger has been called from
         */
        virtual void logMessage(const std::string &pMessage, SeverityLevel pSeverityLevel,
                                std::string pSourceFile, int pSourceLine) = 0;

    protected:
        /**
         * ILogger constructor
         * Is protected because we only allow derived classes to instantiate this interface

         * @param {bool} pMakeDefault True if the new logger must become the default logger, false if not
         * @param {int} pExitCode The exit code to return if the application has to shutdown
         */
        ILogger(bool pMakeDefault, int pExitCode = StatusError);

        /**
         * Write a timestamp and File:Line tag to the provided ostream
         * @param {std::ostream} pOstream The ostream where to write the prefix tag
         * @param {SeverityLevel} pSeverityLevel The severity level for the message to log
         * @param {std::string} pSourceFile The source file where the logger has been called from
         * @param {int} pSourceLine The line number where the logger has been called from
         */
        void writeTag(std::ostream &pOstream, SeverityLevel pSeverityLevel,
                      std::string pSourceFile, int pSourceLine);

        /**
         * Makes the application stop with the appropriate exit code.
         * This method should be only be called on a fatal severity level.
         */
        void fatalShutDown();

    private:
        static ILogger* gInstance;

        bool mActive;
        int mExitCode;

        /**
         * ILogger copy constructor
         * is private because we do not allow copies of a Singleton
         */
        ILogger(const ILogger&);  // Intentionally undefined

        /**
         * ILogger assignment operator
         * is private because we do not allow copies of a Singleton
         */
        ILogger& operator=(const ILogger&); // Intentionally undefined
    }; // class ILogger
} // namespace ANGL
