#include <ANGL/Core/utililty/Clock.hpp>

namespace ANGL {

    Clock::Clock() :
        mStartTime(Time::getCurrentTime())
    {
    }

    Clock::~Clock()
    {
    }

    Time Clock::getElapsedTime() const {
        return Time::getCurrentTime() - mStartTime;
    }

    Time Clock::restart() {
        Time now = Time::getCurrentTime();
        Time elapsed = now - mStartTime;
        mStartTime = now;

        return elapsed;
    }
} // namespace ANGL
