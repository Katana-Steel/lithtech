// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ATTACHBUTE_MGR_H__
#define __ATTACHBUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"

class CAttachButeMgr;
extern CAttachButeMgr* g_pAttachButeMgr;

#ifdef _WIN32
	const char * const g_szAttachButeMgrFile  = "Attributes\\Attachments.txt";
#else
	const char * const g_szAttachButeMgrFile  = "Attributes/Attachments.txt";
#endif

class CAttachButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CAttachButeMgr();
		~CAttachButeMgr();

        LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = g_szAttachButeMgrFile);
		void		Term();

		// Attachments

		int			GetAttachmentIDByName(const char *szName);

		int			GetNumAttachments() { return m_cAttachmentID; }
		CString		GetAttachmentName(int nAttachmentID);
		int			GetAttachmentType(int nAttachmentID);
		CString		GetAttachmentClass(int nAttachmentID);
		CString		GetAttachmentProperties(int nAttachmentID);
		CString		GetAttachmentModel(int nAttachmentID);
		CString		GetAttachmentSkin(int nAttachmentID);
		LTFLOAT		GetAttachmentAlpha(int nAttachmentID);

		int			GetNumRequirements(char *szModelName);
		int			GetRequirementIDs(char *szModelName, int *pBuf, int nBufLen);
		int			GetRequirementAttachment(int nRequirementID);
		CString		GetRequirementSocket(int nRequirementID);

	private :

		int			m_cAttachmentID;
		int			m_cRequirementID;
};

#endif // __AttachBUTE_MGR_H__
