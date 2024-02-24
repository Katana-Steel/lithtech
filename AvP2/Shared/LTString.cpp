
#include "stdafx.h"
#include "LTString.h"
#include <assert.h>

LTString::LTString(bool caseless /* = false */)
	: caseless_(caseless),
	  data_(0),
	  cstr_(0)  {}

LTString::LTString(HSTRING data, bool caseless /* = false */ )
	: caseless_(caseless),
	  data_( data ),
	  cstr_(0)  {}

LTString::LTString(const char * cstring, bool caseless /* = false */ )
	: caseless_(caseless),
	  data_( cstring ? g_pInterface->CreateString(const_cast<char*>(cstring)) : 0 ),
	  cstr_(0)  { }

LTString::LTString(const LTString & other)
	: caseless_(other.caseless_),
	  data_( other.data_ ? g_pInterface->CopyString(other.data_) : 0 ),
	  cstr_(other.cstr_) {}

LTString::~LTString()
{
	if( data_ ) g_pInterface->FreeString(data_);
}

const char * LTString::CStr() const
{
	if( !data_ ) return 0;

	if( !cstr_ )
	{
		// comments in server_de.h warn that GetStringData may be slow.
		cstr_ = g_pInterface->GetStringData( data_ );
	}

	return cstr_;
}


LTString & LTString::operator=(const LTString & other)
{
	if( this != &other )
	{
		// increment other's count first, so that if we have the
		// the same hstring, freestring won't delete it before we assign it
		// to itself!
		HSTRING new_data = other.data_ ? g_pInterface->CopyString( other.data_ ) : 0;
		if( data_ ) g_pInterface->FreeString(data_);
		data_ = new_data;
		cstr_ = other.cstr_;
	}
	return *this;
}

LTString & LTString::operator=(HSTRING other)
{
	if( data_ ) g_pInterface->FreeString(data_);
	data_ = other;
	cstr_ = 0;

	return *this;
}

LTString & LTString::operator=(const char * other)
{
	if( data_ ) g_pInterface->FreeString(data_);
	data_ = other ? g_pInterface->CreateString( const_cast<char*>(other) ) : 0;
	cstr_ = 0;

	return *this;
}


LTString & LTString::operator=(int zero)
{
	assert( zero == 0 );

	return operator=( static_cast<HSTRING>(0) );
}

bool operator==(const LTString & lhs, const LTString & rhs)
{
	if( lhs.HStr() == rhs.HStr() )
		return true;

	if( lhs.IsNull() || rhs.IsNull() )
		return false;  	// The hstr()==hstr() check above will return true if they are both null

	if( lhs.Caseless() || rhs.Caseless() )
		return g_pInterface->CompareStringsUpper( lhs.HStr(), rhs.HStr() ) != FALSE;
		
	return g_pInterface->CompareStrings( lhs.HStr(), rhs.HStr() ) != FALSE;
}

bool operator==(const LTString & lhs, HSTRING rhs)
{
	if( lhs.HStr() == rhs )
		return true;

	if( lhs.IsNull() || rhs == 0 )
		return false;  	// The hstr()==hstr() check above will return true if they are both null

	if( lhs.Caseless() )
		return g_pInterface->CompareStringsUpper( lhs.HStr(), rhs ) != FALSE;
		
	return g_pInterface->CompareStrings( lhs.HStr(), rhs ) != FALSE;
}


bool operator==(const LTString & lhs, const char * rhs)
{
	if( lhs.IsNull() && !rhs )
		return true;

	if( lhs.IsNull() || !rhs )
		return false;

	if( lhs.Caseless() )
		return stricmp( lhs.CStr(), rhs ) == 0;
		
	return strcmp( lhs.CStr(), rhs ) == 0;
}


ILTMessage & operator<<(ILTMessage & out, const LTString & val)
{
	out.WriteByte(val.caseless_);
	out.WriteHString(val.data_);

	return out;
}


ILTMessage & operator>>(ILTMessage & in, LTString & val)
{
	val.caseless_ = (in.ReadByte() != DFALSE);
	val.data_ = in.ReadHString();

	val.cstr_ = 0;
	return in;
}
