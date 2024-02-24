#ifndef CALLBACK4_H
#define CALLBACK4_H

#ifndef RCPOINTER_H
#include "rcpointer.h"
#endif


template<class R, class P1, class P2, class P3, class P4>
class Base_CallbackBody4
{
public:
	virtual R operator()(P1,P2,P3,P4) = 0;
	virtual ~Base_CallbackBody4() { }
};



template<class R, class P1, class P2, class P3, class P4>
class Nil_CallbackBody4
	: public Base_CallbackBody4<R,P1,P2,P3,P4>
{
public:
	/* virtual */ R operator()(P1,P2,P3,P4)
	{
		return R(0);
	}
};

template<class R, class P1, class P2, class P3, class P4>
RCPointer< Base_CallbackBody4<R,P1,P2,P3,P4> >
make_nilcallback4()
{
	static RCPointer< Base_CallbackBody4<R,P1,P2,P3,P4> >
		singleton(new Nil_CallbackBody4<R,P1,P2,P3,P4>);
	
	return singleton;
}

template<class P1, class P2, class P3, class P4>
class Nil_CallbackBody4v
	: public Base_CallbackBody4<void,P1,P2,P3,P4>
{
public:
	/* virtual */ void operator()(P1,P2,P3,P4)
	{
	}
};

template<class P1, class P2, class P3, class P4>
RCPointer< Base_CallbackBody4<void,P1,P2,P3,P4> >
make_nilcallback4v()
{
	static RCPointer< Base_CallbackBody4<void,P1,P2,P3,P4> >
		singleton(new Nil_CallbackBody4v<P1,P2,P3,P4>);
	
	return singleton;
}

template<class R, class P1, class P2,  class P3, class P4,
         class Client, class Member>
class Member_CallbackBody4
	: public Base_CallbackBody4<R,P1,P2,P3,P4>
{
public:

	Member_CallbackBody4(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_)
	{
		return ((&_client)->*_member)(param1_,param2_,param3_,param4_);
	}

private:

	Client & _client;
	Member _member;
};

template<class P1, class P2, class P3, class P4,
         class Client, class Member>
class Member_CallbackBody4v
	: public Base_CallbackBody4<void,P1,P2,P3,P4>
{
public:

	Member_CallbackBody4v(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_, P4 param4_)
	{
		((&_client)->*_member)(param1_,param2_,param3_,param4_);
	}

private:

	Client & _client;
	Member _member;
};


template<class R, class P1, class P2,  class P3, class P4,
         class Function>
class Function_CallbackBody4
	: public Base_CallbackBody4<R,P1,P2,P3,P4>
{
public:
	Function_CallbackBody4(Function & function_) : _function(function_) { }
	
	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_)
	{
		return _function(param1_,param2_,param3_,param4_);
	}

private:
	Function _function;
};


template<class P1, class P2,  class P3, class P4,
         class Function>
class Function_CallbackBody4v
	: public Base_CallbackBody4<void,P1,P2,P3,P4>
{
public:
	Function_CallbackBody4v(Function & function_) : _function(function_) { }
	
	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_, P4 param4_)
	{
		_function(param1_,param2_,param3_,param4_);
	}

private:
	Function _function;
};


template<class R, class P1, class P2,  class P3, class P4 >
class Callback4
{
public:

	Callback4()
			: body_( make_nilcallback4<R,P1,P2,P3,P4>() ) { }
	
	Callback4(Base_CallbackBody4<R,P1,P2,P3,P4> * body)
			: body_(body) { }

	Callback4(const Callback4 & callback)
			: body_( callback.body_ ) { }
	
	~Callback4() { }
	
	Callback4 & operator=(const Callback4 & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_)
	{
		return (*body_)(param1_,param2_,param3_,param4_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback4<R,P1,P2,P3,P4>();
	}

	friend inline bool operator==(const Callback4 & lhs, const Callback4 & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback4 & lhs, const Callback4 & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody4<R,P1,P2,P3,P4> > body_;
};

template<class P1, class P2, class P3, class P4 >
class Callback4v
{
public:

	Callback4v()
			: body_( make_nilcallback4v<P1,P2,P3,P4>() ) { }
	
	Callback4v(Base_CallbackBody4<void,P1,P2,P3,P4> * body)
			: body_(body) { }

	Callback4v(const Callback4v & callback)
			: body_( callback.body_ ) { }
	
	~Callback4v() { }
	
	Callback4v & operator=(const Callback4v & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_, P4 param4_)
	{
		(*body_)(param1_,param2_,param3_,param4_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback4v<P1,P2,P3,P4>();
	}

	friend inline bool operator==(const Callback4v & lhs, const Callback4v & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback4v & lhs, const Callback4v & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody4<void,P1,P2,P3,P4> > body_;
};


template< class R, class P1, class P2,  class P3, class P4,
          class Client, class Member>
Callback4<R,P1,P2,P3,P4>
make_callback4(Client & client_, Member member_)
{
	return Callback4<R,P1,P2,P3,P4>(
	new Member_CallbackBody4<R,P1,P2,P3,P4,Client,Member>(client_,member_)
		);
}

template< class R, class P1, class P2,  class P3, class P4,
          class Function >
Callback4<R,P1,P2,P3,P4>
make_callback4(Function function_)
{
	return Callback4<R,P1,P2,P3,P4>(
     new Function_CallbackBody4<R,P1,P2,P3,P4,Function>(function_)
	  );
}

template< class P1, class P2,  class P3, class P4,
          class Client, class Member>
Callback4v<P1,P2,P3,P4>
make_callback4v(Client & client_, Member member_)
{
	return Callback4v<P1,P2,P3,P4>(
	new Member_CallbackBody4v<P1,P2,P3,P4,Client,Member>(client_,member_)
		);
}

template< class P1, class P2,  class P3, class P4,
          class Function >
Callback4v<P1,P2,P3,P4>
make_callback4v(Function function_)
{
	return Callback4v<P1,P2,P3,P4>(
     new Function_CallbackBody4v<P1,P2,P3,P4,Function>(function_)
	  );
}



#endif //#ifndef CALLBACK4_H
