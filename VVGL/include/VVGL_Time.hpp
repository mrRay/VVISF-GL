#ifndef VVGL_Time_h
#define VVGL_Time_h

#include "VVGL_Defines.hpp"

#include <math.h>
#include <chrono>
#include <iostream>




namespace VVGL
{


using namespace std;



/*!
\ingroup VVGL_MISC
\brief Struct describing a timevalue as the quotient of two numbers, a value (64-bit unsigned int) and a scale factor (32-bit int).

\detail GLBuffers vended by the pool are timestamped so the stamps can be compared and used to determine if the content is different without analyzing the contents of the frame
*/
struct Timestamp	{
	uint64_t	value=0;
	uint32_t	scale=600;
	
	inline Timestamp() { value=0; scale=600; };
	//!	Creates a Timestamp object populated with the passed time value and scale.
	inline Timestamp(const uint64_t & inValue, const uint32_t & inScale=600) { value=inValue;scale=inScale; };
	//!	Creates a Timestamp object populated with the passed time in seconds and scale.
	inline Timestamp(const double & inTimeSec, const uint32_t & inScale=600) { value=uint64_t(floor(inTimeSec*(double)inScale)); scale=inScale; };
	
	//!	Returns the time value in seconds as a double.
	inline double getTimeInSeconds() const { return double(value)/double(scale); };
	friend inline ostream & operator<<(ostream & os, const Timestamp & rs) { os<<"<Timestamp "<<rs.getTimeInSeconds()<<">"; return os; };
	inline bool operator==(const Timestamp & n) const { return (this->value==n.value && this->scale==n.scale); }
	inline bool operator<(const Timestamp & n) const { return (this->getTimeInSeconds() < n.getTimeInSeconds()); };
	inline bool operator>(const Timestamp & n) const { return (this->getTimeInSeconds() > n.getTimeInSeconds()); };
};


//	Timestamper class- vends timestamps for buffers
class Timestamper	{
	private:
		chrono::time_point<chrono::high_resolution_clock>		stampStartTime;
		intmax_t		timescale = 600;
	public:
		Timestamper(const Timestamper & n) = default;
		Timestamper() { reset(); }
		inline void setTimescale(const intmax_t & n) { timescale=n; }
		inline Timestamp nowTime() const { using namespace chrono; return Timestamp((uint64_t)duration_cast<microseconds>(high_resolution_clock::now()-stampStartTime).count()/(1000000/600), 600); }
		inline void reset() { stampStartTime = chrono::high_resolution_clock::now(); };
};




}


#endif /* VVGL_Time_h */
