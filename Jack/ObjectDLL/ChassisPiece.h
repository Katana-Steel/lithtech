// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisPiece.h
//
// PURPOSE : The object used for placing on a Chassis Object.
//
// CREATED : 5/12/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHASSISPIECE_H__
#define __CHASSISPIECE_H__

//
// Includes...
//

#include "PropType.h"

struct ChassisPieceBute;
	
LINKTO_MODULE( ChassisPiece );

class ChassisPiece : public PropType 
{
	public: // Methods...

		ChassisPiece( );
		~ChassisPiece( );
		
		ChassisPieceBute const* GetChassisPieceBute( ) const { return m_pChassisPieceBute; }

		bool IsHeavy( ) const;
		
	protected: // Methods... 

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

		void	CreateSFXMessage(bool bCarried, uint8 nTeam, bool bUpdate = true);
		
		// Message handlers...
		
		virtual bool	OnCarry( HOBJECT hSender );
		virtual bool	OnDrop( HOBJECT hSender );
		virtual bool	OnRespawn( );

		// Override for SetNextUpdate so we can control Prop's deactivation behavior
		virtual void SetNextUpdate(LTFLOAT fDelta, eUpdateControl eControl=eControlDeactivation);


	private: // Methods...

		bool	ReadProp( ObjectCreateStruct *pOCS );
		
		void	Update( );

		void    Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
        void    Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		
	protected:	// Members...

		// Defines how the chassispiece works with the chassis.
		ChassisPieceBute* m_pChassisPieceBute;

		LTObjRef		m_hChassis;			// hObject of a device we are apart of.

		CTimer			m_RespawnTimer;		// Countdown timer for respawning after being dropped.
		float			m_fRespawnTime;		// Time until respawn.
		bool			m_bBlinking;		// Is the piece "blinking" to indicate it will respawn soon

		CTimer			m_DelayTimer;		// Countdown timer to delay picking up the piece after it drops

		LTVector		m_vOriginalPos;		// Position the piece was originally placed.
		
		bool			m_bCanBeCarried;	// Can this piece currently be picked up.
};

class CChassisPiecePlugin : public CPropTypePlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims);
};

#endif // __CHASSISPIECE_H__
