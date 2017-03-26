/**
 * @file Provides the implementation for the clock.
 * The clock is responsible for keeping track of elapsed time from a given moment
 * Clock relies on glfwGetTime through the Time::getCurrentTime() method
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core/utililty/Time.hpp>

namespace ANGL {

    class Clock {

    public:

        /**
         * Clock constructor
         */
        Clock();

        /**
         * Clock de-constructor
         */
        ~Clock();

        /**
         *
         */
        Time getElapsedTime() const;

        /**
         *
         */
        Time restart();

    private:

        Time mStartTime;

    }; // class Clock
} // namespace ANGL
