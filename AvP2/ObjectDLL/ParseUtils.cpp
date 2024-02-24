// ----------------------------------------------------------------------- //
//
// MODULE  : ParseUtils.h
//
// PURPOSE : Parsing utilities.
//
// CREATED : 9/8/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParseUtils.h"
#include "CharacterMgr.h"
#include "AI.h"
#include "AITarget.h"
#include "AINode.h"
#include "AINodeMgr.h"
#include "PlayerObj.h"
#include "iltmessage.h"
#include "AINodeMgr.h"
#include "AIUtils.h" // for operator<<(ILTMessage&,std::string)

ParseLocation::ParseLocation( const char * location_str, const CAI & ai )
	: bOff(false),
	  bTarget(false),
	  hObject(LTNULL),
	  pNode(LTNULL),
	  vPosition(FLT_MAX,FLT_MAX,FLT_MAX)
{
	ConParse parse(const_cast<char*>(location_str));

	if( LT_OK == g_pLTServer->Common()->Parse(&parse) )
	{
		if( parse.m_nArgs == 3 )
		{
			vPosition = LTVector( (LTFLOAT)atof(parse.m_Args[0]),
								  (LTFLOAT)atof(parse.m_Args[1]),
								  (LTFLOAT)atof(parse.m_Args[2]) );
		}
		else if( parse.m_nArgs == 1 )
		{
			if( 0 == stricmp(parse.m_Args[0],"off") )
			{
				bOff = true;
			}
			else if( 0 == stricmp(parse.m_Args[0],"target") )
			{
				bTarget = true;
			}
			else if( 0 == stricmp( parse.m_Args[0], "Player" ) )
			{
				CPlayerObj * pPlayer =  GetClosestPlayerPtr(ai);
				if( pPlayer )
				{
					hObject = pPlayer->m_hObject;
					vPosition = pPlayer->GetPosition();
				}
				else
				{
					_ASSERT(0);
				}
			}
			else
			{
				
				pNode = g_pAINodeMgr->GetNode(parse.m_Args[0]);
				if( pNode )
				{
					vPosition = pNode->GetPos();
				}
				else
				{
					HOBJECT hObj = LTNULL;
					const LTRESULT find_result = FindNamedObject(parse.m_Args[0],hObj);
					if( LT_OK == find_result )
					{
						hObject = hObj;
						g_pInterface->GetObjectPos(hObject, &vPosition);
					}
					else if( LT_ERROR == find_result )
					{
						error = "Multiple objects have the name \"";
						error += parse.m_Args[0];
						error += "\".";
					}
					else
					{
						error = "no object has the name \"";
						error += parse.m_Args[0];
						error += "\".";
					}
				}
			}
		}
		else
		{
			error = "Incorrect location \"";
			error += location_str;
			error += "\", can be [(# # #) | player | target | default | node_name].";
		}
	}
}


ILTMessage & operator<<(ILTMessage & out, const ParseLocation & x)
{
	out << x.error;
	out << x.bOff;
	out << x.bTarget;
	out.WriteDWord( x.pNode ? x.pNode->GetID() : CAINode::kInvalidID );
	out << x.hObject;
	out << const_cast<LTVector&>(x.vPosition);

	return out;
}

ILTMessage & operator>>(ILTMessage & in, ParseLocation & x)
{
	in >> x.error;
	in >> x.bOff;
	in >> x.bTarget;
	x.pNode = g_pAINodeMgr->GetNode( in.ReadDWord() );
	in >> x.hObject;
	in >> x.vPosition;

	return in;
}
