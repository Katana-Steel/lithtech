// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLine.h
//
// PURPOSE : Lines used to debug node system
//
// CREATED : 2/11/99
//
// ----------------------------------------------------------------------- //

#ifndef __NODELINE_H__
#define __NODLINE_H__

#include "cpp_engineobjects_de.h"
#include "ClientServerShared.h"

class NodeLine : public BaseClass
{
	public : // Public methods

		// Ctors/dtors/etc

		NodeLine() : BaseClass (OT_NORMAL) {}

		// Simple accessors

		void	Setup(const DVector& vSource, const DVector& vDestination);
		
	protected : // Protected methods (only accessed by engine)

		DDWORD	EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);
};

#endif
