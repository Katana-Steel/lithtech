#ifndef CALLBACK2_H
#define CALLBACK2_H

#ifndef RCPOINTER_H
#include "rcpointer.h"
#endif

#include <functional>

template<class R, class P1, class P2 >
class Base_CallbackBody2
{
public:
	virtual R operator()(P1,P2) = 0;
	virtual ~Base_CallbackBody2() { }
};



template<class R, class P1, class P2>
class Nil_CallbackBody2
	: public Base_CallbackBody2<R,P1,P2>
{
public:
	/* virtual */ R operator()(P1,P2)
	{
		return R(0);
	}
};

template<class R, class P1, class P2>
RCPointer< Base_CallbackBody2<R,P1,P2> >
make_nilcallback2()
{
	static RCPointer< Base_CallbackBody2<R,P1,P2> >
		singleton(new Nil_CallbackBody2<R,P1,P2>);
	
	return singleton;
}

template<class P1, class P2>
class Nil_CallbackBody2v
	: public Base_CallbackBody2<void,P1,P2>
{
public:
	/* virtual */ void operator()(P1,P2)
	{
	}
};

template<class P1, class P2>
RCPointer< Base_CallbackBody2<void,P1,P2> >
make_nilcallback2v()
{
	static RCPointer< Base_CallbackBody2<void,P1,P2> >
		singleton(new Nil_CallbackBody2v<P1,P2>);
	
	return singleton;
}

template<class R, class P1, class P2,
         class Client, class Member>
class Member_CallbackBody2
	: public Base_CallbackBody2<R,P1,P2>
{
public:

	Member_CallbackBody2(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ R operator()(P1 param1_, P2 param2_)
	{
		return ((&_client)->*_member)(param1_,param2_);
	}

private:

	Client & _client;
	Member _member;
};

template<class P1, class P2,
         class Client, class Member>
class Member_CallbackBody2v
	: public Base_CallbackBody2<void,P1,P2>
{
public:

	Member_CallbackBody2v(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ void operator()(P1 param1_, P2 param2_)
	{
		((&_client)->*_member)(param1_,param2_);
	}

private:

	Client & _client;
	Member _member;
};


template<class R, class P1, class P2,
         class Function>
class Function_CallbackBody2
	: public Base_CallbackBody2<R,P1,P2>
{
public:
	Function_CallbackBody2(Function & function_) : _function(function_) { }
	
	/* virtual */ R operator()(P1 param1_, P2 param2_)
	{
		return _function(param1_,param2_);
	}

private:
	Function _function;
};


template<class P1, class P2, class Function>
class Function_CallbackBody2v
	: public Base_CallbackBody2<void,P1,P2>
{
public:
	Function_CallbackBody2v(Function & function_) : _function(function_) { }
	
	/* virtual */ void operator()(P1 param1_, P2 param2_)
	{
		_function(param1_,param2_);
	}

private:
	Function _function;
};


template<class R, class P1, class P2>
class Callback2 : public std::binary_function<P1,P2,R>
{
public:

	Callback2()
			: body_( make_nilcallback2<R,P1,P2>() ) { }
	
	Callback2(Base_CallbackBody2<R,P1,P2> * body)
			: body_(body) { }

	Callback2(const Callback2 & callback)
			: body_( callback.body_ ) { }
	
	~Callback2() { }
	
	Callback2 & operator=(const Callback2 & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ R operator()(P1 param1_, P2 param2_)
	{
		return (*body_)(param1_,param2_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback2<R,P1,P2>();
	}

	friend inline bool operator==(const Callback2 & lhs, const Callback2 & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback2 & lhs, const Callback2 & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody2<R,P1,P2> > body_;
};

template<class P1, class P2 >
class Callback2v : public std::binary_function<P1,P2,void>
{
public:

	Callback2v()
			: body_( make_nilcallback2v<P1,P2>() ) { }
	
	Callback2v(Base_CallbackBody2<void,P1,P2> * body)
			: body_(body) { }

	Callback2v(const Callback2v & callback)
			: body_( callback.body_ ) { }
	
	~Callback2v() { }
	
	Callback2v & operator=(const Callback2v & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ void operator()(P1 param1_, P2 param2_)
	{
		(*body_)(param1_,param2_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback2v<P1,P2>();
	}

	friend inline bool operator==(const Callback2v & lhs, const Callback2v & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback2v & lhs, const Callback2v & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody2<void,P1,P2> > body_;
};


template< class R, class P1, class P2,class Client, class Member>
Callback2<R,P1,P2>
make_callback2(Client & client_, Member member_)
{
	return Callback2<R,P1,P2>(
	new Member_CallbackBody2<R,P1,P2,Client,Member>(client_,member_)
		);
}

template< class R, class P1, class P2, class Function >
Callback2<R,P1,P2>
make_callback2(Function function_)
{
	return Callback2<R,P1,P2>(
     new Function_CallbackBody2<R,P1,P2,Function>(function_)
	  );
}

template< class P1, class P2,class Client, class Member>
Callback2v<P1,P2>
make_callback2v(Client & client_, Member member_)
{
	return Callback2v<P1,P2>(
	new Member_CallbackBody2v<P1,P2,Client,Member>(client_,member_)
		);
}

template< class P1, class P2, class Function >
Callback2v<P1,P2>
make_callback2v(Function function_)
{
	return Callback2v<P1,P2>(
     new Function_CallbackBody2v<P1,P2,Function>(function_)
	  );
}



#endif //#ifndef CALLBACK2_H
