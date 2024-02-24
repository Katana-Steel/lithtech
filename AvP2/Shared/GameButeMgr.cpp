// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.cpp
//
// PURPOSE : GameButeMgr - Implementation
//
// CREATED : 03/30/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameButeMgr.h"

#define BUTE_DEBUG_LEVEL		0

const char * g_szCurrentAttributeFile = LTNULL;

void GBM_DisplayError(const char* szMsg)
{
    LTFLOAT fVal = 0.0f;

#ifdef _CLIENTBUILD

    if (!g_pLTClient) return;

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("DebugLevel");
	if(hVar)
	{
        fVal = g_pLTClient->GetVarValueFloat(hVar);
	}
#else

    if (!g_pLTServer) return;

    HCONVAR hVar = g_pLTServer->GetGameConVar("DebugLevel");
	if (hVar)
	{
        fVal = g_pLTServer->GetVarValueFloat(hVar);
	}
#endif

	if (fVal >= BUTE_DEBUG_LEVEL)
	{
#ifdef _CLIENTBUILD
        if (g_pLTClient)
		{
            g_pLTClient->CPrint("%s, %s", 
				g_szCurrentAttributeFile ? g_szCurrentAttributeFile : "<unknown file>",
				szMsg );
		}
#else
        if (g_pLTServer)
		{
            g_pLTServer->CPrint("%s, %s", 
				g_szCurrentAttributeFile ? g_szCurrentAttributeFile : "<unknown file>",
				szMsg );
		}
#endif
	
        TRACE("%s, %s", 
			g_szCurrentAttributeFile ? g_szCurrentAttributeFile : "<unknown file>",
			szMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameButeMgr::Parse()
//
//	PURPOSE:	Parse from a rez file
//
// ----------------------------------------------------------------------- //

LTBOOL CGameButeMgr::Parse(ILTCSBase *pInterface, const char* sButeFile)
{
	// Sanity checks...

    if (!sButeFile) return(LTFALSE);

	BOOL bRet = TRUE;

	// NOTE!!! When ALLOW_BUTE_FILE_SAVING is NOT defined, this code
	// will need to be updated to support being called from DEdit!!!!
	// (from a IObjectPlugin::PreHook_EditStringList() call)...

	LTBOOL bDirectParse = LTFALSE;

// 07/01/01 - MarkS: disabled (wasn't being used, and broke rez file distros)
// #define ALLOW_BUTE_FILE_SAVING
#if defined(ALLOW_BUTE_FILE_SAVING)

	bDirectParse = LTTRUE;

#endif  // ALLOW_BUTE_FILE_SAVING


	if(bDirectParse || !pInterface)
	{
		// If we're going to allow the bute file to be saved by the game, it must
		// be read in from a file (not a .rez file)...

		// Append the NOLF directory onto the filename if this file is normally
		// stored in the .rez file...

		if (m_bInRezFile)
		{
#ifdef _WIN32
			m_strAttributeFile.Format("AVP2\\%s", sButeFile);
#else
			m_strAttributeFile.Format("AVP2/%s", sButeFile);
#endif
		}
		else
		{
			m_strAttributeFile.Format("%s", sButeFile);
		}


		if (m_pCryptKey)
		{
			bRet = m_buteMgr.Parse(m_strAttributeFile, m_pCryptKey);
		}
		else
		{
			bRet = m_buteMgr.Parse(m_strAttributeFile);
		}

		return bRet;
	}


	m_strAttributeFile = sButeFile;

	// Open the file...

	char sConstFile[256];
	strncpy(sConstFile, sButeFile, 255);

    ILTStream* pDStream;

    LTRESULT dr = pInterface->OpenFile(sConstFile, &pDStream);

    if (dr != LT_OK) return(FALSE);


	// Read the file...

	unsigned long uLen = pDStream->GetLen();

	char* pData = new char[uLen];
	if (!pData)
	{
		pDStream->Release();
		return(FALSE);
	}

	pDStream->Read(pData, uLen);


	// Parse the file...
	g_szCurrentAttributeFile = sConstFile;
	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(pData, uLen, m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(pData, uLen);
	}
	g_szCurrentAttributeFile = LTNULL; 

	// Clean up...

	pDStream->Release();
	delete pData;


	// Check for an error...

	if (!bRet)
	{

		TRACE("CGameButeMgr::Parse() ERROR in file \"%s\"!\n",
			m_strAttributeFile.GetBuffer() );
		return(FALSE);
	}


	// All done...

	return(TRUE);
}