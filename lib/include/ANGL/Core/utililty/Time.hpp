/**
 * @file Provides the implementation for a time
 * A duration is responsible for storing a time in milliseconds
 * Its main purpose is to handle time and clock management using the underlying GLFW library (glfwGetTime())
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>

namespace ANGL {

    class Time {

    public:

        /**
         * Time constructor
         * sets the duration at 0 milliseconds
         */
        Time();

        /**
         * Time constructor
         *
         * @param {Uint32} pMilliseconds Duration in milliseconds
         */
        Time(double pSeconds);
        
        /**
         * Time de-constructor
         */
        ~Time();


        /**
         * @return {Time} the duration in milliseconds since SDL was initialized
         */
        static Time getCurrentTime();

        /**
         * @return {float} the time as seconds
         */
        double asSeconds() const;

        /**
         * @return {float} the time as milliseconds
         */
        double asMilliseconds() const;


        // Operators overloads
        bool operator==(const Time& rhs) const;
        bool operator!=(const Time& rhs) const;

        bool operator<(const Time& rhs) const;
        bool operator>(const Time& rhs) const;
        bool operator<=(const Time& rhs) const;
        bool operator>=(const Time& rhs) const;

        Time& operator+=(const Time& rhs);
        Time operator+(const Time& rhs) const;
        Time& operator-=(const Time& rhs);
        Time operator-(const Time& rhs) const;

        Time& operator=(const double& rhs);
        Time& operator=(const Time& rhs);

    private:
        double mSeconds;
    }; // class Time

} // namespace ANGL