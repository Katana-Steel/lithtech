// ----------------------------------------------------------------------- //
//
// MODULE  : CAIPathMgr.h
//
// PURPOSE : CAIPathMgr definition
//
// CREATED : 12/30/1999
//
// ----------------------------------------------------------------------- //

#ifndef __AI_PATH_MGR_H__
#define __AI_PATH_MGR_H__

class CAI;
class CAIPath;
class CAINode;
class CAIVolume;
class CCharacter;

// Externs

extern class CAIPathMgr* g_pAIPathMgr;

// Classes

class CAIPathMgr
{
	public :

		// Ctors/Dtors/etc

		CAIPathMgr();
		~CAIPathMgr();

		void Init();
		void Term();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Path finding methods

		LTBOOL FindPath(CAI* pAI, const LTVector& vPosDest, CAIPath* pPath, LTFLOAT fMaxSearchCost = -1.0f );
		LTBOOL FindPath(CAI* pAI, const CAINode* pNodeDest, CAIPath* pPath, LTFLOAT fMaxSearchCost = -1.0f );
		LTBOOL FindPath(CAI* pAI, const CAIVolume* pVolumeDest, CAIPath* pPath, LTFLOAT fMaxSearchCost = -1.0f );
		LTBOOL FindPath(CAI* pAI, const CCharacter * pChar, CAIPath* pPath, LTFLOAT fMaxSearchCost = -1.0f );
		LTBOOL FindSequentialPath(CAI* pAI, CAIVolume* pCurrentVolume, CAIVolume* pPreviousVolume, CAIPath* pPath, LTFLOAT fMaxSearchCost = -1.0f );

};

#endif // __AI_PATH_MGR_H__
