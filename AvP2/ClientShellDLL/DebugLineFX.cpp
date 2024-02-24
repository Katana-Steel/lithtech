// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineFX.cpp
//
// PURPOSE : DebugLine special FX - Implementation
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebugLineFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "RCMessage.h"
#include "MsgIDs.h"
#include <algorithm>

#ifdef _DEBUG
//#define DEBUGLINEFX_DEBUG 
#endif

namespace /* unnamed */
{
	class EraseLine
	{
		CClientDE*	 m_pClientDE;
		HLOCALOBJ    m_hObject;

	public:

		EraseLine(CClientDE * pClientDE, HLOCALOBJ hObject)
			: m_pClientDE(pClientDE),
			  m_hObject(hObject)        { ASSERT( pClientDE );  ASSERT( hObject ); }

		CDebugLineFX::LineListElement operator()(CDebugLineFX::LineListElement x) const
		{
			// x.first is the line's handle, type HTLINE.
			// x.second is the line's information, type DebugLine.

			if( x.first )
			{
				m_pClientDE->RemoveLine(m_hObject, x.first);
				x.first = LTNULL;
			}
			return x;
		}
	};

	class DrawLine
	{
		CClientDE*	 m_pClientDE;
		HLOCALOBJ    m_hObject;
		LTVector	 m_vSystemOrigin;

	public:

		DrawLine(CClientDE * pClientDE, HLOCALOBJ hObject, const LTVector & system_origin)
			: m_pClientDE(pClientDE),
			  m_hObject(hObject),
		      m_vSystemOrigin(system_origin) { ASSERT( pClientDE );  ASSERT( hObject ); }

		CDebugLineFX::LineListElement operator()(CDebugLineFX::LineListElement x) const
		{
			// x.first is the line's handle, type HTLINE.
			// x.second is the line's information, type DebugLine.

			if( !x.first )
			{
				const DebugLine & debug_line = x.second;
				LTLine lt_line;

				lt_line.m_Points[0].m_Pos = debug_line.vSource - m_vSystemOrigin;
				lt_line.m_Points[0].r = debug_line.vColor.x;
				lt_line.m_Points[0].g = debug_line.vColor.y;
				lt_line.m_Points[0].b = debug_line.vColor.z;
				lt_line.m_Points[0].a = debug_line.fAlpha;

				lt_line.m_Points[1].m_Pos = debug_line.vDest - m_vSystemOrigin;
				lt_line.m_Points[1].r = debug_line.vColor.x;
				lt_line.m_Points[1].g = debug_line.vColor.y;
				lt_line.m_Points[1].b = debug_line.vColor.z;
				lt_line.m_Points[1].a = debug_line.fAlpha;

				x.first = m_pClientDE->AddLine(m_hObject, &lt_line);

			}
			return x;
		}
	};

}; // namespace /* unnamed */







// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Init
//
//	PURPOSE:	Initialize from a server message. Set-up an init struct and call the other Init().
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	if( !hRead ) return LTFALSE;

	// Let our base class read his info.
	if (!CSpecialFX::Init(hServObj, hRead)) return LTFALSE;


	// Set-up the create struct.
	DebugLineCreator createStruct;
	createStruct.hServerObj = hServObj;
	
	// Initialize with the create struct.
	return Init(&createStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Init
//
//	PURPOSE:	Initialize from the init struct.
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	// This is here for future work.
	//DebugLineCreator* pDL = (DebugLineCreator*)psfxCreateStruct;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::CreateObject
//
//	PURPOSE:	Creates the object and sends a message to the server for the data.
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::CreateObject(CClientDE* pClientDE)
{
	if (!CBaseLineSystemFX::CreateObject(pClientDE)) return LTFALSE;

	ASSERT( m_pClientDE );

#ifdef DEBUGLINEFX_DEBUG 
		m_pClientDE->CPrint("%f : Linesystem %d created.", 
			m_pClientDE->GetTime(), GetObject() );
#endif


	// Send a message to the server object requesting data.
	RCMessage pMsg;
	if( !pMsg.null() )
	{
		pMsg->WriteByte( SFX_DEBUGLINE_ID );
		pMsg->WriteObject( m_hServerObject );

		pMsg->WriteByte( LTTRUE );
	}

	m_pClientDE->SendToServer(*pMsg,MID_SFX_MESSAGE,0);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::WantRemove
//
//	PURPOSE:	Tells the server to stop updating this client.
//
// ----------------------------------------------------------------------- //
void CDebugLineFX::WantRemove(DBOOL bRemove) 
{
	
	// Send a message to the server object requesting to stop data.
	RCMessage pMsg;
	if( !pMsg.null() )
	{
		pMsg->WriteByte( SFX_DEBUGLINE_ID );
		pMsg->WriteObject( m_hServerObject );

		pMsg->WriteByte( LTFALSE );
	}

	m_pClientDE->SendToServer(*pMsg,MID_SFX_MESSAGE,0);

	// Let the base class take care of the rest of removal.
	CBaseLineSystemFX::WantRemove(bRemove);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::OnServerMessage
//
//	PURPOSE:	Read an update message from the server.
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::OnServerMessage(HMESSAGEREAD hMessage)
{

	// Read the number of lines.
	const int num_lines = hMessage->ReadWord();

	// Set the new maximum number of lines
	m_nMaxLines = hMessage->ReadWord();

	// See if the server is telling us to clear our old lines.
	*hMessage >> m_bClearOldLines;

#ifdef DEBUGLINEFX_DEBUG 
		m_pClientDE->CPrint("Reading %d lines, clear lines is %s.",
			num_lines, m_bClearOldLines ? "true" : "false");
#endif

	// If we don't have any lines, we want to clear our old lines
	// so that the object will be re-positioned correctly.
	if( lines.empty() )
		m_bClearOldLines = true;

	// Clear the lines from memory.  The lines will be removed from
	// the line system in Update.
	if( m_bClearOldLines )
	{
#ifdef DEBUGLINEFX_DEBUG 
		m_pClientDE->CPrint("Clearing %d lines.", lines.size());
#endif

		std::for_each(lines.begin(),lines.end(), EraseLine(m_pClientDE,m_hObject) );
		lines.clear();
	}

	// Read each line.
	for(int i = 0; i < num_lines; ++i)
	{
		DebugLine new_line;
		*hMessage >> new_line;

		lines.push_back( LineListElement(LTNULL,new_line) );
	}

	// Make sure the lines get updated.
	m_bUpdateLines = true;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Update
//
//	PURPOSE:	If needed, update the cast line (recalculate end point),
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return LTFALSE;

	if ( IsWaitingForRemove() )
	{
		// No sense in adding lines now.
		return LTFALSE;
	}

	LTVector vCurrentLocation;
	m_pClientDE->GetObjectPos(m_hObject, &vCurrentLocation);

	if (m_bUpdateLines || !m_vLastLocation.Equals( vCurrentLocation, 0.01f ) )
	{
		m_bUpdateLines = false;
		m_vLastLocation = vCurrentLocation;

#ifdef DEBUGLINEFX_DEBUG 
		{
			for( LineList::iterator iter = lines.begin(); iter != lines.end(); ++iter )
			{
				m_pClientDE->CPrint("Element %d handle is %x before update.", iter - lines.begin(), iter->first);
			}
		}
#endif

		// Trim the list to max lines or less.
		while( lines.size() > (uint32)m_nMaxLines )
		{
			const HLTLINE line_handle = lines.front().first;
			if( line_handle )
			{
				m_pClientDE->RemoveLine(m_hObject, line_handle);
			}

			lines.pop_front();
		}

		// Clear all the old lines from the line system.
		std::transform(lines.begin(),lines.end(), lines.begin(), EraseLine(m_pClientDE,m_hObject) );



		// Add all the lines (translated by the line system's location).
		std::transform( lines.begin(), lines.end(), lines.begin(), DrawLine(m_pClientDE,m_hObject,m_vLastLocation) );

#ifdef DEBUGLINEFX_DEBUG  
		{
			m_pClientDE->CPrint("System center is (%.2f,%.2f,%.2f).", 
				vLineSystemLocation.x, vLineSystemLocation.y, vLineSystemLocation.z );
		}
		{
			for( LineList::iterator iter = lines.begin(); iter != lines.end(); ++iter )
			{
				m_pClientDE->CPrint("Element %d handle is %x after drawing.", iter - lines.begin(), iter->first);
			}
		}
#endif

	}

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_CastLineFactory
//-------------------------------------------------------------------------------------------------

const SFX_DebugLineFactory SFX_DebugLineFactory::registerThis;

CSpecialFX* SFX_DebugLineFactory::MakeShape() const
{
	return new CDebugLineFX();
}

