#ifndef VVGL_Range_h
#define VVGL_Range_h

#include "VVGL_Defines.hpp"




namespace VVGL
{




struct Range	{
	size_t		loc = 0;
	size_t		len = 0;
	
	Range() {}
	Range(const size_t & inLoc, const size_t & inLen) : loc(inLoc), len(inLen) {}
	
	inline size_t max() const { return loc+len; }
	inline size_t min() const { return loc; }
	inline bool intersects(const Range & n) const	{
		size_t		tmpLoc = this->min();
		size_t		tmpMin = n.min();
		size_t		tmpMax = n.max();
		if (tmpLoc>=tmpMin && tmpLoc<=tmpMax)
			return true;
		tmpLoc = this->max();
		if (tmpLoc>=tmpMin && tmpLoc<=tmpMax)
			return true;
		return false;
	}
	Range & operator=(const Range & n) { loc=n.loc; len=n.len; return *this; }
	bool operator==(const Range & n) { return (loc==n.loc && len==n.len); }
};




}




#endif /* VVGL_Range_h */
