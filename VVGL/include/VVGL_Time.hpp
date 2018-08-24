#ifndef VVGL_Time_h
#define VVGL_Time_h

#include "VVGL_Defines.hpp"

#include <math.h>
#include <chrono>
#include <iostream>




namespace VVGL
{


using namespace std;
using namespace chrono;


//! Struct describing a timevalue as the quotient of two numbers, a value (64-bit unsigned int) and a scale factor (32-bit int).
/*!
\ingroup VVGL_MISC
GLBuffers vended by the pool are timestamped so the stamps can be compared and used to determine if the content is different without analyzing the contents of the frame.
*/

using RawTSClockType = std::chrono::steady_clock;
using RawTSDuration = std::chrono::microseconds;
using RawTSTime = std::chrono::time_point<RawTSClockType, RawTSDuration>;

using DoubleTimeDuration = std::chrono::duration<double, std::ratio<1>>;
using DoubleTimeTime = std::chrono::time_point<RawTSClockType, DoubleTimeDuration>;

struct Timestamp	{
	
	
	//!	Creates a new Timestamp with the current time
	Timestamp()	{
		rawTime = time_point_cast<RawTSDuration>(RawTSClockType::now());
	}
	//!	Creates a new Timestamp with the passed time as a chrono::time_point< chrono::steady_clock, chrono::microseconds> >
	Timestamp(const RawTSTime & inRawTime)	{
		rawTime = inRawTime;
	}
	//!	Creates a new Timestamp with the passed time as a chrono::microseconds
	Timestamp(const RawTSDuration & inRawDuration)	{
		rawTime = RawTSTime(inRawDuration);
	}
	//!	Creates a new Timestamp with the passed time in seconds as a double
	Timestamp(const double & inTimeInSeconds)	{
		rawTime = time_point_cast<RawTSDuration>( DoubleTimeTime( DoubleTimeDuration(inTimeInSeconds) ) );
	}
	//!	Creates a new Timestamp expressed as the quotient of the two passed numbers (the number of frames divided by the number of seconds in which they occur)
	Timestamp(const int & inFrameCount, const int & inSecondsCount)	{
		rawTime = time_point_cast<RawTSDuration>( DoubleTimeTime( DoubleTimeDuration( double(inFrameCount)/double(inSecondsCount) ) ) );
	}

	
	//!	Calculates the frame index of the receiver with the passed FPS, expressed as the quotient of two integers.
	inline double frameIndexForFPS(const int & inFrameCount, const int & inSecondsCount)	{
		RawTSTime		tmpTime(RawTSDuration(rawTime.time_since_epoch().count() * inFrameCount / inSecondsCount));
		return std::chrono::time_point_cast<DoubleTimeDuration>(tmpTime).time_since_epoch().count();
	}
	//!	Calculates the frame index of the receiver with the passed FPS.
	inline double frameIndexForFPS(const double & inFPS)	{
		return frameIndexForFPS(int(inFPS * 1000000), 1000000);
	}
	//!	Calls 'frameIndexForFPS()' and then rounds the result before returning it
	inline int64_t nearestFrameIndexForFPS(const int & inFrameCount, const int & inSecondsCount)	{
		return int64_t(round(frameIndexForFPS(inFrameCount, inSecondsCount)));
	}
	//!	Calls 'frameIndexForFPS()' and then rounds the result before returning it
	inline int64_t nearestFrameIndexForFPS(const double & inFPS)	{
		return nearestFrameIndexForFPS(int(inFPS * 1000000), 1000000);
	}
	
	
	//!	Calculates and returns the receivers time in seconds, expressed as a double.
	inline double getTimeInSeconds() const	{
		return std::chrono::time_point_cast<DoubleTimeDuration>(rawTime).time_since_epoch().count();
	}
	
	
	friend inline ostream & operator<<(ostream & os, const Timestamp & rs)	{
		os << "<Timestamp " << rs.getTimeInSeconds() << ">";
		return os;
	}
	inline Timestamp operator-(const Timestamp & n) const	{
		return Timestamp(RawTSDuration(this->rawTime.time_since_epoch().count() - n.rawTime.time_since_epoch().count()));
	}
	inline Timestamp operator+(const Timestamp & n) const	{
		return Timestamp(RawTSDuration(this->rawTime.time_since_epoch().count() + n.rawTime.time_since_epoch().count()));
	}
	inline bool operator==(const Timestamp & n) const	{
		return (this->rawTime == n.rawTime);
	}
	inline bool operator<(const Timestamp & n) const	{
		return (this->rawTime < n.rawTime);
	}
	inline bool operator>(const Timestamp & n) const	{
		return (this->rawTime > n.rawTime);
	}
	
private:
	RawTSTime		rawTime;
};




}


#endif /* VVGL_Time_h */
