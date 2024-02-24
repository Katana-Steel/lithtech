#ifndef CALLBACK3_H
#define CALLBACK3_H

#ifndef RCPOINTER_H
#include "rcpointer.h"
#endif


template<class R, class P1, class P2, class P3>
class Base_CallbackBody3
{
public:
	virtual R operator()(P1,P2,P3) = 0;
	virtual ~Base_CallbackBody3() { }
};



template<class R, class P1, class P2, class P3>
class Nil_CallbackBody3
	: public Base_CallbackBody3<R,P1,P2,P3>
{
public:
	/* virtual */ R operator()(P1,P2,P3)
	{
		return R(0);
	}
};

template<class R, class P1, class P2, class P3>
RCPointer< Base_CallbackBody3<R,P1,P2,P3> >
make_nilcallback3()
{
	static RCPointer< Base_CallbackBody3<R,P1,P2,P3> >
		singleton(new Nil_CallbackBody3<R,P1,P2,P3>);
	
	return singleton;
}


template<class P1, class P2, class P3>
class Nil_CallbackBody3v
	: public Base_CallbackBody3<void,P1,P2,P3>
{
public:
	/* virtual */ void operator()(P1,P2,P3)
	{
	}
};

template<class P1, class P2, class P3>
RCPointer< Base_CallbackBody3<void,P1,P2,P3> >
make_nilcallback3v()
{
	static RCPointer< Base_CallbackBody3<void,P1,P2,P3> >
		singleton(new Nil_CallbackBody3v<P1,P2,P3>);
	
	return singleton;
}


template<class R, class P1, class P2,  class P3,
         class Client, class Member>
class Member_CallbackBody3
	: public Base_CallbackBody3<R,P1,P2,P3>
{
public:

	Member_CallbackBody3(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_)
	{
		return ((&_client)->*_member)(param1_,param2_,param3_);
	}

private:

	Client & _client;
	Member _member;
};

template<class P1, class P2, class P3,
         class Client, class Member>
class Member_CallbackBody3v
	: public Base_CallbackBody3<void,P1,P2,P3>
{
public:

	Member_CallbackBody3v(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_)
	{
		((&_client)->*_member)(param1_,param2_,param3_);
	}

private:

	Client & _client;
	Member _member;
};


template<class R, class P1, class P2,  class P3,
         class Function>
class Function_CallbackBody3
	: public Base_CallbackBody3<R,P1,P2,P3>
{
public:
	Function_CallbackBody3(Function & function_) : _function(function_) { }
	
	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_)
	{
		return _function(param1_,param2_,param3_);
	}

private:
	Function _function;
};


template<class P1, class P2,  class P3,
         class Function>
class Function_CallbackBody3v
	: public Base_CallbackBody3<void,P1,P2,P3>
{
public:
	Function_CallbackBody3v(Function & function_) : _function(function_) { }
	
	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_)
	{
		_function(param1_,param2_,param3_);
	}

private:
	Function _function;
};


template<class R, class P1, class P2,  class P3 >
class Callback3
{
public:

	Callback3()
			: body_( make_nilcallback3<R,P1,P2,P3>() ) { }
	
	Callback3(Base_CallbackBody3<R,P1,P2,P3> * body)
			: body_(body) { }

	Callback3(const Callback3 & callback)
			: body_( callback.body_ ) { }
	
	~Callback3() { }
	
	Callback3 & operator=(const Callback3 & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_)
	{
		return (*body_)(param1_,param2_,param3_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback3<R,P1,P2,P3>();
	}

	friend inline bool operator==(const Callback3 & lhs, const Callback3 & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback3 & lhs, const Callback3 & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody3<R,P1,P2,P3> > body_;
};

template<class P1, class P2, class P3 >
class Callback3v
{
public:

	Callback3v()
			: body_( make_nilcallback3v<P1,P2,P3>() ) { }
	
	Callback3v(Base_CallbackBody3<void,P1,P2,P3> * body)
			: body_(body) { }

	Callback3v(const Callback3v & callback)
			: body_( callback.body_ ) { }
	
	~Callback3v() { }
	
	Callback3v & operator=(const Callback3v & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_)
	{
		(*body_)(param1_,param2_,param3_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback3v<P1,P2,P3>();
	}

	friend inline bool operator==(const Callback3v & lhs, const Callback3v & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback3v & lhs, const Callback3v & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody3<void,P1,P2,P3> > body_;
};


template< class R, class P1, class P2,  class P3,
          class Client, class Member>
Callback3<R,P1,P2,P3>
make_callback3(Client & client_, Member member_)
{
	return Callback3<R,P1,P2,P3>(
	new Member_CallbackBody3<R,P1,P2,P3,Client,Member>(client_,member_)
		);
}

template< class R, class P1, class P2,  class P3,
          class Function >
Callback3<R,P1,P2,P3>
make_callback3(Function function_)
{
	return Callback3<R,P1,P2,P3>(
     new Function_CallbackBody3<R,P1,P2,P3,Function>(function_)
	  );
}



template< class P1, class P2,  class P3,
          class Client, class Member>
Callback3v<P1,P2,P3>
make_callback3v(Client & client_, Member member_)
{
	return Callback3v<P1,P2,P3>(
	new Member_CallbackBody3v<P1,P2,P3,Client,Member>(client_,member_)
		);
}

template< class P1, class P2,  class P3,
          class Function >
Callback3v<P1,P2,P3>
make_callback3v(Function function_)
{
	return Callback3v<P1,P2,P3>(
     new Function_CallbackBody3v<P1,P2,P3,Function>(function_)
	  );
}


#endif //#ifndef CALLBACK3_H
