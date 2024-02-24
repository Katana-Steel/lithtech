#ifndef CALLBACK5_H
#define CALLBACK5_H

#ifndef RCPOINTER_H
#include "rcpointer.h"
#endif


template<class R, class P1, class P2, class P3, class P4, class P5 >
class Base_CallbackBody5
{
public:
	virtual R operator()(P1,P2,P3,P4,P5) = 0;
	virtual ~Base_CallbackBody5() { }
};



template<class R, class P1, class P2, class P3, class P4, class P5>
class Nil_CallbackBody5 : public Base_CallbackBody5<R,P1,P2,P3,P4,P5>
{
public:
	/* virtual */ R operator()(P1,P2,P3,P4,P5)
	{
		return R(0);
	}
};

template<class R, class P1, class P2, class P3, class P4, class P5>
RCPointer< Base_CallbackBody5<R,P1,P2,P3,P4,P5> >
make_nilcallback5()
{
	static RCPointer< Base_CallbackBody5<R,P1,P2,P3,P4,P5> >
		singleton(new Nil_CallbackBody5<R,P1,P2,P3,P4,P5>);
	
	return singleton;
}

template<class P1, class P2, class P3, class P4, class P5>
class Nil_CallbackBody5v : public Base_CallbackBody5<void,P1,P2,P3,P4,P5>
{
public:
	/* virtual */ void operator()(P1,P2,P3,P4,P5)
	{
	}
};

template<class P1, class P2, class P3, class P4, class P5>
RCPointer< Base_CallbackBody5<void,P1,P2,P3,P4,P5> >
make_nilcallback5v()
{
	static RCPointer< Base_CallbackBody5<void,P1,P2,P3,P4,P5> >
		singleton(new Nil_CallbackBody5v<P1,P2,P3,P4,P5>);
	
	return singleton;
}

template<class R, class P1, class P2,  class P3, class P4, class P5,
         class Client, class Member>
class Member_CallbackBody5 : public Base_CallbackBody5<R,P1,P2,P3,P4,P5>
{
public:

	Member_CallbackBody5(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_, P5 param5_)
	{
		return ((&_client)->*_member)(param1_,param2_,param3_,param4_,param5_);
	}

private:

	Client & _client;
	Member _member;
};

template<class P1, class P2, class P3, class P4, class P5,
         class Client, class Member>
class Member_CallbackBody5v
	: public Base_CallbackBody5<void,P1,P2,P3,P4,P5>
{
public:

	Member_CallbackBody5v(Client & client_, Member member_)
			: _client(client_), _member(member_) { }

	/* virtual */ void operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_, P5 param5_)
	{
		((&_client)->*_member)(param1_,param2_,param3_,param4_,param5_);
	}

private:

	Client & _client;
	Member _member;
};


template<class R, class P1, class P2,  class P3, class P4, class P5,
         class Function>
class Function_CallbackBody5 : public Base_CallbackBody5<R,P1,P2,P3,P4,P5>
{
public:
	Function_CallbackBody5(Function & function_) : _function(function_) { }
	
	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_, P5 param5_)
	{
		return _function(param1_,param2_,param3_,param4_,param5_);
	}

private:
	Function _function;
};


template<class P1, class P2,  class P3, class P4, class P5,
         class Function>
class Function_CallbackBody5v
	: public Base_CallbackBody5<void,P1,P2,P3,P4,P5>
{
public:
	Function_CallbackBody5v(Function & function_) : _function(function_) { }
	
	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_, P4 param4_, P5 param5_)
	{
		_function(param1_,param2_,param3_,param4_,param5_);
	}

private:
	Function _function;
};


template<class R, class P1, class P2, class P3, class P4, class P5 >
class Callback5
{
public:

	Callback5()
			: body_( make_nilcallback5<R,P1,P2,P3,P4,P5>() ) { }
	
	Callback5(Base_CallbackBody5<R,P1,P2,P3,P4,P5> * body)
			: body_(body) { }

	Callback5(RCPointer< Base_CallbackBody5<R,P1,P2,P3,P4,P5> > body)
			: body_(body) { }
	
	Callback5(const Callback5 & callback)
			: body_( callback.body_ ) { }
	
	~Callback5() { }
	
	Callback5 & operator=(const Callback5 & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ R operator()(P1 param1_, P2 param2_,
										P3 param3_, P4 param4_, P5 param5_)
	{
		return (*body_)(param1_,param2_,param3_,param4_,param5_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback5<R,P1,P2,P3,P4,P5>();
	}

	friend inline bool operator==(const Callback5 & lhs, const Callback5 & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback5 & lhs, const Callback5 & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody5<R,P1,P2,P3,P4,P5> > body_;
};

template<class P1, class P2, class P3, class P4, class P5 >
class Callback5v
{
public:

	Callback5v()
			: body_( make_nilcallback5v<P1,P2,P3,P4,P5>() ) { }
	
	Callback5v(Base_CallbackBody5<void,P1,P2,P3,P4,P5> * body)
			: body_(body) { }

	Callback5v(RCPointer< Base_CallbackBody5<void,P1,P2,P3,P4,P5> > body)
			: body_(body) { }

	Callback5v(const Callback5v & callback)
			: body_( callback.body_ ) { }
	
	~Callback5v() { }
	
	Callback5v & operator=(const Callback5v & callback)
	{
		if( this != &callback )
		{
			body_ = callback.body_;
		}
		return *this;
	}
	
	/* virtual */ void operator()(P1 param1_, P2 param2_,
											P3 param3_, P4 param4_, P5 param5_)
	{
		(*body_)(param1_,param2_,param3_,param4_,param5_);
	}

	bool isNil() const
	{
		return body_ == make_nilcallback5v<P1,P2,P3,P4,P5>();
	}

	friend inline bool operator==(const Callback5v & lhs, const Callback5v & rhs)
	{
		return lhs.body_ == rhs.body_;
	}

	friend inline bool operator<(const Callback5v & lhs, const Callback5v & rhs)
	{
		return lhs.body_ < rhs.body_;
	}

private:

	RCPointer< Base_CallbackBody5<void,P1,P2,P3,P4,P5> > body_;
};


template< class R, class P1, class P2,  class P3, class P4, class P5,
          class Client, class Member>
Callback5<R,P1,P2,P3,P4,P5>
make_callback5(Client & client_, Member member_)
{
	return Callback5<R,P1,P2,P3,P4,P5>(
		new Member_CallbackBody5<R,P1,P2,P3,P4,P5,Client,Member>(client_,member_)
		);
}

template< class R, class P1, class P2,  class P3, class P4, class P5,
          class Function >
Callback5<R,P1,P2,P3,P4,P5>
make_callback5(Function function_)
{
	return Callback5<R,P1,P2,P3,P4,P5>(
     new Function_CallbackBody5<R,P1,P2,P3,P4,P5,Function>(function_)
	  );
}

template< class P1, class P2,  class P3, class P4, class P5,
          class Client, class Member>
Callback5v<P1,P2,P3,P4,P5>
make_callback5v(Client & client_, Member member_)
{
	return Callback5v<P1,P2,P3,P4,P5>(
		new Member_CallbackBody5v<P1,P2,P3,P4,P5,Client,Member>(client_,member_)
		);
}

template< class P1, class P2,  class P3, class P4, class P5,
          class Function >
Callback5v<P1,P2,P3,P4,P5>
make_callback5v(Function function_)
{
	return Callback5v<P1,P2,P3,P4,P5>(
     new Function_CallbackBody5v<P1,P2,P3,P4,P5,Function>(function_)
	  );
}



#endif //#ifndef CALLBACK5_H
