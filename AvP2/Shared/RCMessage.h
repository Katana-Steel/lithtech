#ifndef RCMESSAGE_H
#define RCMESSAGE_H

#ifndef _GLOBALS_H_
#include "Globals.h"
#endif

///
// RCMessage
//
//      Provides Reference-Counted ILTMessage pointer.  Simply use it as 
//  an ILTMessage pointer.  It will release itself when it goes out of scope.
//  It may be copied and passed around freely. 
//
// NOTE:
//   _Please_ don't call Release with one of these. That will cause a double delete.
//   The program will crash with a debug build, and do almost anything in a release build.
///


class RCMessage
{
	struct RCMessage_Body
	{
		// Use incReferences and decReferences to increment and decrement reference_count.
		int reference_count;
		ILTMessage * message_ptr;
		
		RCMessage_Body() : reference_count(0), message_ptr(0) { }
		explicit RCMessage_Body(ILTMessage * object) : reference_count(1), message_ptr(object) { }


		void decReferences()
		{
				if( message_ptr && --reference_count <= 0 )
				{
					message_ptr->Release();
					message_ptr = 0;
				}
		}
		
		void incReferences()
		{
			if(message_ptr) ++reference_count;
		}
		
		~RCMessage_Body()
		{
			if( message_ptr ) message_ptr->Release();
		}
			
	private:
			
		// Copy construction and assignment is not permitted.
		RCMessage_Body(const RCMessage_Body & );
		RCMessage_Body & operator=(const RCMessage_Body & );
	};
	
	RCMessage_Body * rcmessage_body;

	
	
	static ILTMessage * createILTMessage()
	{
		ILTMessage * message_ptr;
		if( LT_OK != g_pInterface->Common()->CreateMessage(message_ptr) )
		{
			return LTNULL;
		}

		return message_ptr;
	}

public:

	// creates a new message.
	RCMessage()
		: rcmessage_body(new RCMessage_Body(createILTMessage())) {}

	// hand pointer to be managed (poor style, but can be more efficient)
	explicit RCMessage(ILTMessage * message_ptr) : rcmessage_body(new RCMessage_Body(message_ptr)) { }

	// copy constructor
	RCMessage(const RCMessage & other) : rcmessage_body(other.rcmessage_body)
	{
		other.rcmessage_body->incReferences();
	}

	~RCMessage()
	{
		rcmessage_body->decReferences();
		if( rcmessage_body->reference_count <= 0 ) delete rcmessage_body;
	}

	RCMessage & operator=(const RCMessage & other)
	{
		if( this != &other )
		{
			// Increment _must_occur_ before decrement to avoid
			// self-assignment problem (ie. two smart pointers refering
			// to same message_ptr are used.)!!!!!!!!!!!!!!!!!
			other.rcmessage_body->incReferences();
			rcmessage_body->decReferences();
			rcmessage_body = other.rcmessage_body;
		}
		return *this;
	}

 	RCMessage & operator=(int zero)
 	{
 		assert(zero == 0);
 		rcmessage_body->decReferences();
 		rcmessage_body = 0;

		return *this;
 	}
	
	ILTMessage * operator->() const
	{
		return rcmessage_body->message_ptr;
	}

	ILTMessage & operator*() const
	{
		assert(rcmessage_body->message_ptr);
		return *rcmessage_body->message_ptr;
	}

	ILTMessage * get() const
	{
		return rcmessage_body->message_ptr;
	}

	operator ILTMessage*() const
	{
		return get();
	}

	bool null() const
	{
		return rcmessage_body->message_ptr == 0;
	}

	int getReferenceCounts() const
	{
		return rcmessage_body->reference_count;
	}

	friend inline bool operator==(const RCMessage & lhs, const RCMessage & rhs)
	{
		return lhs.rcmessage_body->message_ptr == rhs.rcmessage_body->message_ptr;
	}

	friend inline bool operator<(const RCMessage & lhs, const RCMessage & rhs)
	{
		return lhs.rcmessage_body->message_ptr < rhs.rcmessage_body->message_ptr;
	}
};



#endif //#ifndef RCMESSAGE_H
