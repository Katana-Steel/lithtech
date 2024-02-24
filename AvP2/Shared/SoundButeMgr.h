//*********************************************************************************
//*********************************************************************************
// Project:		Alien vs. Predator 2
// Purpose:		Retrieves attributes from the SoundButes.txt
//*********************************************************************************
// File:		SoundButeMgr.h
// Created:		November 29, 2000
// Updated:		November 29, 2000
// Author:		Andy Kaplan
//*********************************************************************************
//*********************************************************************************

#ifndef		_SOUND_BUTE_MGR_H_
#define		_SOUND_BUTE_MGR_H_

//*********************************************************************************

#include "HierarchicalButeMgr.h"
#include "SoundTypes.h"

#include <deque>

//*********************************************************************************

#ifdef _WIN32
	#define SOUND_BUTES_DEFAULT_FILE	"Attributes\\SoundButes.txt"
#else
	#define SOUND_BUTES_DEFAULT_FILE	"Attributes/SoundButes.txt"
#endif

#define SOUND_BUTES_STRING_SIZE		64

//*********************************************************************************

class CSoundButeMgr;
extern CSoundButeMgr* g_pSoundButeMgr;

//*********************************************************************************

struct SoundFiles
{
	typedef std::deque<char *>	SoundFileList;
	typedef std::deque<LTFLOAT>	WeightList;

	SoundFiles()
	{
		m_nNumSounds = 0;
		memset(m_szName,0,sizeof(m_szName));
		memset(m_szParent,0,sizeof(m_szParent));
	}

	~SoundFiles();

	uint32			m_nNumSounds;		// The number of sounds in the set

	// Our name in the bute file.
	char	      m_szName[SOUND_BUTES_STRING_SIZE];			// Sound FX name

	// Parent information
	char	m_szParent[SOUND_BUTES_STRING_SIZE];	// The name of the parent of this character

	// The list of sounds and their weights.
	SoundFileList m_szSounds;									// Skin filenames
	WeightList	  m_fWeights;									// Random chance weights
};


struct SoundButes
{
	SoundButes::SoundButes();

	LTFLOAT	m_fInnerRad;				// Inner sound radius
	LTFLOAT	m_fOuterRad;				// Outer sound radius
	LTFLOAT	m_fPitch;					// The pitch of the sound
	LTFLOAT	m_fPlayChance;				// The chance that a sound will play

	uint8			m_nVolume;			// The volume of the sound
	SoundPriority	m_ePriority;		// The priority of the sound

	uint32			m_nFlags;			// Sound flags

	LTBOOL			m_bLipSync;			// Will lip sync if able
	LTFLOAT			m_fMinDelay;		// min delay before playing this sound again
	LTFLOAT			m_fMaxDelay;		// max delay before playing this sound again

};

inline SoundButes::SoundButes()
{
	memset(this, 0, sizeof(SoundButes));
}

//*********************************************************************************

class CSoundButeMgr : public CHierarchicalButeMgr
{
	public:

		CSoundButeMgr();
		virtual ~CSoundButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = SOUND_BUTES_DEFAULT_FILE);
		void		Term();

		LTBOOL		WriteFile()						{ return m_buteMgr.Save(); }
		void		Reload()						{ m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumSoundButes()				const { return m_nNumSoundButes; }

		int			GetSoundSetFromName(const char *szName)		const;

		const char* GetRandomSoundFile(uint32 nSet);
		const char* GetRandomSoundFileWeighted(uint32 nSet);

		char*		GetSoundButeName(int nSet)		{ CheckSound(nSet); return m_pSoundFiles[nSet].m_szName; }
		char*		GetSoundButeParent(int nSet)	{ CheckSound(nSet); return m_pSoundFiles[nSet].m_szParent; }

		//-------------------------------------------------------------------------------//

		void		CheckSound(int nSet)			const { ASSERT(nSet < m_nNumSoundButes); ASSERT(m_pSoundButes); }

		//-------------------------------------------------------------------------------//
		// Special function to fill in a SoundButes structure

		const SoundButes & GetSoundButes(int nSet)		const { CheckSound(nSet); return m_pSoundButes[nSet];  }
		const SoundButes & GetSoundButes(const char *szName)	const;

		const SoundFiles & GetSoundFiles(int nSet) const { CheckSound(nSet); return m_pSoundFiles[nSet]; }
		const SoundFiles & GetSoundFiles(const char *szName)	const;

	private:

		static bool CountTags(const char *szTagName, void *pData);
		static bool LoadButes(const char *szTagName, void *pData);

		// Loading functions (called from Init)
		void LoadSoundButes(const char * szTagName, SoundButes * pButes, SoundFiles * pFiles);

	private:

		// Sound attribute set variables
		int				m_nNumSoundButes;
		SoundButes		*m_pSoundButes;
		SoundFiles		*m_pSoundFiles;


		IndexTable    m_IndexTable;
};

////////////////////////////////////////////////////////////////////////////
//
// CSoundButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use SoundMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CSoundButeMgrPlugin : public IObjectPlugin
{
	public:

		CSoundButeMgrPlugin()	{}

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength,
			const char* szParent = LTNULL);

	protected :

		static LTBOOL				sm_bInitted;
		static CSoundButeMgr	sm_ButeMgr;
};

#endif // _CLIENTBUILD

#endif // _SOUND_BUTE_MGR_H_
