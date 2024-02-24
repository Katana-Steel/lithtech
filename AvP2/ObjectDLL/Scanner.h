// ----------------------------------------------------------------------- //
//
// MODULE  : Scanner.h
//
// PURPOSE : An object which scans for the player and then sends a message
//			 (based on old SecurityCamera class)
//
// CREATED : 6/7/99
//
// ----------------------------------------------------------------------- //

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "cpp_engineobjects_de.h"
#include "CharacterAlignment.h"
#include "Prop.h"

class CCharacter;

class CScanner : public Prop
{
	public :

		CScanner();
		~CScanner();

		DBOOL CScanner::CanSeeObject(ObjectFilterFn fn, HOBJECT hObject);
		DBOOL CScanner::CanSeePos(ObjectFilterFn fn, const DVector& vPos);

		CharacterClass GetCharacterClass() { return CORPORATE; }

		static DBOOL DefaultFilterFn(HOBJECT hObj, void *pUserData);
		static DBOOL BodyFilterFn(HOBJECT hObj, void *pUserData);

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		virtual DBOOL		UpdateDetect();
		virtual DRotation	GetScanRotation();
		virtual void		SetLastDetectedEnemy(HOBJECT hObj);
		
		void	SetLastDetectedDeathPos(DVector vPos) { m_vLastDetectedDeathPos = vPos; }
		void	SetDestroyedModel();

		const DVector & GetPos()                 const { return m_vPos; }
		const DVector & GetInitialPitchYawRoll() const { return m_vInitialPitchYawRoll; }
		DFLOAT GetVisualRange()            const { return m_fVisualRange; }
		HOBJECT GetLastDetectedEnemy()     const { return m_hLastDetectedEnemy; }

	private :

		DFLOAT	m_fFOV;
		DFLOAT	m_fVisualRange;

		DFLOAT	m_fDetectInterval;
		DFLOAT	m_fDetectTimer;

		DVector	m_vPos;
		DVector	m_vInitialPitchYawRoll;

		HSTRING	m_hstrDestroyedFilename;
		HSTRING	m_hstrDestroyedSkin;

		HSTRING	m_hstrSpotMessage;
		HSTRING m_hstrSpotTarget;

		HOBJECT	m_hLastDetectedEnemy;
		DVector	m_vLastDetectedDeathPos;

		//** EVERYTHING BELOW HERE DOES NOT NEED SAVING

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();
};

#endif // __SCANNER_H__
