#ifndef VVGL_Time_h
#define VVGL_Time_h

#include "VVGL_Defines.hpp"

#include <math.h>
#include <chrono>
#include <iostream>




namespace VVGL
{


using namespace std;



//! Struct describing a timevalue as the quotient of two numbers, a value (64-bit unsigned int) and a scale factor (32-bit int).
/*!
\ingroup VVGL_MISC
GLBuffers vended by the pool are timestamped so the stamps can be compared and used to determine if the content is different without analyzing the contents of the frame.
*/
struct Timestamp	{
	uint64_t	value=0;
	uint32_t	scale=600;
	
	inline Timestamp() { value=0; scale=600; }
	//!	Creates a Timestamp object populated with the passed time value and scale.
	inline Timestamp(const uint64_t & inValue, const uint32_t & inScale=600) { value=inValue;scale=inScale; }
	//!	Creates a Timestamp object populated with the passed time in seconds and scale.
	inline Timestamp(const double & inTimeSec, const uint32_t & inScale=600) { value=uint64_t(floor(inTimeSec*static_cast<double>(inScale))); scale=inScale; }
	
	//!	Returns the time value in seconds as a double.
	inline double getTimeInSeconds() const { return double(value)/double(scale); }
	friend inline ostream & operator<<(ostream & os, const Timestamp & rs) { os<<"<Timestamp "<<rs.getTimeInSeconds()<<">"; return os; }
	inline bool operator==(const Timestamp & n) const { return (this->value==n.value && this->scale==n.scale); }
	inline bool operator<(const Timestamp & n) const { return (this->getTimeInSeconds() < n.getTimeInSeconds()); }
	inline bool operator>(const Timestamp & n) const { return (this->getTimeInSeconds() > n.getTimeInSeconds()); }
};


//! Class that vends Timestamp structs
/*!
\ingroup VVGL_MISC
GLBuffers vended by the pool are timestamped so the stamps can be compared and used to determine if the content is different without analyzing the contents of the frame.
*/
class Timestamper	{
	private:
		chrono::time_point<chrono::high_resolution_clock>		stampStartTime;
		uint32_t		timescale = 600;
	public:
		Timestamper(const Timestamper & n) = default;
		Timestamper() { reset(); }
		//!	Sets the timescale of the timestamps vended by the receiver.
		inline void setTimescale(const uint32_t & n) { timescale=n; }
		//!	Returns a Time
		inline Timestamp nowTime() const { using namespace chrono; return Timestamp(static_cast<uint64_t>(duration_cast<microseconds>(high_resolution_clock::now()-stampStartTime).count())/(1000000/timescale), timescale); }
		inline void reset() { stampStartTime = chrono::high_resolution_clock::now(); }
};




}


#endif /* VVGL_Time_h */
