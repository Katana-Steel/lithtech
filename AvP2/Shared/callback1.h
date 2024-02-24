#ifndef CALLBACK1_H
#define CALLBACK1_H

#ifndef RCPOINTER_H
#include "rcpointer.h"
#endif

#include <functional>

template<class R, class P1 >
class Base_CallbackBody1
{
public:
	virtual R operator()(P1) = 0;
	virtual ~Base_CallbackBody1() { }
};



template<class R, class P1>
class Nil_CallbackBody1
	: public Base_CallbackBody1<R,P1>
{
public:
	/* virtual */ R operator()(P1)
	{
		return R(0);
	}
};


template<class R, class P1>
RCPointer< Base_CallbackBody1<R,P1> >
make_nilcallback1()
{
	static RCPointer< Base_CallbackBody1<R,P1> >
		singleton(new Nil_CallbackBody1<R,P1>);
	
	return singleton;
}

template<class P1>
class Nil_CallbackBody1v
	: public Base_CallbackBody1<void,P1>
{
public:
	/* virtual */ void operator()(P1)
	{
	}
};

template<class P1>
RCPointer< Base_CallbackBody1<void,P1> >
make_nilcallback1v()
{
	static RCPointer< Base_CallbackBody1<void,P1> >
		singleton(new Nil_CallbackBody1v<P1>);
	
	return singleton;
}



template<class R, class P1,
         class Client, class Member>
class Member_CallbackBody1
	: public Base_CallbackBody1<R,P1>
{
public:

	Member_CallbackBody1(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ R operator()(P1 param1_)
	{
		return ((&_client)->*_member)(param1_);
	}

private:

	Client & _client;
	Member _member;
};

template<class P1,
         class Client, class Member>
class Member_CallbackBody1v
	: public Base_CallbackBody1<void,P1>
{
public:

	Member_CallbackBody1v(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ void operator()(P1 param1_)
	{
		((&_client)->*_member)(param1_);
	}

private:

	Client & _client;
	Member _member;
};


template<class R, class P1,
         class Function>
class Function_CallbackBody1
	: public Base_CallbackBody1<R,P1>
{
public:
	Function_CallbackBody1(Function & function_) : _function(function_) { }
	
	/* virtual */ R operator()(P1 param1_)
	{
		return _function(param1_);
	}

private:
	Function _function;
};


template<class P1, class Function>
class Function_CallbackBody1v
	: public Base_CallbackBody1<void,P1>
{
public:
	Function_CallbackBody1v(Function & function_) : _function(function_) { }
	
	/* virtual */ void operator()(P1 param1_)
	{
		_function(param1_);
	}

private:
	Function _function;
};


template<class R, class P1>
class Callback1 : public std::unary_function<P1,R>
{
public:

	Callback1()
			: body_( make_nilcallback1<R,P1>() ) { }
	
	Callback1(Base_CallbackBody1<R,P1> * body)
			: body_(body) { }

	Callback1(const Callback1 & callback)
			: body_( callback.body_ ) { }
	
	~Callback1() { }
	
	Callback1 & operator=(const Callback1 & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ R operator()(P1 param1_) const
	{
		return (*body_)(param1_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback1<R,P1>();
	}

	friend inline bool operator==(const Callback1 & lhs, const Callback1 & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback1 & lhs, const Callback1 & rhs)
	{
		return lhs.body_ < rhs.body_;
	}


private:

	RCPointer< Base_CallbackBody1<R,P1> > body_;
};

template<class P1>
class Callback1v  : public std::unary_function<P1,void>
{
public:

	Callback1v()
			: body_( make_nilcallback1v<P1>() ) { }
	
	Callback1v(Base_CallbackBody1<void,P1> * body)
			: body_(body) { }

	Callback1v(const Callback1v & callback)
			: body_( callback.body_ ) { }
	
	~Callback1v() { }
	
	Callback1v & operator=(const Callback1v & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ void operator()(P1 param1_) const
	{
		(*body_)(param1_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback1v<P1>();
	}

	friend inline bool operator==(const Callback1v & lhs, const Callback1v & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback1v & lhs, const Callback1v & rhs)
	{
		return lhs.body_ < rhs.body_;
	}


private:

	RCPointer< Base_CallbackBody1<void,P1> > body_;
};


template< class R, class P1, class Client, class Member>
Callback1<R,P1>
make_callback1(Client & client_, Member member_)
{
	return Callback1<R,P1>(
	new Member_CallbackBody1<R,P1,Client,Member>(client_,member_)
		);
}

template< class R, class P1, class Function >
Callback1<R,P1>
make_callback1(Function function_)
{
	return Callback1<R,P1>(
     new Function_CallbackBody1<R,P1,Function>(function_)
	  );
}

template< class P1, class Client, class Member>
Callback1v<P1>
make_callback1v(Client & client_, Member member_)
{
	return Callback1v<P1>(
	new Member_CallbackBody1v<P1,Client,Member>(client_,member_)
		);
}

template< class P1, class Function >
Callback1v<P1>
make_callback1v(Function function_)
{
	return Callback1v<P1>(
     new Function_CallbackBody1v<P1,Function>(function_)
	  );
}


#endif //#ifndef CALLBACK1_H
