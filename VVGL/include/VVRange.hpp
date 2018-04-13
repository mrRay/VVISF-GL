#ifndef VVRange_h
#define VVRange_h

#if ISF_TARGET_QT
#include "vvgl_qt_global.h"
#endif




namespace VVGL
{




struct VVRange	{
	size_t		loc = 0;
	size_t		len = 0;
	
	VVRange() {}
	VVRange(const size_t & inLoc, const size_t & inLen) : loc(inLoc), len(inLen) {}
	
	inline size_t max() const { return loc+len; }
	inline size_t min() const { return loc; }
	inline bool intersects(const VVRange & n) const	{
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
	VVRange & operator=(const VVRange & n) { loc=n.loc; len=n.len; return *this; }
	bool operator==(const VVRange & n) { return (loc==n.loc && len==n.len); }
};




}




#endif /* VVRange_h */
