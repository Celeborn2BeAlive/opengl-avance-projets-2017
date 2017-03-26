/**
 * @file Provides the Core typedefs, enums and forward class declarations.
 */

#pragma once

namespace ANGL {

    // TODO Move this enum to ILogger ?
    // Enumeration of all Logging severity levels
    enum SeverityLevel
    {
        // Logger severity levels range from 0 to 4
        SeverityDebug = 0,      // Debug severity level
        SeverityInfo,           // Information severity level
        SeverityWarning,        // Warning severity type
        SeverityError,          // Error severity type
        SeverityFatal,          // Fatal severity type
        SeverityNoLog           // No logs
    };

    // TODO Move this enum to IApp ?
    // Status Enumeration for Status Return values
    enum StatusType
    {
        StatusAppMissingAsset = -4,  // Application failed due to missing asset file
        StatusAppStackEmpty   = -3,  // Application States stack is empty
        StatusAppInitFailed   = -2,  // Application initialization failed
        StatusError           = -1,  // General error status response
        StatusAppOK           =  0,  // Application quit without error
        StatusNoError         =  0,  // General no error status response
        StatusFalse           =  0,  // False status response
        StatusTrue            =  1,  // True status response
        StatusOK              =  1   // OK status response
    };

    // Forward declarations
    class ILogger;
    class IApp;
    class IState;
    class StateManager;
    class WindowManager;

    class Clock;
    class Time;
} // namespace ANGL