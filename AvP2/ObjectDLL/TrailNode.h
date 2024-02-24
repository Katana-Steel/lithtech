// ----------------------------------------------------------------------- //
//
// MODULE  : TrailNode.h
//
// PURPOSE : TrailNode - Definition
//
// CREATED : 10/13/00
//
// ----------------------------------------------------------------------- //

#ifndef __TRAIL_NODE_H__
#define __TRAIL_NODE_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"

// ----------------------------------------------------------------------- //

#define TRAILNODE_TYPE_DEFAULT				0

#define TRAILNODE_TYPE_MIN_BLOOD			1
#define TRAILNODE_TYPE_HUMAN_BLOOD			1
#define TRAILNODE_TYPE_PREDATOR_BLOOD		2
#define TRAILNODE_TYPE_ALIEN_BLOOD			3
#define TRAILNODE_TYPE_SYNTH_BLOOD			4
#define TRAILNODE_TYPE_MAX_BLOOD			4

#define TRAILNODE_TYPE_MAX					5

// ----------------------------------------------------------------------- //

class TrailNode : public GameBase
{
	public:

		// Construction and destruction
		TrailNode();
		~TrailNode();

		// Initialization
		void		Setup(uint8 nType, LTFLOAT fLifetime);

		// Main access to the properties
		uint8		GetNodeType()				{ return m_nType; }

		// Helper functions
		LTBOOL		IsBlood()
			{ return (m_nType >= TRAILNODE_TYPE_MIN_BLOOD) && (m_nType <= TRAILNODE_TYPE_MAX_BLOOD); }

	protected:

		// Handles engine messages
		uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32		ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

		// Reads the properties and saves them in the member variables
		LTBOOL		ReadProp(ObjectCreateStruct *pStruct);

		void		Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

	private:

		// Member variables
		uint8		m_nType;		// The type of trail node
		LTFLOAT		m_fLifetime;	// The lifetime of the trail node
};

// ----------------------------------------------------------------------- //

class CTrailNodePlugin : public IObjectPlugin
{
  public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath, 
		const char* szPropName, 
		char* const * aszStrings, 
		uint32* pcStrings, 
		const uint32 cMaxStrings, 
		const uint32 cMaxStringLength);

  private:

};

// ----------------------------------------------------------------------- //

#endif // __TRAIL_NODE_H__

