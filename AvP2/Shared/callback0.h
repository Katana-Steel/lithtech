#ifndef CALLBACK0_H
#define CALLBACK0_H

#ifndef RCPOINTER_H
#include "rcpointer.h"
#endif

#include <functional>

template<class R>
class Base_CallbackBody0
{
public:
	virtual R operator()() = 0;

	virtual ~Base_CallbackBody0() { }
};



template<class R>
class Nil_CallbackBody0
	: public Base_CallbackBody0<R>
{
public:
	/* virtual */ R operator()()
	{
		return R(0);
	}
};

template<>
class Nil_CallbackBody0<void>
	: public Base_CallbackBody0<void>
{
public:
	/* virtual */ void operator()()
	{
	}
};

template<class R>
RCPointer< Base_CallbackBody0<R> >
make_nilcallback0()
{
	static RCPointer< Base_CallbackBody0<R> >
		singleton(new Nil_CallbackBody0<R>);
	
	return singleton;
}

RCPointer< Base_CallbackBody0<void> > 
make_nilcallback0v();

template<class R, class Client, class Member>
class Member_CallbackBody0
	: public Base_CallbackBody0<R>
{
public:

	Member_CallbackBody0(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ R operator()()
	{
		return ((&_client)->*_member)();
	}

private:

	Client & _client;
	Member _member;
};

template<class Client, class Member>
class Member_CallbackBody0v
	: public Base_CallbackBody0<void>
{
public:

	Member_CallbackBody0v(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ void operator()()
	{
		((&_client)->*_member)();
	}

private:

	Client & _client;
	Member _member;
};


template<class R, class Function>
class Function_CallbackBody0
	: public Base_CallbackBody0<R>
{
public:
	Function_CallbackBody0(Function & function_) : _function(function_) { }
	
	/* virtual */ R operator()()
	{
		return _function();
	}

private:
	Function _function;
};


template<class Function>
class Function_CallbackBody0v
	: public Base_CallbackBody0<void>
{
public:
	Function_CallbackBody0v(Function & function_) : _function(function_) { }
	
	/* virtual */ void operator()()
	{
		_function();
	}

private:
	Function _function;
};


template<class R>
class Callback0
{
public:

	Callback0()
			: body_( make_nilcallback0<R>() ) { }
	
	Callback0(Base_CallbackBody0<R> * body)
			: body_(body) { }

	Callback0(const Callback0 & callback)
			: body_( callback.body_ ) { }
	
	~Callback0() { }
	
	Callback0 & operator=(const Callback0 & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ R operator()()
	{
		return (*body_)();
	}

	bool isNil() const
	{
		return body_ == make_nilcallback0<R>();
	}

	friend inline bool operator==(const Callback0 & lhs, const Callback0 & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback0 & lhs, const Callback0 & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody0<R> > body_;
};

class Callback0v
{
public:

	Callback0v()
			: body_( make_nilcallback0v() ) { }
	
	Callback0v(Base_CallbackBody0<void> * body)
			: body_(body) { }

	Callback0v(const Callback0v & callback)
			: body_( callback.body_ ) { }
	
	~Callback0v() { }
	
	Callback0v & operator=(const Callback0v & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ void operator()()
	{
		(*body_)();
	}

	bool isNil() const
	{
		return body_ == make_nilcallback0v();
	}

	friend inline bool operator==(const Callback0v & lhs, const Callback0v & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback0v & lhs, const Callback0v & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody0<void> > body_;
};


template< class R, class Client, class Member>
Callback0<R>
make_callback0(Client & client_, Member member_)
{
	return Callback0<R>(
	new Member_CallbackBody0<R,Client,Member>(client_,member_)
		);
}

template< class R, class Function >
Callback0<R>
make_callback0(Function function_)
{
	return Callback0<R>(
     new Function_CallbackBody0<R,Function>(function_)
	  );
}

template< class Client, class Member>
Callback0v
make_callback0v(Client & client_, Member member_)
{
	return Callback0v(
	new Member_CallbackBody0v<Client,Member>(client_,member_)
		);
}

template< class Function >
Callback0v
make_callback0v(Function function_)
{
	return Callback0v(
     new Function_CallbackBody0v<Function>(function_)
	  );
}


#endif //#ifndef CALLBACK0_H
