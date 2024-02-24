// ----------------------------------------------------------------------- //
//
// MODULE  : Chassis.h
//
// PURPOSE : The object used to represent the chassis object.  An object
//				that allows players to build using chassispiece objects.
//
// CREATED : 5/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHASSIS_H__
#define __CHASSIS_H__

//
// Includes...
//

#include "PropType.h"
#include "Timer.h"

class CPlayerObj;
class ChassisPiece;
struct ChassisPieceBute;

class Chassis : public PropType 
{
	public: // Methods...

		Chassis( );
		~Chassis( );

		// Team owner of the chassis.
		uint8	GetTeamID( )			const { return m_nOwningTeamID; }

		// How close a dropped piece has to be to get placed on chassis.
		float	GetDropZoneRadius( )	const { return m_fDropZoneRadius; }

		// Adds a piece to the chassis.
		virtual bool	AddChassisPiece( ChassisPiece& chassisPiece, CPlayerObj* pPlayer );

		// Removes a piece from the chassis.
		virtual bool	RemoveChassisPiece( ChassisPiece& chassisPiece, CPlayerObj* pPlayer );

		// Checks if chassis has all its pieces.
		bool	IsAllPiecesPlaced( ) const;

		// List of all existing chassis in the world.
		typedef std::vector< Chassis* > ChassisList;
		static ChassisList const& GetChassisList( ) { return m_lstChassis; }

	protected: // Methods... 

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

	private:

		void	InitialUpdate( );

	protected:

		void	CreateSFXMessage( );

		bool	ReadProp( ObjectCreateStruct* pStruct );

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Removes all the chassis piece slots from the list.
		void	ClearChassisPieceSlotList( );

		// Team that owns this device.
		uint8					m_nOwningTeamID;

		// Radius of the area considered the 'drop zone'. 
		// When a piece is dropped within this radius it will be aded to the device.
		float					m_fDropZoneRadius;

		// Command sent when chassis is complete.
		std::string				m_sCommandComplete;

		// Represents a single slot that takes a ChassisPiece.
		struct ChassisPieceSlot
		{
			ChassisPieceSlot( )
			{
				m_pChassisPieceBute = NULL;
			}

			// Bute defines the slot.
			ChassisPieceBute*	m_pChassisPieceBute;

			// Target model for when there isn't a piece placed.
			LTObjRef			m_hPlaceHolderObject;

			// Holds placed piece.
			LTObjRef			m_hChassisPiece;
		};

		// Find the slot based on the bute.
		ChassisPieceSlot* FindChassisPieceSlot( ChassisPieceBute const& chassisPieceBute );

		// The chassispiece proptypes that this chassis
		// receives.
		typedef std::vector< ChassisPieceSlot > ChassisPieceSlotList;
		ChassisPieceSlotList	m_lstChassisPieceSlots;

		// List of all chassis within the game.
		static ChassisList		m_lstChassis;
};

class ChassisPlugin : public CPropTypePlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_PropChanged( const char *szObjName, 
										   const char *szPropName,
										   const int nPropType,
										   const GenericProp &gpPropValue,
										   ILTPreInterface *pInterface,
										   const char *szModifiers );

		CCommandMgrPlugin m_CommandMgrPlugin;
};

#endif // __CHASSIS_H__
