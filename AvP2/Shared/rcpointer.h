#ifndef RCPOINTER_H
#define RCPOINTER_H

///
// RCPointer
//
//      Provides Reference-Counted Pointer.  Simple hand this
// an object to copy, or a pointer to take-over and it will
// manage it.
//
// NOTE: If you want to use poly-morphism, you must give RCPointer the
//   the base class

template<class T>
class RCPointer
{
	struct RCPointer_Body
	{
		// Use incReferences and decReferences to increment and decrement count_.
		int count_;
		T * object_;
		
		RCPointer_Body() : count_(0), object_(0) { }
		RCPointer_Body(T * object) : count_(1), object_(object) { }


		void decReferences()
		{
				if( object_ && --count_ <= 0 )
				{
					delete object_;
					object_ = 0;
				}
		}
		
		void incReferences()
		{
			if(object_) ++count_;
		}
		
		~RCPointer_Body()
		{
			delete object_;
		}
			
	private:
			
		RCPointer_Body(const RCPointer_Body & other);
	};
	
	RCPointer_Body * body_;
	
public:
	// create null pointer
	RCPointer() : body_(new RCPointer_Body()) { }

	// create pointer to copy of object
	RCPointer(const T & object)
			: body_( new RCPointer_Body(new T(object)) ) { }

	// hand pointer to be managed (poor style, but can be more efficient)
	RCPointer(T * object) : body_(new RCPointer_Body(object)) { }

	// copy constructor
	RCPointer(const RCPointer & other) : body_(other.body_)
	{
		other.body_->incReferences();
	}

	~RCPointer()
	{
		body_->decReferences();
		if( body_->count_ <= 0 ) delete body_;
	}

	RCPointer & operator=(const RCPointer & other)
	{
		if( this != &other )
		{
			// Increment _must_occur_ before decrement to avoid
			// self-assignment problem (ie. two smart pointers refering
			// to same object_ are used.)!!!!!!!!!!!!!!!!!
			other.body_->incReferences();
			body_->decReferences();
			body_ = other.body_;
		}
		return *this;
	}

 	RCPointer & operator=(int zero)
 	{
 		assert(zero == 0);
 		body_->decReferences();
 		body_ = 0;
 	}
	
	T * operator->() const
	{
		return body_->object_;
	}

	T & operator*() const
	{
		return *body_->object_;
	}

	T * get() const
	{
		return body_->object_;
	}

	bool null() const
	{
		return body_->object_ == 0;
	}

	int getReferenceCounts() const
	{
		return body_->count_;
	}

	friend inline bool operator==(const RCPointer & lhs, const RCPointer & rhs)
	{
		return lhs.body_->object_ == rhs.body_->object_;
	}

	friend inline bool operator<(const RCPointer & lhs, const RCPointer & rhs)
	{
		return lhs.body_->object_ < rhs.body_->object_;
	}
};



#endif //#ifndef RCPOINTER_H
