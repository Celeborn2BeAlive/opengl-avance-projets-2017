#include <ANGL/Core/utililty/Time.hpp>

namespace ANGL {

    Time::Time()
    {
    }

    Time::Time(double pSeconds) :
        mSeconds(pSeconds)
    {
    }

    Time::~Time()
    {
    }


    Time Time::getCurrentTime() {
        return Time(glfwGetTime());
    }

    double Time::asSeconds() const
    {
        return mSeconds;
    }

    double Time::asMilliseconds() const
    {
        return mSeconds * 1000.f;
    }


    bool Time::operator==(const Time& rhs) const
    {
        return this->mSeconds == rhs.mSeconds;
    }

    bool Time::operator!=(const Time& rhs) const
    {
        return this->mSeconds != rhs.mSeconds;
    }

    bool Time::operator<(const Time& rhs) const
    {
        return this->mSeconds < rhs.mSeconds;
    }

    bool Time::operator>(const Time& rhs) const
    {
        return this->mSeconds > rhs.mSeconds;
    }

    bool Time::operator<=(const Time& rhs) const
    {
        return this->mSeconds <= rhs.mSeconds;
    }

    bool Time::operator>=(const Time& rhs) const
    {
        return this->mSeconds >= rhs.mSeconds;
    }

    Time& Time::operator+=(const Time& rhs)
    {
        this->mSeconds += rhs.mSeconds;
        return *this;
    }

    Time Time::operator+(const Time& rhs) const
    {
        Time result(*this);
        result += rhs;
        return result;
    }

    Time& Time::operator-=(const Time& rhs)
    {
        this->mSeconds -= rhs.mSeconds;
        return *this;
    }

    Time Time::operator-(const Time& rhs) const
    {
        Time result(*this);
        result -= rhs;
        return result;
    }

    Time& Time::operator=(const double& rhs)
    {
        this->mSeconds = rhs;
        return *this;
    }

    Time& Time::operator=(const Time &rhs)
    {
        this->mSeconds = rhs.mSeconds;
        return *this;
    }
} // namespace ANGL