// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineSystem.cpp
//
// PURPOSE : 
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

// This whole file is empty for final build.
#ifndef _FINAL

#include "DebugLineSystem.h"
#include "SFXMsgIds.h"
#include "DebugLine.h"
#include "MsgIDs.h"
#include "RCMessage.h"

#include <vector>

#ifdef _DEBUG
//#include "AIUtils.h"
//#define LINESYSTEM_DEBUG
#endif

BEGIN_CLASS(DebugLineSystem)
	ADD_LONGINTPROP_FLAG(MaxLines, 125, 0)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(DebugLineSystem, BaseClass, NULL, NULL, CF_HIDDEN)



namespace /* unnamed */
{
	const float s_fUpdateDelay = 0.1f;

	// There are 10 bytes of overhead (see DebugLineSystem::Update), 
	// plus 40 bytes per line.  The -100 is to allow some room just in case!
	const int   s_MaxLinesPerMessage = (MAX_PACKET_LEN - 10 - 100) / 40;

	void TrimToWorld( DebugLine * debug_line_ptr, const LTVector & vWorldMin, const LTVector & vWorldMax)
	{
		_ASSERT( vWorldMin.x <= vWorldMax.x );
		_ASSERT( vWorldMin.y <= vWorldMax.y );
		_ASSERT( vWorldMin.z <= vWorldMax.z );

		_ASSERT( debug_line_ptr );
		if( !debug_line_ptr ) return;

		if( debug_line_ptr->vSource.x < vWorldMin.x ) debug_line_ptr->vSource.x = vWorldMin.x;
		if( debug_line_ptr->vSource.x > vWorldMax.x ) debug_line_ptr->vSource.x = vWorldMax.x;

		if( debug_line_ptr->vSource.y < vWorldMin.y ) debug_line_ptr->vSource.y = vWorldMin.y;
		if( debug_line_ptr->vSource.y > vWorldMax.y ) debug_line_ptr->vSource.y = vWorldMax.y;

		if( debug_line_ptr->vSource.z < vWorldMin.z ) debug_line_ptr->vSource.z = vWorldMin.z;
		if( debug_line_ptr->vSource.z > vWorldMax.z ) debug_line_ptr->vSource.z = vWorldMax.z;
	}
};



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Spawn
//
//	PURPOSE:	Creates a new debug line system.
//
// ----------------------------------------------------------------------- //
DebugLineSystem * DebugLineSystem::Spawn(const char * name, int max_lines /* = 300 */)
{
	const HCLASS hClass = g_pServerDE->GetClass("DebugLineSystem");

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	SAFE_STRCPY( theStruct.m_Name, name);
	theStruct.m_Pos = LTVector(0.0f,0.0f,0.0f);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE | FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType = OT_LINESYSTEM;

	char szProp[20];
	sprintf(szProp,"MaxLines %d", max_lines);

	return dynamic_cast<DebugLineSystem*>( g_pServerDE->CreateObjectProps(hClass, &theStruct, szProp) );
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::UpdateClient
//
//	PURPOSE:	Tells the line system a client object exists and should be given data.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::UpdateClient(HCLIENT hClient, LTBOOL bUpdate)
{
	if( bUpdate )
	{
		++m_nNumClients;
		nextLineToSend = lines.begin();
	}
	else
	{
		_ASSERT( m_nNumClients > 0 );
		if( m_nNumClients > 0 )
			--m_nNumClients;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::AddLine
//
//	PURPOSE:	Adds a new line to be drawn.  This should be the only
//				way a line is added to the system.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::AddLine(const DebugLine & new_line)
{
	if( !m_hObject ) return;

	// Record the offset of the sending iterator
	int send_offset = nextLineToSend - lines.begin();

	_ASSERT( (uint32)send_offset <= lines.size() );
	_ASSERT( send_offset >= 0 );

	// Trim the system if the number of lines is more than
	// the max_size.
	if( lines.size() >= (uint32)maxLines ) 
	{
		// Adjust the centering.
		m_vVertexSum -= lines.front().vSource;
//		m_vVertexSum -= lines.front().vDest;
		m_fNumSummedVertices -= 1.0f;

		// Remove the front (oldest) element.
		lines.pop_front();

		// Reduce the send offset.
		if( send_offset > 0 ) --send_offset;
	}

	// Add the new line to the system.
	lines.push_back(new_line);

	// Trim the line to the world coordinates.
	LTVector vWorldMax, vWorldMin;
	g_pServerDE->GetWorldBox(vWorldMin,vWorldMax);
	TrimToWorld( &lines.back(), vWorldMin, vWorldMax );

	// Reset the system center.

	m_vVertexSum += lines.back().vSource;
//	m_vVertexSum += lines.back().vDest;
	m_fNumSummedVertices += 1.0f;

	LTVector engine_should_use_const_ref = (m_vVertexSum/m_fNumSummedVertices);
	g_pLTServer->SetObjectPos(m_hObject, &engine_should_use_const_ref );


#ifdef LINESYSTEM_DEBUG
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject,&vPos);

	g_pLTServer->CPrint("%f Linesystem %s located at (%.2f,%.2f,%.2f).",
		g_pLTServer->GetTime(),
		g_pLTServer->GetObjectName(m_hObject),
		vPos.x, vPos.y, vPos.z );
#endif

	// Reset nextLineToSend.  This must be done even if the iterator
	// will point to the same spot.  An iterator into a deque or vector may 
	// become invalid after an insert.
	nextLineToSend = lines.begin() + send_offset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::EngineMessageFn
//
//	PURPOSE:	The engine stuff.
//
// ----------------------------------------------------------------------- //
LTRESULT DebugLineSystem::EngineMessageFn(LTRESULT messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE :
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
				}
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if( !m_hObject ) return 0;

			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			InitialUpdate();
			if (g_pServerDE) g_pServerDE->SetNextUpdate(m_hObject, s_fUpdateDelay);
			return dwRet;
		}

		case MID_UPDATE:
		{
			if( !m_hObject ) return 0;

			Update();
			if (g_pServerDE) g_pServerDE->SetNextUpdate(m_hObject, s_fUpdateDelay);
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::ReadProp
//
//	PURPOSE:	Reads creation parameters.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
	if (!pStruct) return;

	if ( g_pServerDE->GetPropGeneric( "MaxLines", &genProp ) == DE_OK )
	{
		maxLines = genProp.m_Long;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::InitialUpdate
//
//	PURPOSE:	Sets up the object's SFX message.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::InitialUpdate()
{
	//
	// Create the special fx.
	//

	// Create the message.
	RCMessage pMsg;
	if( !pMsg.null() )
	{
		// Record the ID
		pMsg->WriteByte( SFX_DEBUGLINE_ID );
	}

	// Send it.
	g_pServerDE->SetObjectSFXMessage(m_hObject, *pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Clear
//
//	PURPOSE:	Removes all lines from the system.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::Clear()
{ 
	lines.clear();
	nextLineToSend = lines.end();
	m_vVertexSum = LTVector(0,0,0);
	m_fNumSummedVertices = 0.0f;
	m_bClearOldLines = true;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Update
//
//	PURPOSE:	Checks for new lines, clients, or a clear line and sends
//				the data down to any clients.
//
// ----------------------------------------------------------------------- //
void DebugLineSystem::Update()
{
	if( !m_hObject ) return;

	if( m_nNumClients > 0
		&& (nextLineToSend != lines.end() || m_bClearOldLines) )
	{
		// Set up the message.
		RCMessage pMsg;
		if( !pMsg.null() )
		{
			// Record the ID and server object, used to route the message.
			pMsg->WriteByte( SFX_DEBUGLINE_ID );
			pMsg->WriteObject( m_hObject );

			// Record the number of entries.
			const int num_lines_left = (lines.end() - nextLineToSend);
			if( num_lines_left < s_MaxLinesPerMessage )
			{
				pMsg->WriteWord( num_lines_left );
			}
			else
			{
				pMsg->WriteWord( s_MaxLinesPerMessage );
			}

			// Record the maximum number of lines.
			pMsg->WriteWord( maxLines );

			// Tell whether we want to clear old lines or not, 
			*pMsg << m_bClearOldLines;

			// Record each entry.

			int num_lines_sent = 0;
			LTVector system_center(0,0,0);
			LTFLOAT  system_center_count = 0;
			while( nextLineToSend != lines.end() && num_lines_sent < s_MaxLinesPerMessage)
			{
				*pMsg << *nextLineToSend;

				++nextLineToSend;
				++num_lines_sent;
			}

#ifdef LINESYSTEM_DEBUG
			g_pServerDE->CPrint("Sent %d lines. %d lines left to send.", 
				num_lines_sent, lines.end() - nextLineToSend );
#endif

			// Send the message!
			g_pServerDE->SendToClient(*pMsg, MID_SFX_MESSAGE, LTNULL, MESSAGE_GUARANTEED | MESSAGE_NAGGLEFAST);

		} //if( !pMsg.null() )
		
		// If we have cleared out our lines and have no more to send,
		// why should we exist?
		if( m_bClearOldLines && lines.empty() )
		{
			g_pLTServer->RemoveObject(m_hObject);
		}

		// Reset m_bClearOldLines so that we don't re-enter this block.
		m_bClearOldLines = false;

	} //if( m_nNumClients > 0 && (nextLineToSend != lines.end() || m_bClearOldLines) )
}


void DebugLineSystem::AddBox( const LTVector & vBoxPos, const LTVector & vBoxDims, 
							   const LTVector & vColor /* = LTVector(1.0f,1.0f,1.0f) */,
							   float fAlpha /* = 1.0f */ )
{
	// Front face.
	AddLine(vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,vBoxDims.z), vColor, fAlpha );
	AddLine(vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,vBoxDims.z), vColor, fAlpha);

	// Sides.
	AddLine(vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, fAlpha);

	// Back face.
	AddLine(vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(vBoxDims.x,-vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
	AddLine(vBoxPos + LTVector(-vBoxDims.x,-vBoxDims.y,-vBoxDims.z),vBoxPos + LTVector(-vBoxDims.x,vBoxDims.y,-vBoxDims.z), vColor, fAlpha);
}



namespace LineSystem
{
	struct SystemEntry
	{
		LTSmartLink handle;
		DebugLineSystem * line_system_ptr;

		SystemEntry()
			: line_system_ptr(0) {}
	};

	typedef std::map<std::string,SystemEntry> SystemMap;

	SystemMap g_systems;

	DebugLineSystem null_line_system;

	DebugLineSystem * GetSystemPtr( SystemMap & system_map, const std::string & name)
	{
		SystemEntry & entry = system_map[name];

		if( !entry.handle.GetHOBJECT() )
		{
			entry.line_system_ptr = DebugLineSystem::Spawn(name.c_str());
			if( entry.line_system_ptr )
			{
				entry.handle = entry.line_system_ptr->m_hObject;
			}
			else
			{
				entry.line_system_ptr = &null_line_system;
				entry.handle = LTNULL;
			}

		}

		_ASSERT( entry.line_system_ptr );

		return entry.line_system_ptr;
	}

	DebugLineSystem & GetSystem(const std::string & name)
	{
		return *GetSystemPtr(g_systems,name);
	}

	DebugLineSystem & GetSystem(const void * pOwner, const std::string & name)
	{
		char buffer[20];

#ifdef _WIN32
		return *GetSystemPtr(g_systems,std::string(_ltoa(reinterpret_cast<long>(pOwner),buffer,10)) + name);
#else
		long val = reinterpret_cast<long>(pOwner);
		std::string strval;
		sprintf(buffer, "%d", val);
		strval = buffer + name;

		return *GetSystemPtr(g_systems, strval);
#endif
	}

	void ClearAll()
	{
		std::vector<std::string> remove_list;

		{for( SystemMap::iterator iter = g_systems.begin();
		     iter != g_systems.end(); ++iter )
		{
			SystemEntry & entry = iter->second;

			if( entry.handle )
			{
				*entry.line_system_ptr << LineSystem::Clear();
			}
			else
			{
				remove_list.push_back(iter->first);
			}
		}}

		{for( std::vector<std::string>::iterator iter = remove_list.begin();
			  iter != remove_list.end(); ++iter )
		{
			g_systems.erase(*iter);
		}}
	}

				
}

#endif // !_FINAL
