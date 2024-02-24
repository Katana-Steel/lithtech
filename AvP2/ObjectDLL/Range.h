#ifndef _RANGE_H_
#define _RANGE_H_

#include "iltmessage.h"

template< typename NumericT >
class Range
{

public :

	Range() 
		: rangeMin(0),
		  rangeMax(0) {}

	Range(NumericT new_min, NumericT new_max) 
		: rangeMin(new_min),
		  rangeMax(new_max)  {}

	Range(const Range & new_range)
		: rangeMin(new_range.rangeMin),
		  rangeMax(new_range.rangeMax) {}

	NumericT Min() const { return rangeMin; }
	NumericT Max() const { return rangeMax; }

	friend ILTMessage & operator<<(ILTMessage & out, /*const*/ Range & x)
	{
		out << x.rangeMin;
		out << x.rangeMax;
		
		return out;
	}

	friend ILTMessage & operator>>(ILTMessage & in, Range & x)
	{
		in >> x.rangeMin;
		in >> x.rangeMax;
		
		return in;
	}

private :

	NumericT rangeMin;
	NumericT rangeMax;

};

typedef Range<LTFLOAT> FRange;

#endif
