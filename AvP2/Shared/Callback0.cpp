
#include "stdafx.h"
#include "callback0.h"

// Because this isn't a template, it can not be in the header file.
RCPointer< Base_CallbackBody0<void> > 
make_nilcallback0v()
{
	static RCPointer< Base_CallbackBody0<void> >
		singleton(new Nil_CallbackBody0<void>);
	
	return singleton;
}
