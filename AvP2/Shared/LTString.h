#ifndef LTSTRING_H
#define LTSTRING_H

#include "ltbasedefs.h"    //for HSTRING

class LTString
{
public:

	friend ILTMessage & operator<<(ILTMessage & out, const LTString & val);
	friend ILTMessage & operator>>(ILTMessage & in, LTString & val);

public:

	LTString(bool caseless = false);
	LTString(HSTRING data, bool caseless = false);
	LTString(const char * cstr, bool caseless = false);
	LTString(const LTString & other);

	~LTString();

	void Caseless(bool val)  { caseless_ = val; }
	bool Caseless() const	 { return caseless_; }

	const char * CStr() const;

	HSTRING HStr() const { return data_; }

	operator HSTRING() const { return HStr(); }

	LTString & operator=(const LTString & other);
	LTString & operator=(HSTRING other);
	LTString & operator=(const char * other);
	LTString & operator=(int zero);  // used to set string to null

	bool IsNull() const
	{
		return data_ == 0;
	}

private:

	bool	caseless_;
	HSTRING data_;
	mutable const char * cstr_;

};

class LTStringNoCase : public LTString
{
	// Becuase this class has no data, it (and LTString) doesn't need a virtual destructor.
public:
	LTStringNoCase() : LTString(true) {}
	LTStringNoCase(HSTRING data) : LTString(data,true) {}
	LTStringNoCase(const char * cstr) : LTString(cstr,true) {}
	LTStringNoCase(const LTString & other) : LTString(other) { Caseless(true); }
};



bool operator==(const LTString & lhs, const LTString & rhs);
bool operator==(const LTString & lhs, HSTRING rhs);
inline bool operator==(HSTRING lhs, const LTString & rhs) {	return rhs == lhs; }
bool operator==(const LTString & lhs, const char * rhs);
inline bool operator==(const char * lhs, const LTString & rhs) { return rhs == lhs; }

#endif  // LTSTRING_H
