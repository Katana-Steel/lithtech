
#include "stdafx.h"
#include "Sprinkles.h"
#include "SFXMsgIds.h"


#define ADD_SPRINKLE_PROP(num, defFilename) \
	ADD_STRINGPROP_FLAG(Filename##num##, defFilename, PF_FILENAME)\
	ADD_STRINGPROP_FLAG(SkinName##num##, "", PF_FILENAME)\
	ADD_LONGINTPROP(Count##num##, 16)\
	ADD_REALPROP(Speed##num##, 50.0f)\
	ADD_REALPROP(Size##num##, 10.0f)\
	ADD_REALPROP_FLAG(SpawnRadius##num##, 500.0f, PF_RADIUS)\
	ADD_COLORPROP(ColorMin##num##, 255.0f, 255.0f, 255.0f)\
	ADD_COLORPROP(ColorMax##num##, 255.0f, 255.0f, 255.0f)\
	ADD_VECTORPROP_VAL(AnglesVel##num##, 1.0f, 1.0f, 1.0f)
	


BEGIN_CLASS(Sprinkles)

#ifdef _WIN32
	ADD_SPRINKLE_PROP(0, "SFX\\Snow\\SprTex\\Snow1.dtx")
#else
	ADD_SPRINKLE_PROP(0, "SFX/Snow/SprTex/Snow1.dtx")
#endif

	ADD_SPRINKLE_PROP(1, "")
	ADD_SPRINKLE_PROP(2, "")
	ADD_SPRINKLE_PROP(3, "")

	ADD_SPRINKLE_PROP(4, "")
	ADD_SPRINKLE_PROP(5, "")
	ADD_SPRINKLE_PROP(6, "")
	ADD_SPRINKLE_PROP(7, "")

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(Sprinkles, BaseClass, NULL, NULL)



// --------------------------------------------------------------------------------- //
// SprinkleType
// --------------------------------------------------------------------------------- //

SprinkleType::SprinkleType()
{
	m_hFilename = NULL;
	m_hSkinName = NULL;
	m_Count = 0;
	m_Speed = 0.0f;
}


SprinkleType::~SprinkleType()
{
	if(m_hFilename)
	{
		g_pServerDE->FreeString(m_hFilename);
		m_hFilename = NULL;
	}
}


// --------------------------------------------------------------------------------- //
// Sprinkles
// --------------------------------------------------------------------------------- //

Sprinkles::Sprinkles() : BaseClass(OT_NORMAL)
{
	m_nTypes = 0;
}


Sprinkles::~Sprinkles()
{
}


DDWORD Sprinkles::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			OnPreCreate((ObjectCreateStruct*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			OnInitialUpdate();
		}
		break;

		case MID_UPDATE:
		{
			OnUpdate();
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


void Sprinkles::OnPreCreate(ObjectCreateStruct *pStruct)
{
	char propName[64];
	GenericProp gProp;
	SprinkleType *pType;
	DDWORD i;

	pStruct->m_Flags = FLAG_FORCECLIENTUPDATE;

	m_nTypes = 0;
	for(i=0; i < MAX_SPRINKLE_TYPES; i++)
	{
		pType = &m_Types[i];

		// FilenameX
		sprintf(propName, "Filename%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) != LT_OK)
			break;

		if(gProp.m_String[0] == 0)
			break;

		pType->m_hFilename = g_pServerDE->CreateString(gProp.m_String);
		if(!pType->m_hFilename)
			break;

		// SkinNameX
		sprintf(propName, "SkinName%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) != LT_OK)
			break;

		pType->m_hSkinName = g_pServerDE->CreateString(gProp.m_String);

		// CountX
		sprintf(propName, "Count%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_Count = (DDWORD)gProp.m_Long;
			if(pType->m_Count > 255)
			{
#ifndef _FINAL
				g_pServerDE->CPrint("Warning: SprinklesFX count > 255, clamping");
#endif
				pType->m_Count = 255;
			}
		}

		// SpeedX
		sprintf(propName, "Speed%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_Speed = gProp.m_Float;
		}
		
		// SizeX
		sprintf(propName, "Size%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_Size = gProp.m_Float;
		}
		
		// SpawnRadiusX
		sprintf(propName, "SpawnRadius%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_SpawnRadius = gProp.m_Float;
		}
		
		// AnglesVelX
		sprintf(propName, "AnglesVel%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_AnglesVel = gProp.m_Vec;
		}

		// ColorMinX
		sprintf(propName, "ColorMin%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_ColorMin = gProp.m_Color;
		}
		
		// ColorMaxX
		sprintf(propName, "ColorMax%d", i);
		if(g_pServerDE->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_ColorMax = gProp.m_Color;
		}
		
		++m_nTypes;
	}
}


void Sprinkles::OnInitialUpdate()
{
	LMessage *pMsg;
	SprinkleType *pType;
	DDWORD i;

	if(g_pServerDE->Common()->CreateMessage(pMsg) == LT_OK)
	{
		pMsg->WriteByte(SFX_SPRINKLES_ID);

		pMsg->WriteDWord(m_nTypes);
		for(i=0; i < m_nTypes; i++)
		{
			pType = &m_Types[i];

			pMsg->WriteHString(pType->m_hFilename);
			pMsg->WriteHString(pType->m_hSkinName);
			pMsg->WriteByte((BYTE)pType->m_Count);
			pMsg->WriteFloat(pType->m_Speed);
			pMsg->WriteFloat(pType->m_Size);
			pMsg->WriteFloat(pType->m_SpawnRadius);
			pMsg->WriteVector(pType->m_AnglesVel);
			
			pMsg->WriteByte((BYTE)pType->m_ColorMin.x);
			pMsg->WriteByte((BYTE)pType->m_ColorMin.y);
			pMsg->WriteByte((BYTE)pType->m_ColorMin.z);

			pMsg->WriteByte((BYTE)pType->m_ColorMax.x);
			pMsg->WriteByte((BYTE)pType->m_ColorMax.y);
			pMsg->WriteByte((BYTE)pType->m_ColorMax.z);
		}

		g_pServerDE->SetObjectSFXMessage(m_hObject, *pMsg);
		pMsg->Release();
	}

}


void Sprinkles::OnUpdate()
{
}

