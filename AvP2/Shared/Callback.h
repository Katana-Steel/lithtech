#ifndef CALLBACK_H
#define CALLBACK_H

///
// Callback
//
//    Provides nice callback implementation.  Can handle member functions,
// functors, and function pointers.  This generality is gained by
// sacrificing speed in the initialization stage ('new' memory allocation
// occurs once for every make_callback call).
//
//
//
//     Usage :
// ----------------------
//   double f(int x) { return double(x)/2.0; }
//
//   Callback1<double,int> SingleParameterCallback
//      = make_callback1<double,int>(f);
//
//   double result = SingleParameterCallback(3);
//
//   class F { void f(int x) { cout << double(x)/2.0 << endl; };
//   F an_F;
//
//   Callback1v<int> SingleParameterMemberCallback
//      = make_callback1v<int>(an_F,F::f);
//
//    SingleParameterMemberCallback(5);
// ----------------------
//
// An unassigned callback is created as if you assigned it to, make_nillcallback#()
// which returns the single instance of a NilCallback class (it is a singleton).
// The member function isNil() checks to see if the call back is unassigned.
// operator<() and operator==() have been implemented.  operator==() returns true if
// a callback body is the same instance.  In other words, it is usually only true if
// one callback was previously set to the other (through operator=()).  operator<()
// compares the pointer values of the callback body, it may be used for sorting a 
// list of callbacks (ie. to create an associative array, using the pointers as keys).
//
// WARNING : Be sure memory allocation for storing the function pointer
//   occurs in the same link unit (library) as de-allocation.  Allocation
//   happens where-ever make_callback (an inlined function) is called.
//   This can be fixed, ie. non-inlined, when compilers start implementing
//   'export'.
//
// ------------------------------------------------------------------------
//	This CallBack implementation is based on a paper by Paul Jakubik.
//	See: http://www.primenet.com/~jakubik/callback.html
//
// Much help was gained by consulting the code of Gary W. Powell.
//
// This is a modification of code by Paul Jakubik, which has the following
// restrictions:
// ------------------------------------------------------------------------
// Copyright (c) 1996 Paul Jakubik 
// 
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Paul Jakubik makes no representations 
// about the suitability of this software for any purpose.  It is 
// provided "as is" without express or implied warranty.
// ------------------------------------------------------------------------
///

// 4786 is the warning about a classname over 256 characters not being able
// to be used by the debugger (templates have their name mangled into long names).
//
// This will disable the warning for all files that include this header.  That is the 
// only way, as templates are generated in the source code (as the .cpp file is parsed).
#pragma warning( disable:4786 ) 

#include "callback5.h"
#include "callback4.h"
#include "callback3.h"
#include "callback2.h"
#include "callback1.h"
#include "callback0.h"


#endif //#ifndef CALLBACK_H
