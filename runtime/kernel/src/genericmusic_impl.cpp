#include "bdefs.h"
#include "console.h"
#include "ltpvalue.h"
#include "lith.h"
#include "genericmusic_impl.h"
#include "ltdirectmusiccontrolfile.h"
#include "iltclient.h"
//instantiate our implementation class of ILTDirectMusicMgr
define_interface(CLTDirectMusicMgr, ILTDirectMusicMgr);

//ILTClient game interface
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

#define DEFAULT_PCHANNELS	256
#include <string>

#ifndef NOLITHTECH
extern int32 g_CV_LTDMConsoleOutput;
#else
extern signed int g_CV_LTDMConsoleOutput;
extern void DebugConsoleOutput(const char* msg, uint8 nRed = 255, uint8 nBlue = 255, uint8 nGreen = 255);
#endif

#if 0
#define ACQUIRE_SAMPLE_ON_PLAY
#endif

// output error to console
void LTDMConOutError(const char *pMsg, ...)
{
	if (g_CV_LTDMConsoleOutput >= 1)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
		va_end(marker);

#ifndef NOLITHTECH
		if (msg[strlen(msg) - 1] == '\n') msg[strlen(msg) - 1] = '\0';
		con_PrintString(CONRGB(255, 0, 128), 0, msg);
#else
		DebugConsoleOutput(msg, 255, 0, 0);
#endif
	}
}

// output warning to console
void LTDMConOutWarning(const char *pMsg, ...)
{
	if (g_CV_LTDMConsoleOutput >= 2)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
		va_end(marker);

#ifndef NOLITHTECH
		if (msg[strlen(msg) - 1] == '\n') msg[strlen(msg) - 1] = '\0';
		con_PrintString(CONRGB(0, 255, 128), 0, msg);
#else
		DebugConsoleOutput(msg, 0, 255, 0);
#endif
	}
}

// output message to console
void LTDMConOutMsg(int nLevel, const char *pMsg, ...)
{
	if (g_CV_LTDMConsoleOutput >= nLevel)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
		va_end(marker);

#ifndef NOLITHTECH
		if (msg[strlen(msg) - 1] == '\n') msg[strlen(msg) - 1] = '\0';
		con_PrintString(CONRGB(128, 255, 128), 0, msg);
#else
		DebugConsoleOutput(msg, 0, 0, 0);
#endif
	}
}

// if the mgr is not initialized we fail
void LTDMConOutMgrNotInitialized(const char* sFuncName)
{
	LTDMConOutError("ERROR! LTDirectMusic not Initialized (%s)\n", sFuncName);
}

// we will fail if level has not been initialized
void LTDMConOutLevelNotInitialized(const char* sFuncName)
{
	LTDMConOutError("ERROR! LTDirectMusic not Initialized (%s)\n", sFuncName);
}

CLithChunkAllocator<CLTDirectMusicMgr::CCommandItem> CLTDirectMusicMgr::CCommandItem::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CSegment> CLTDirectMusicMgr::CSegment::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CSegmentState> CLTDirectMusicMgr::CSegmentState::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CBand> CLTDirectMusicMgr::CBand::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CStyle> CLTDirectMusicMgr::CStyle::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CDLSBank> CLTDirectMusicMgr::CDLSBank::m_ChunkAllocator;

void CLTDirectMusicMgr::HandleSegmentNotification_SegAlmostEnd(CSoundInstance** pSoundInstances)
{
	// check if this notification came from a secondary segment
	{
		// check for secondary segment in list
		CSegmentState* pSegState = m_lstSecondarySegmentsPlaying.Find(pSoundInstances);

		// if segment was found then we should not process this message
		if (pSegState != NULL)
		{
			return;
		}
	}

	// check if this notification came from a MOTIF
	{
		// check for MOTIF in list
		CSegmentState* pSegState = m_lstMotifsPlaying.Find(pSoundInstances);

		// if motif was found then we should not process this message
		if (pSegState != NULL)
		{
			return;
		}
	}

	// make sure we are on the last played primairy segment state
	// otherwise we should not be processing the command loop
	{
		// get the first segment state
		CSegmentState* pFirstSegState = m_lstPrimairySegmentsPlaying.GetFirst();

		// if there is an item in the primairy segment playing list
		if (pFirstSegState != NULL)
		{
			// if this is not a notification for the last segment state in the list
			if (pFirstSegState->GetSoundInstanceArray() != pSoundInstances)
			{
				return;
			}
		}
	}

	// holds the current command we are working on
	CCommandItem* pCurCommand;

	// check if there was a previous command
	if (m_pLastCommand != LTNULL)
	{
		// go to the next command
		pCurCommand = m_pLastCommand->Next();
	}
	else
	{
		// get the first command
		pCurCommand = m_lstCommands.GetFirst();
	}

	// loop to execute commands
	bool bDoneProcessing = false;
	while (!bDoneProcessing && (pCurCommand != LTNULL))
	{
		switch (pCurCommand->GetCommandType())
		{
		case LTDMCommandLoopToStart:
		{
			// change command pointer to a loop pointer
			CCommandItemLoopToStart* pCmdLoop = (CCommandItemLoopToStart*)pCurCommand;

			// check if we were finished looping previously
			if (pCmdLoop->GetNumLoops() == 0)
			{
				// go to the next command
				pCurCommand = pCurCommand->Next();
				break;
			}

			// check if we need to decrement the loop counter
			if (pCmdLoop->GetNumLoops() > 0)
			{
				pCmdLoop->SetNumLoops(pCmdLoop->GetNumLoops() - 1);
			}

			// check if we are finished looping
			if (pCmdLoop->GetNumLoops() == 0)
			{
				// go to the next command
				pCurCommand = pCurCommand->Next();
				break;
			}

			// if this was not the last loop
			else
			{
				// go to start of command queue
				pCurCommand = m_lstCommands.GetFirst();
			}

			break;
		}

		case LTDMCommandChangeIntensity:
		{
			// change command pointer to a loop pointer
			CCommandChangeIntensity* pCmdChangeIntensity = (CCommandChangeIntensity*)pCurCommand;

			// change the intensity
			if (ChangeIntensity(pCmdChangeIntensity->GetNewIntensity()) == LT_OK)
			{
				// Changing intensity clears the command queue, so reset the current command
				pCurCommand = m_pLastCommand;

				// we are done processing commands for now
				bDoneProcessing = true;
			}

			break;
		}

		case LTDMCommandPauseQueue:
		{
			// we are done processing commands for now
			bDoneProcessing = true;
			break;
		}

		case LTDMCommandPlaySegment:
		{
			// change command pointer to a loop pointer
			CCommandItemPlaySegment* pCmdSeg = (CCommandItemPlaySegment*)pCurCommand;
			CSegment* pSegment = pCmdSeg->GetSegment();
			CSoundInstance** instances = pSegment->GetSoundInstanceArray();

			if (instances != LTNULL)
			{
				if (PlayInstance(instances[0]))
				{
					m_lstPrimairySegmentsPlaying.CreateSegmentState(instances,
						pSegment, pSegment->GetNumInstances());
				}
			}
			// we are done processing commands for now
			bDoneProcessing = true;
			break;
		}

		case LTDMCommandStopSegment:
		{
			// change command pointer to a loop pointer
			CCommandItemStopSegment* pCmdSeg = (CCommandItemStopSegment*)pCurCommand;

			// stop this segment
			//m_pPerformance->Stop( pCmdSeg->GetSegment()->GetDMSegment(), LTNULL, 0, DMUS_SEGF_MEASURE );
			LTDMConOutWarning("STUB: LTDMCommandStopSegment\n");

			// go to the next command
			pCurCommand = pCurCommand->Next();

			break;
		}

		case LTDMCommandPlayTransition:
		{
			// change command pointer to a loop pointer
			CCommandItemPlayTransition* pCmdSeg = (CCommandItemPlayTransition*)pCurCommand;
			CTransition* transition = pCmdSeg->GetTransition();
			CSoundInstance** instances = transition->GetSoundInstanceArray();

			if (instances != LTNULL)
			{
				if (PlayInstance(instances[0]))
				{
					m_lstPrimairySegmentsPlaying.CreateSegmentState(instances,
						NULL, transition->GetNumInstances());
				}
			}

			// delete this command from the command queue
			m_lstCommands.Delete(pCurCommand);

			// set the last command to start of the command queue
			pCurCommand = NULL;

			// We're done processing
			bDoneProcessing = true;
			break;
		}

		default:
		{
			// these commands have not been implemented so just skip them
			// LTDMCommandNull
			// LTDMCommandStopPlaying
			// LTDMCommandAdjustVolume
			// LTDMCommandPlaySecondarySegment
			// LTDMCommandPlayMotif
			// LTDMCommandClearOldCommands

			// go to the next command
			pCurCommand = pCurCommand->Next();
			break;
		}
		}
	}

	// update the last command pointer
	m_pLastCommand = pCurCommand;

	// go through secondary command queue
	// just used for secondary segment and motif play commands
	pCurCommand = m_lstCommands2.GetFirst();

	// loop to execute commands
	while (pCurCommand != LTNULL)
	{
		switch (pCurCommand->GetCommandType())
		{
		case LTDMCommandPlaySecondarySegment:
		{
			// change command pointer to a loop pointer
			CCommandItemPlaySecondarySegment* pCmdSeg = (CCommandItemPlaySecondarySegment*)pCurCommand;
			CSegment* pSegment = pCmdSeg->GetSegment();
			CSoundInstance** instances = pSegment->GetSoundInstanceArray();
			if (instances != LTNULL)
			{
				if (PlayInstance(instances[0]))
				{
					m_lstSecondarySegmentsPlaying.CreateSegmentState(instances,
						pSegment, pSegment->GetNumInstances());
				}
			}

			break;
		}

		case LTDMCommandPlayMotif:
		{
			// change command pointer to a loop pointer
			CCommandItemPlayMotif* pCmdSeg = (CCommandItemPlayMotif*)pCurCommand;
			CSegment* pSegment = pCmdSeg->GetSegment();
			CSoundInstance** instances = pSegment->GetSoundInstanceArray();
			if (instances != LTNULL)
			{
				if (PlayInstance(instances[0]))
				{
					m_lstMotifsPlaying.CreateSegmentState(instances,
						pSegment, pSegment->GetNumInstances());
				}
			}

			break;
		}

		default:
		{
			break;
		}
		}

		// go to the next command
		CCommandItem* pNextCommand = pCurCommand->Next();

		// delete this command from the command queue
		m_lstCommands2.Delete(pCurCommand);

		// set the next command
		pCurCommand = pNextCommand;
	}
}

void CLTDirectMusicMgr::HandleSegmentNotification_SegEnd(CSoundInstance** pSoundInstances)
{
	// make sure directmusic segment state exists
	if (pSoundInstances == LTNULL)
		return;

	// search for a primairy segment that matches this segment state pointer
	CSegmentState* pFindSegState = m_lstPrimairySegmentsPlaying.Find(pSoundInstances);

	// if we found our primairy segment
	if (pFindSegState != LTNULL)
	{
		// clean up the segment state
		// Don't call with LTDMEnactInvalid - otherwise, instances are never stopped
		// and music cannot be looped. -RKN
		m_lstPrimairySegmentsPlaying.CleanupSegmentState(this, pFindSegState);
	}

	else
	{
		// search for a secondary segment that matches this segment pointer for this segment
		CSegmentState* pFindSegState = m_lstSecondarySegmentsPlaying.Find(pSoundInstances);

		// if we found our secondary segment
		if (pFindSegState != LTNULL)
		{
			// clean up the segment state
			m_lstSecondarySegmentsPlaying.CleanupSegmentState(this, pFindSegState);
		}

		// if we didn't find it
		else
		{
			// search for a motif segment that matches this segment pointer for this segment
			CSegmentState* pFindSegState = m_lstMotifsPlaying.Find(pSoundInstances);

			// if we found our motif segment
			if (pFindSegState != LTNULL)
			{
				// clean up the motif segment state
				m_lstMotifsPlaying.CleanupSegmentState(this, pFindSegState);
			}
		}
	}
}

CLTDirectMusicMgr::CLTDirectMusicMgr()
{
	// mgr is initially not initialized
	m_bInitialized = false;
	m_bLevelInitialized = false;

	m_pAudiopath = LTNULL;
	m_pAudiopathBuffer = LTNULL;
	//m_pReverb = LTNULL; RKNSTUB
	m_aryTransitions = LTNULL;
	m_aryIntensities = LTNULL;
	m_sWorkingDirectoryAny = LTNULL;
	m_sWorkingDirectoryControlFile = LTNULL;
	m_nNumPChannels = DEFAULT_PCHANNELS;
	m_nNumVoices = 64;
	m_nSynthSampleRate = 44100;

	//ZeroMemory(&m_ReverbParameters, sizeof(DSFXWavesReverb));
	//ZeroMemory(&m_ReverbDesc, sizeof(DSEFFECTDESC));

#ifdef NOLITHTECH
	m_sRezFileName = LTNULL;
#endif
	m_pLastCommand = LTNULL;

	m_nPaused = 0;
};

CLTDirectMusicMgr::~CLTDirectMusicMgr()
{
	// call term if the class is still initialized
	if (m_bInitialized) Term();
};

LTRESULT CLTDirectMusicMgr::Init()
{

	//	_CrtSetBreakAlloc(309);

	LTDMConOutMsg(3, "CLTDirectMusicMgr::Init\n");

	// init will fail if the Mgr is already initialized
	if (m_bInitialized)
	{
		LTDMConOutError("ERROR! CLTDirectMusicMgr alread initialized. (CLTDirectMusicMgr::Init)\n");

		return LT_ERROR;
	}

	// set up chunk allocators
	LT_MEM_TRACK_ALLOC(CCommandItem::m_ChunkAllocator.Init(100, 1), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CSegment::m_ChunkAllocator.Init(20, 1), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CSegmentState::m_ChunkAllocator.Init(10, 1), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CBand::m_ChunkAllocator.Init(5, 0), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CStyle::m_ChunkAllocator.Init(5, 0), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CDLSBank::m_ChunkAllocator.Init(5, 0), LT_MEM_TYPE_MUSIC);

	// set Mgr to initialized we have succeeded
	m_bInitialized = true;

	// return LT_OK we have succeeded
	return LT_OK;
};

LTRESULT CLTDirectMusicMgr::Term()
{
	LTDMConOutMsg(3, "CLTDirectMusicMgr::Term\n");

	// if we are not initialized then just exit
	if (!m_bInitialized)
	{
		LTDMConOutWarning("WARNING! LTDirectMusicMgr already terminated or never initialized. (CLTDirectMusicMgr::Term)\n");

		return LT_OK;
	}

	// check if the level has been terminated, if not we must terminate it
	if (m_bLevelInitialized)
	{
		TermLevel();
	}

	// terminate chunk allocators
	CCommandItem::m_ChunkAllocator.Term();
	CSegment::m_ChunkAllocator.Term();
	CSegmentState::m_ChunkAllocator.Term();
	CBand::m_ChunkAllocator.Term();
	CStyle::m_ChunkAllocator.Term();
	CDLSBank::m_ChunkAllocator.Term();

	// set initialized to false we are finished
	m_bInitialized = false;

	// return LT_OK we have succeeded
	return LT_OK;
};

LTRESULT CLTDirectMusicMgr::InitLevel(const char* sWorkingDirectory, const char* sControlFileName, const char* sDefine1,
	const char* sDefine2, const char* sDefine3)
{
	LTDMConOutMsg(3, "CLTDirectMusicMgr::InitLevel sWorkingDirectory=%s sControlFileName=%s sDefine1=%s sDefine2=%s sDeine3=%s\n",
		sWorkingDirectory, sControlFileName, sDefine1, sDefine2, sDefine3);

	// control file mgr object used for reading the control file
#ifdef NOLITHTECH
	CControlFileMgrRezFile controlFile(m_sRezFileName);
#else
	CControlFileMgrDStream controlFile;
#endif

	// if the mgr is not initialized we fail
	if (m_bInitialized == false)
	{
		LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::InitLevel");
		return LT_ERROR;
	}

	// we will fail if InitLevel has already been called and has not been terminated
	if (m_bLevelInitialized) { LTDMConOutError("ERROR! InitLevel has already been called and not terminated. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// make sure that control file name is not null
	if (sControlFileName == LTNULL) { LTDMConOutError("ERROR! Control file name not valid. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// make sure that working directory file name is not null
	if (sWorkingDirectory == LTNULL) { LTDMConOutError("ERROR! Working directory not valid. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// set working directory name
	SetWorkingDirectory(sWorkingDirectory);

	// set defines for control file if they are used
	if (sDefine1 != LTNULL) controlFile.AddDefine(sDefine1);
	if (sDefine2 != LTNULL) controlFile.AddDefine(sDefine2);
	if (sDefine3 != LTNULL) controlFile.AddDefine(sDefine3);

	// check if control file name contains no path
	if ((strchr(sControlFileName, '\\') == LTNULL) && (strchr(sControlFileName, '/') == LTNULL) &&
		(strchr(sControlFileName, ':') == LTNULL))
	{
		// create a string to hold the new control file name
		char * sNewControlFileName;
		uint32 nNewControlFileNameLen = (uint32)(strlen(sWorkingDirectory) + strlen(sControlFileName) + 2);
		LT_MEM_TRACK_ALLOC(sNewControlFileName = new char[nNewControlFileNameLen], LT_MEM_TYPE_MUSIC);
		if (sNewControlFileName == LTNULL) return LT_ERROR;

		// copy working directory to string
		LTStrCpy(sNewControlFileName, sWorkingDirectory, nNewControlFileNameLen);

		// see if we need to append a backslash to directory
		if (strlen(sWorkingDirectory) > 0)
		{
			int nLastPos = (int)strlen(sWorkingDirectory) - 1;
			if ((sWorkingDirectory[nLastPos] != '\\') &&
				(sWorkingDirectory[nLastPos] != '/') &&
				(sWorkingDirectory[nLastPos] != ':'))
			{
				// append backslash
				LTStrCat(sNewControlFileName, "\\", nNewControlFileNameLen);
			}
		}

		// append the file name
		LTStrCat(sNewControlFileName, sControlFileName, nNewControlFileNameLen);

		// initialize control file
		if (!controlFile.Init(sNewControlFileName))
		{
			LTDMConOutError("ERROR! Unable to read control file. (CLTDirectMusicMgr::InitLevel)\n");

			// delete the new control file name
			delete[] sNewControlFileName;

			return LT_ERROR;
		}

		// delete the new control file name
		delete[] sNewControlFileName;
	}
	// control file name has a path so just use it
	else
	{
		// initialize control file
		if (!controlFile.Init(sControlFileName))
		{
			LTDMConOutError("ERROR! Unable to read control file. (CLTDirectMusicMgr::InitLevel)\n");
			return LT_ERROR;
		}
	}

	// set misc variable defaults
	m_nNumIntensities = 0;
	m_nInitialIntensity = 0;
	m_nInitialVolume = 0;
	m_nVolumeOffset = 0;
	m_nNumPChannels = DEFAULT_PCHANNELS;
	m_nNumVoices = 64;
	m_nSynthSampleRate = 44100;
	m_nVolume = 1;

	// read in misc variables from control file if they are present
	controlFile.GetKeyVal(LTNULL, "NUMINTENSITIES", m_nNumIntensities);
	controlFile.GetKeyVal(LTNULL, "INITIALINTENSITY", m_nInitialIntensity);
	controlFile.GetKeyVal(LTNULL, "INITIALVOLUME", m_nInitialVolume);
	controlFile.GetKeyVal(LTNULL, "VOLUMEOFFSET", m_nVolumeOffset);
	controlFile.GetKeyVal(LTNULL, "PCHANNELS", m_nNumPChannels);
	controlFile.GetKeyVal(LTNULL, "VOICES", m_nNumVoices);
	controlFile.GetKeyVal(LTNULL, "SYNTHSAMPLERATE", m_nSynthSampleRate);

	// get the dls bank directory if user specified it and set it in the loader
	{
		CControlFileKey* pKey = controlFile.GetKey(LTNULL, "DLSBANKDIRECTORY");
		if (pKey != LTNULL)
		{
			CControlFileWord* pWord = pKey->GetFirstWord();
			if (pWord != LTNULL)
			{
				//m_pLoader->SetSearchDirectory(CLSID_DirectMusicCollection, pWord->GetVal(), false); RKNSTUB
			}
		}
	}

	// Initialize reverb parameters from control file
	InitReverb(controlFile);

	// if number of intensities is less than 1 or insensity array or transition matrix
	// were not allocated we can not proceed
	if (m_nNumIntensities < 1)
	{
		LTDMConOutError("ERROR! Number of intensities is not valid. NumIntensities = %i (CLTDirectMusicMgr::InitLevel)\n", m_nNumIntensities);

		// terminate the control file
		controlFile.Term();

		// exit function we have failed
		return LT_ERROR;
	}

	// allocate intensity array (we make one extra because we don't use the 0 index)
	LT_MEM_TRACK_ALLOC(m_aryIntensities = new CIntensity[m_nNumIntensities + 1], LT_MEM_TYPE_MUSIC);

	// set up default intensity array values
	{
		// loop through all intensities in array
		for (int nLoop = 0; nLoop <= m_nNumIntensities; nLoop++)
		{
			m_aryIntensities[nLoop].SetNumLoops(0);
			m_aryIntensities[nLoop].SetIntensityToSetAtFinish(0);
		}
	}

	// calculate the number of transitions (we waste a column in each dimension beacuse
	// we don't use the zero index of the intensity array)
	//
	// Add 1 to take into account transitioning from the last intensity to the last
	// intensity.  This is required because the GetTransition function appears to be
	// adding 1 to ignore the first element in the table.
	m_nNumTransitions = (m_nNumIntensities + 1)*(m_nNumIntensities + 1) + 1;

	// allocate transition matrix
	LT_MEM_TRACK_ALLOC(m_aryTransitions = new CTransition[m_nNumTransitions], LT_MEM_TYPE_MUSIC);

	// set up default transition matrix values
	{
		// loop through all transitions in array
		for (int nLoop = 0; nLoop < m_nNumTransitions; nLoop++)
		{
			m_aryTransitions[nLoop].SetEnactTime(LTDMEnactNextMeasure);
			m_aryTransitions[nLoop].SetManual(true);
			m_aryTransitions[nLoop].SetSoundInstances(LTNULL, 0);
		}
	}

	// if insensity array or transition matrix were not allocated we can not proceed
	if ((m_aryIntensities == LTNULL) || (m_aryTransitions == LTNULL))
	{
		LTDMConOutError("ERROR! Unable to allocate memory for Intensity and Transition arrays. (CLTDirectMusicMgr::InitLevel)\n", m_nNumIntensities);

		// terminate the control file
		controlFile.Term();

		// free the intensity array
		if (m_aryIntensities != LTNULL)
		{
			delete[] m_aryIntensities;
			m_aryIntensities = LTNULL;
		}

		// free the transition matrix
		if (m_aryTransitions != LTNULL)
		{
			delete[] m_aryTransitions;
			m_aryTransitions = LTNULL;
		}

		// exit function we have failed
		return LT_ERROR;
	}

	// read in and set up DLS Banks
	ReadDLSBanks(controlFile);

	// read in and load styles and bands
	ReadStylesAndBands(controlFile);

	// read in intensity descriptions
	ReadIntensities(controlFile);

	// read in secondary segments
	ReadSecondarySegments(controlFile);

	// read in motifs
	ReadMotifs(controlFile);

	// read in transition matrix
	ReadTransitions(controlFile);

	// set level initialized to true we have succeeded
	m_bLevelInitialized = true;

	// terminate the control file mgr
	controlFile.Term();

	// Set the initial volume
	SetVolume(m_nInitialVolume);

	// set the last command to LTNULL because it hasn't been done yet
	m_pLastCommand = LTNULL;

	// set up the current intensity
	m_nCurIntensity = 0;

	// set up the previous intensity
	m_nPrevIntensity = 0;

	// return LT_OK we have succeeded
	return LT_OK;
};

LTRESULT CLTDirectMusicMgr::TermLevel()
{
	LTDMConOutMsg(3, "CLTDirectMusicMgr::TermLevel\n");

	// if the mgr is not initialized we fail
	if (m_bInitialized == false)
	{
		LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::TermLevel");
		return LT_ERROR;
	}

	// if level is not initialized just exit
	if (!m_bLevelInitialized)
	{
		LTDMConOutWarning("WARNING! Level already terminated or never initialized. (CLTDirectMusicMgr::TermLevel)\n");

		return LT_OK;
	}

	// clear the command queue
	ClearCommands();

	// stop music playing
	Stop();

	// terminate any reverb effects
	TermReverb();

	// release all primairy and secondary segments
	m_lstSegments.CleanupSegments(this);

	// release all motif segments
	m_lstMotifs.CleanupSegments(this);

	// clean up all the primairy segment states
	m_lstPrimairySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all the secondary segments
	m_lstSecondarySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all the motif segments
	m_lstMotifsPlaying.CleanupSegmentStates(this);

	// unload all bands
	{
		CBand* pBand;
		while ((pBand = m_lstBands.GetFirst()) != LTNULL)
		{
			// remove our Band object from the Band list
			m_lstBands.Delete(pBand);

			// delete our Band object
			delete pBand;
		}
	}

	// rlease all style files
	{
		CStyle* pStyle;
		while ((pStyle = m_lstStyles.GetFirst()) != LTNULL)
		{

			// remove our style object from the style list
			m_lstStyles.Delete(pStyle);

			// delete our style object
			delete pStyle;
		}
	}

	// unload all dls banks
	{
		CDLSBank* pDLSBank;
		while ((pDLSBank = m_lstDLSBanks.GetFirst()) != LTNULL)
		{
			// remove our DLSBank object from the DLSBank list
			m_lstDLSBanks.Delete(pDLSBank);

			// delete our DLSBank object
			delete pDLSBank;
		}
	}

	// delete working directory string for any type
	if (m_sWorkingDirectoryAny != LTNULL)
	{
		delete[] m_sWorkingDirectoryAny;
		m_sWorkingDirectoryAny = LTNULL;
	}

	// delete working directory string for control files
	if (m_sWorkingDirectoryControlFile != LTNULL)
	{
		delete[] m_sWorkingDirectoryControlFile;
		m_sWorkingDirectoryControlFile = LTNULL;
	}

	// let loader clean up it's object lists
	//RKNSTUB
	//if (m_pLoader != LTNULL)
	//{
	//	m_pLoader->ClearObjectList();
	//}

#ifdef NOLITHTECH
	// delete working directory string for any type
	if (m_sRezFileName != LTNULL)
	{
		delete[] m_sRezFileName;
		m_sRezFileName = LTNULL;
	}
#endif

	// free the intensity array
	if (m_aryIntensities != LTNULL)
	{
		delete[] m_aryIntensities;
		m_aryIntensities = LTNULL;
	}

	// free the transition matrix
	if (m_aryTransitions != LTNULL)
	{
		delete[] m_aryTransitions;
		m_aryTransitions = LTNULL;
	}

	// flush the directmusic cache here!!!!!

	// set level initalized to false we are done
	m_bLevelInitialized = false;

	return LT_OK;
};


LTRESULT CLTDirectMusicMgr::Play()
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::Play\n");

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::Play"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::Play"); return LT_ERROR; }

	// set the intensity to the initial intensity
	return ChangeIntensity(m_nInitialIntensity, LTDMEnactImmediately);
}

LTRESULT CLTDirectMusicMgr::Stop(const LTDMEnactTypes nStart)
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::Stop nStart=%i\n", (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::Stop"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::Stop"); return LT_ERROR; }

	// Don't be paused
	while (m_nPaused)
		UnPause();


	// clear command queue
	{
		CCommandItem* pCommand;
		while ((pCommand = m_lstCommands.GetFirst()) != LTNULL)
		{
			// remove our Command object from the Command list
			m_lstCommands.Delete(pCommand);

			// delete our Command object
			delete pCommand;
		}
	}

	// clear 2nd command queue
	{
		CCommandItem* pCommand;
		while ((pCommand = m_lstCommands2.GetFirst()) != LTNULL)
		{
			// remove our Command object from the Command list
			m_lstCommands2.Delete(pCommand);

			// delete our Command object
			delete pCommand;
		}
	}

	// clean up all primairy segment sates
	m_lstPrimairySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all secondary segment states
	m_lstSecondarySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all motif segment states
	m_lstMotifsPlaying.CleanupSegmentStates(this);

	// set last commnd to LTNULL
	m_pLastCommand = LTNULL;

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::Pause(const LTDMEnactTypes nStart)
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::Pause nStart=%i\n", (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::Pause"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::Pause"); return LT_ERROR; }

	if (m_nPaused == 0)
	{
		//RKNSTUB
	}

	++m_nPaused;

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::UnPause()
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::UnPause\n");

	if (m_bInitialized)
	{
		return LT_ERROR;
	}

	if (m_bLevelInitialized)
	{
		return LT_ERROR;
	}

	if (!m_nPaused)
		return LT_OVERFLOW;

	--m_nPaused;
	if (m_nPaused == 0)
	{
		//RKNSTUB

		// Release the stuff we saved so we could pause
		m_cCurPauseState.Clear();
	}

	return LT_OK;
}



LTRESULT CLTDirectMusicMgr::SetVolume(const long nVolume)
{
	CSegment* pSegment;
	CSoundInstance** array;
	float fVolume = 0.0f;
	unsigned int i, num_instances;
	fVolume = 100.0f / 7500.0f;
	fVolume *= ((float)nVolume + 2500.0f);
	m_nVolume = (uint16)fVolume;

	pSegment = m_lstSegments.GetFirst();

	while (pSegment != LTNULL)
	{
		num_instances = pSegment->GetNumInstances();
		array = pSegment->GetSoundInstanceArray();

		for (i = 0; i < num_instances; i++)
		{
			array[i]->SetVolumeNoMultiplier(m_nVolume);
		}

		pSegment = pSegment->Next();
	}

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::ChangeIntensity(const int nNewIntensity, const LTDMEnactTypes nStart)
{
	CTransition* pTransition;
	CSegment* pSegment = LTNULL;
	CSegment* pQueuedSegStart = LTNULL;
	uint32 nFlags;
	CSoundInstance* instance;
	CSoundInstance** array;
	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);
	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_nPriority = 0;
	psi.m_UserData = 1;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::ChangeIntensity nNewIntensity=%i nStart=%i\n", nNewIntensity, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::ChangeIntensity"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::ChangeIntensity"); return LT_ERROR; }

	// if we asked for an intensity of 0 we really meant stop so stop sounds
	if (nNewIntensity == 0)
	{
		Stop(nStart);
		return LT_ERROR;
	}

	// we fail if intensity is not valid
	if ((nNewIntensity <= 0) || (nNewIntensity > m_nNumIntensities)) return LT_ERROR;

	// we don't need to do anything if the new intensity is the same as the current intensity
	if (m_nCurIntensity == nNewIntensity)
	{
		LTDMConOutMsg(4, "CLTDirectMusicMgr::ChangeIntensity new intensity same as old nothing to do, exiting.\n");

		return LT_OK;
	}

	// get the transition that we will be using

	// if we have a value for the previous transition,
	// then we're waiting for the new one to start
	// and we can't hear it yet, therefore,
	// grab the transition for the old intensity, so it sounds right.
	if (m_nPrevIntensity)
		pTransition = GetTransition(m_nPrevIntensity, nNewIntensity);
	else
		pTransition = GetTransition(m_nCurIntensity, nNewIntensity);

	// keep track of the last intensity in case
	// we have to make a change before the new one
	// starts, so we can play the correct transition
	m_nPrevIntensity = m_nCurIntensity;

	// make sure transition is valid
	if (pTransition == LTNULL)
	{
		LTDMConOutError("ERROR! Invalid transition found, exiting. (CLTDirectMusicMgr::ChangeIntensity)\n");

		return LT_ERROR;
	}

	// clear command queue
	ClearCommands();

	// this is the enact time value we are going to use
	LTDMEnactTypes nEnactValue;

	// figure out which enact value to use
	if (nStart == LTDMEnactInvalid) nEnactValue = pTransition->GetEnactTime();
	else nEnactValue = nStart;

	// figure out the correct flags to pass into PlaySegment based on the enact time
	nFlags = EnactTypeToFlags(nEnactValue);

	bool bSkipFirstInQueue = false;

	// if transition segment is not LTNULL then play it
	array = pTransition->GetSoundInstanceArray();
	if (array != LTNULL)
	{
		// play segment
		instance = array[0];
		if (PlayInstance(instance))
		{
			m_lstPrimairySegmentsPlaying.CreateSegmentState(array,
				NULL, pTransition->GetNumInstances());
		}

		// Queue the segments for this intensity
		pQueuedSegStart = m_aryIntensities[nNewIntensity].GetSegmentList().GetFirst();
	}
	// if there is no transition segment then play the first segment in the command queue
	else
	{
		// Play the first intensity segment
		pSegment = m_aryIntensities[nNewIntensity].GetSegmentList().GetFirst();

		while (pSegment != LTNULL)
		{
			array = pSegment->GetSoundInstanceArray();
			instance = (array != LTNULL) ? array[0] : LTNULL;

			if (instance != LTNULL)
			{
				if (PlayInstance(instance))
				{
					m_lstPrimairySegmentsPlaying.CreateSegmentState(array, pSegment, pSegment->GetNumInstances());
				}

				pQueuedSegStart = pSegment;
				bSkipFirstInQueue = true;
				break;
			}
			else
			{
				pSegment = pSegment->Next();
			}
		}
		// If there's nothing valid to queue, don't.
		if (!pSegment)
			pQueuedSegStart = LTNULL;
	}

	// Queue the remaining segments for this intensity
	while (pQueuedSegStart != LTNULL)
	{
		if (pQueuedSegStart->GetSoundInstanceArray() != LTNULL)
		{
			// create a new command queue play segment item
			CCommandItemPlaySegment* pComItemSeg;
			LT_MEM_TRACK_ALLOC(pComItemSeg = new CCommandItemPlaySegment, LT_MEM_TYPE_MUSIC);

			// set the directmusic segment inside the new item
			pComItemSeg->SetSegment(pQueuedSegStart);

			// add the new item to the command queue
			m_lstCommands.InsertLast(pComItemSeg);

		}
		// get the next segment
		pQueuedSegStart = pQueuedSegStart->Next();
	}

	// Start the queue over next time something is processed
	if (bSkipFirstInQueue)
		m_pLastCommand = m_lstCommands.GetFirst();
	else
		m_pLastCommand = LTNULL;

	// add a loop command to the end of the command queue
	// create a new command to loop
	CCommandItemLoopToStart* pComItemLoop;
	LT_MEM_TRACK_ALLOC(pComItemLoop = new CCommandItemLoopToStart, LT_MEM_TYPE_MUSIC);

	// set the number of times to loop (-1 for infinite)
	int nNumLoops = m_aryIntensities[nNewIntensity].GetNumLoops();
	// If we're going to run this intensity again, do an infinite loop
	if (m_aryIntensities[nNewIntensity].GetIntensityToSetAtFinish() == nNewIntensity)
		nNumLoops = -1;
	pComItemLoop->SetNumLoops(nNumLoops);

	// add the new item to the command queue
	m_lstCommands.InsertLast(pComItemLoop);

	// add a change intensity command to the end of the command queue if it is present
	if (m_aryIntensities[nNewIntensity].GetIntensityToSetAtFinish() > 0)
	{
		// create a new command to change intensity
		CCommandChangeIntensity* pComItemChangeIntensity;
		LT_MEM_TRACK_ALLOC(pComItemChangeIntensity = new CCommandChangeIntensity, LT_MEM_TYPE_MUSIC);

		// set the intensity to change to
		pComItemChangeIntensity->SetNewIntensity(m_aryIntensities[nNewIntensity].GetIntensityToSetAtFinish());

		// add the new item to the command queue
		m_lstCommands.InsertLast(pComItemChangeIntensity);
	}

	// set the new intensity
	m_nCurIntensity = nNewIntensity;

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::PlaySecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart)
{
	CSegment* pMainSegment;
	CSoundInstance** instances;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::PlaySecondary sSecondarySegment=%s nStart=%i\n", sSecondarySegment, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::PlaySecondary"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::PlaySecondary"); return LT_ERROR; }

	// find the main segment
	pMainSegment = m_lstSegments.Find(sSecondarySegment);

	// if the segment is not found we can't play it so fail
	if (pMainSegment == LTNULL) return LT_ERROR;

	instances = pMainSegment->GetSoundInstanceArray();

	if (instances == LTNULL) return LT_ERROR;

	// if default enact time was passed in then get default enact time from the original segment
	if (nStart == LTDMEnactDefault) nEnactTime = pMainSegment->GetDefaultEnact();

	// play the new segment (unless we are not supposed to play until the next segment)
	if (nEnactTime != LTDMEnactNextSegment)
	{
		if (PlayInstance(instances[0]))
		{
			m_lstSecondarySegmentsPlaying.CreateSegmentState(instances, pMainSegment, pMainSegment->GetNumInstances());
		}
	}

	// if we are supposed to wait until the end of the segment then queue up a play secondary command
	else
	{
		// create a new command queue play segment item
		CCommandItemPlaySecondarySegment* pComItemSeg;
		LT_MEM_TRACK_ALLOC(pComItemSeg = new CCommandItemPlaySecondarySegment, LT_MEM_TYPE_MUSIC);

		// make sure allocation worked
		if (pComItemSeg != LTNULL)
		{
			// set the directmusic segment inside the new item
			pComItemSeg->SetSegment(pMainSegment);

			// add the new item to the command queue so it will be executed next when a segment ends
			m_lstCommands2.Insert(pComItemSeg);
		}
	}

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::StopSecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart)
{
	CSegmentState* pStopSegmentState;
	CSegmentState* pNextSegmentState;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::StopSecondary sSecondarySegment=%s nStart=%i\n", sSecondarySegment, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::StopSecondary"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::StopSecondary"); return LT_ERROR; }

	// if name is LTNULL then stop them all
	if (sSecondarySegment == LTNULL)
	{
		// get the first secondary segment
		pStopSegmentState = m_lstSecondarySegmentsPlaying.GetFirst();

		while (pStopSegmentState != LTNULL)
		{
			// get the next segment to process after this one
			pNextSegmentState = pStopSegmentState->Next();

			// make sure the sound instance is OK if not leave
			if (pStopSegmentState->GetSoundInstanceArray() != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// clean up this segment (unless it is supposed to play to the end)
				if (nEnactTime != LTDMEnactNextSegment) m_lstSecondarySegmentsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);

				// update stop segment to next segment
				pStopSegmentState = pNextSegmentState;
			}
		}
	}

	// find the one we want to stop
	else
	{
		// loop until all of the segments with this name have been found
		for (;;)
		{

			// find the secondary segment
			pStopSegmentState = m_lstSecondarySegmentsPlaying.Find(sSecondarySegment);

			// make sure we found something
			if (pStopSegmentState != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// make sure the sound instance is OK if not leave
				if (pStopSegmentState->GetSegment()->GetSoundInstanceArray() != LTNULL)
				{
					// clean up this segment (unless it is supposed to play to the end)
					if (nEnactTime != LTDMEnactNextSegment) m_lstSecondarySegmentsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);
				}
			}

			// time to exit loop
			else break;
		}
	}

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::PlayMotif(const char* sMotifName, const LTDMEnactTypes nStart)
{
	CSegment* pMainSegment;
	CSoundInstance** instances;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::PlayMotif sMotifName=%s nStart=%i\n", sMotifName, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::PlayMotif"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::PlayMotif"); return LT_ERROR; }

	// find the main segment
	pMainSegment = m_lstMotifs.Find(sMotifName);

	// if the segment is not found we can't play it so fail
	if (pMainSegment == LTNULL) return LT_ERROR;

	instances = pMainSegment->GetSoundInstanceArray();
	if (instances == LTNULL) return LT_ERROR;

	// if default enact time was passed in then get default enact time from the original segment
	if (nStart == LTDMEnactDefault) nEnactTime = pMainSegment->GetDefaultEnact();

	// play the new segment (unless we are not supposed to play until the next segment)
	if (nEnactTime != LTDMEnactNextSegment)
	{
		// play segment
		if (PlayInstance(instances[0]))
		{
			// add segment state to list of motif segment states that are playing
			m_lstMotifsPlaying.CreateSegmentState(instances, pMainSegment, pMainSegment->GetNumInstances());
		}
	}

	// if we are supposed to wait until the end of the segment then queue up a play secondary command
	else
	{
		// create a new command queue play segment item
		CCommandItemPlayMotif* pComItemSeg;
		LT_MEM_TRACK_ALLOC(pComItemSeg = new CCommandItemPlayMotif,LT_MEM_TYPE_MUSIC);

		// make sure allocation worked
		if (pComItemSeg != LTNULL)
		{
			// set the directmusic segment inside the new item
			pComItemSeg->SetSegment(pMainSegment);

			// add the new item to the command queue so it will be executed next when a segment ends
			m_lstCommands2.Insert(pComItemSeg);
		}
	}

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::PlayMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart)
{
	return PlayMotif(sMotifName, nStart);
}

LTRESULT CLTDirectMusicMgr::StopMotif(const char* sMotifName, const LTDMEnactTypes nStart)
{
	CSegmentState* pStopSegmentState;
	CSegmentState* pNextSegmentState;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::StopMotif sMotifName=%s nStart=%i\n", sMotifName, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::StopMotif"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::StopMotif"); return LT_ERROR; }

	// if name is LTNULL then stop them all
	if (sMotifName == LTNULL)
	{
		// get the first secondary segment
		pStopSegmentState = m_lstMotifsPlaying.GetFirst();

		while (pStopSegmentState != LTNULL)
		{
			// get the next segment to process after this one
			pNextSegmentState = pStopSegmentState->Next();

			if (pStopSegmentState->GetSoundInstanceArray() != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// clean up this segment (unless it is supposed to play to the end)
				if (nEnactTime != LTDMEnactNextSegment) m_lstMotifsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);

				// update stop segment to next segment
				pStopSegmentState = pNextSegmentState;
			}
		}
	}

	// find the one we want to stop
	else
	{
		// loop until all of the segments with this name have been found
		for(;;)
		{

			// find the secondary segment
			pStopSegmentState = m_lstMotifsPlaying.Find(sMotifName);

			// make sure we found something
			if (pStopSegmentState != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				if (pStopSegmentState->GetSegment()->GetSoundInstanceArray() != LTNULL)
				{
					// clean up this segment (unless it is supposed to play to the end)
					if (nEnactTime != LTDMEnactNextSegment) m_lstMotifsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);
				}
			}

			// time to exit loop
			else break;
		}
	}

	return LT_OK;
}

LTRESULT CLTDirectMusicMgr::StopMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart)
{
	return StopMotif(sMotifName, nStart);
}

void CLTDirectMusicMgr::SetWorkingDirectory(const char* sWorkingDirectory, LTDMFileTypes nFileType)
{
	// store the any type locally
	if (nFileType == LTDMFileTypeAny)
	{
		// remove the old value
		if (m_sWorkingDirectoryAny != LTNULL)
		{
			delete[] m_sWorkingDirectoryAny;
			m_sWorkingDirectoryAny = LTNULL;
		}

		// store the new value if it is not null
		if (sWorkingDirectory != LTNULL)
		{
			LT_MEM_TRACK_ALLOC(m_sWorkingDirectoryAny = new char[strlen(sWorkingDirectory) + 1], LT_MEM_TYPE_MUSIC);
			strcpy(m_sWorkingDirectoryAny, sWorkingDirectory);
		}
	}

	// store the control file type locally
	if (nFileType == LTDMFileTypeControlFile)
	{
		// remove the old value
		if (m_sWorkingDirectoryControlFile != LTNULL)
		{
			delete m_sWorkingDirectoryControlFile;
			m_sWorkingDirectoryControlFile = LTNULL;
		}

		// store the new value if it is not null
		if (sWorkingDirectory != LTNULL)
		{
			LT_MEM_TRACK_ALLOC(m_sWorkingDirectoryControlFile = new char[strlen(sWorkingDirectory) + 1], LT_MEM_TYPE_MUSIC);
			strcpy(m_sWorkingDirectoryControlFile, sWorkingDirectory);
		}
	}

	// convert file type to GUID type
	//RKNSTUB
	//~ CLSID m_clsID = GUID_NULL;
	//~ switch (nFileType)
	//~ {
	//~ case LTDMFileTypeAny: { m_clsID = GUID_DirectMusicAllTypes; break; }
	//~ case LTDMFileTypeDLS: { m_clsID = CLSID_DirectMusicCollection; break; }
	//~ case LTDMFileTypeStyle: { m_clsID = CLSID_DirectMusicStyle; break; }
	//~ case LTDMFileTypeSegment: { m_clsID = CLSID_DirectMusicSegment; break; }
	//~ case LTDMFileTypeChordMap: { m_clsID = CLSID_DirectMusicChordMap; break; }
	//~ }

	// set the search directory for directmusic if GUID is valid
	//RKNSTUB
	//if (m_clsID != GUID_NULL && m_pLoader != LTNULL)
	//{
	//	m_pLoader->SetSearchDirectory(m_clsID, sWorkingDirectory, false);
//	}
}


#ifdef NOLITHTECH

void CLTDirectMusicMgr::SetRezFile(const char* sRezFile)
{
	// remove the old value
	if (m_sRezFileName != LTNULL)
	{
		delete[] m_sRezFileName;
		m_sRezFileName = LTNULL;
	}

	// store the new value if it is not null
	if (sRezFile != LTNULL)
	{
		LT_MEM_TRACK_ALLOC(m_sRezFileName = new char[strlen(sRezFile) + 1], LT_MEM_TYPE_MUSIC);
		strcpy(m_sRezFileName, sRezFile);
	}

	m_pLoader->SetRezFile(sRezFile);
}
#endif

bool CLTDirectMusicMgr::LoadSegment(const char* sSegmentName)
{
	int hr = 1;
	ILTSoundSys* sys = NULL;
	CSegment* pSegment;
	std::vector<std::string> wavs;
	std::string full_file, wav_full_name;
	unsigned int w, wav_len;
	char file[3];
	ILTStream* stream;
	// FileRef playSoundFileRef;
	FileIdentifier* pIdent;
	CSoundMgr* soundmgr = GetClientILTSoundMgrImpl();
	PlaySoundInfo psi;
	CSoundBuffer* pSoundBuffer;
	CSoundInstance** pSoundInstances = LTNULL;
	LHSAMPLE sample;

	full_file = m_sWorkingDirectoryAny;
	full_file += "\\";
	full_file += sSegmentName;

	PLAYSOUNDINFO_INIT(psi);
	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= PLAYSOUND_CLIENT; //needed for ReadObjectSubPacket to work
	psi.m_nPriority = 0;
	psi.m_UserData = 1;

	// first check and see if this segment has already been loaded
	if (m_lstSegments.Find(sSegmentName) != LTNULL)
	{
		// it has already been loaded to we don't need to load it again just exit happily
		return true;
	}


	ilt_client->OpenFile(full_file.c_str(), &stream);
	if (!stream)
	{
		return false;
	}

	while (stream->Read(file, 1) == LT_OK)
	{
		if (file[0] == 'f')
		{
			stream->Read(file, 3);
			if (!strncmp(file, "ile", 3))
			{
				stream->Read(&wav_len, sizeof(unsigned int));
				if (wav_len > 0)
				{
					wav_full_name = "";
					for (w = 0; w < wav_len; w += 2)
					{
						stream->Read(file, 1);
						wav_full_name += file[0];
						stream->Read(file, 1); //ignore blank
					}
					wav_full_name = std::string(m_sWorkingDirectoryAny) + "\\" + wav_full_name;
					wavs.push_back(wav_full_name);
				}
			}
		}
	}

	stream->Release();

	if (wavs.size() > 0)
	{

		pSoundInstances = new CSoundInstance*[wavs.size()];
		//only allow 1 wav per segment instead of wavs.size() for now
		for (w = 0; w < wavs.size(); w++)
		{
			FileRef playSoundFileRef{FILE_CLIENTFILE, wavs[w].c_str()};
			pIdent = client_file_mgr->GetFileIdentifier(&playSoundFileRef, TYPECODE_SOUND);

			if (pIdent)
			{
				strncpy(psi.m_szSoundName, playSoundFileRef.m_pFilename, _MAX_PATH);
				pSoundBuffer = soundmgr->CreateBuffer(*pIdent);
				pSoundInstances[w] = new CLocalSoundInstance();
				pSoundInstances[w]->Init(*pSoundBuffer, psi, 0);
#ifndef ACQUIRE_SAMPLE_ON_PLAY
				sys = GetSoundSys();
				sample = sys->AllocateSampleHandle(NULL);
				sys->SetSampleUserData(sample, SAMPLE_TYPE, SAMPLETYPE_SW);
				pSoundInstances[w]->SetSampleHandle(sample);
				pSoundInstances[w]->AcquireSample();
#endif

			}
			else
			{
				hr = 0;
			}
		}
	}


	// check if the load succeeded
	if (hr > 0)
	{
		// make a new segment item
		LT_MEM_TRACK_ALLOC(pSegment = new CSegment, LT_MEM_TYPE_MUSIC);
		if (pSegment == LTNULL) return false;
		// set the new segment up
		pSegment->SetSegmentName(sSegmentName);
		pSegment->SetSoundInstances(pSoundInstances, wavs.size());
		m_lstSegments.Insert(pSegment);

		// output debut info
		LTDMConOutMsg(3, "LTDirectMusic loaded segment %s\n", sSegmentName);
		return true;
	}
	else
	{
		LTDMConOutWarning("WARNING! LTDirectMusic failed to load segment %s\n", sSegmentName);
 		return false;
	}
}

bool CLTDirectMusicMgr::LoadDLSBank(const char* sFileName)
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::LoadStyleAndBands(char* sStyleFileName, CControlFileMgr& controlFile)
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::LoadBand(void* pStyle, const char* sBandName)
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::ReadDLSBanks(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "DLSBANK");
	CControlFileWord* pWord;

	// loop through all DLSBANK keys
	while (pKey != LTNULL)
	{
		// get the first value
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// load the DLS Bank
			LoadDLSBank(pWord->GetVal());
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

bool CLTDirectMusicMgr::ReadStylesAndBands(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "STYLE");
	CControlFileWord* pWord;

	// loop through all STYLE keys
	while (pKey != LTNULL)
	{
		// get the first value
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// load the style and any associated bands
			LoadStyleAndBands(pWord->GetVal(), controlFile);
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

bool CLTDirectMusicMgr::ReadIntensities(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "INTENSITY");
	CControlFileWord* pWord;
	int nIntensity;
	int nLoop;
	int nNextIntensity;
	char* seg_name = NULL;

	// loop through all INTENSITY keys
	while (pKey != LTNULL)
	{
		// get the first word which is the intensity number
		pWord = pKey->GetFirstWord();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the value for the intensity
		nIntensity = 0;
		pWord->GetVal(nIntensity);

		// make sure the intensity number is valid
		if ((nIntensity <= 0) || (nIntensity > m_nNumIntensities))
		{
			// this is an invalid intensity go on to the next one
			// TO DO!!! Add OutputDebug information here and other places in this function!
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which is the loop value
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the loop value
		nLoop = -1;
		pWord->GetVal(nLoop);

		// get the next word with is the next intensity to switch to when this one finishes
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next intensity to switch to value
		nNextIntensity = 0;
		pWord->GetVal(nNextIntensity);

		// get the next word which is the first segment name for this intensity
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// loop through all segment names and add them to the intensity and load them in
		while (pWord != LTNULL)
		{
			// load in the segment
			LoadSegment(pWord->GetVal());

			// find the loaded segment in the master segment list
			CSegment* pMasterSegment = m_lstSegments.Find(pWord->GetVal());
			if (pMasterSegment == LTNULL) break;
			if (pMasterSegment->GetNumInstances() == 0) break;

			// add the segment to this intensity
			CSegment* pNewSeg;
			LT_MEM_TRACK_ALLOC(pNewSeg = new CSegment, LT_MEM_TYPE_MUSIC);
			if (pNewSeg == LTNULL) break;
			pNewSeg->SetSoundInstances(pMasterSegment->GetSoundInstanceArray(),
				pMasterSegment->GetNumInstances());
			//~ pNewSeg->SetSoundInstance(pMasterSegment->GetSoundInstance());
			pNewSeg->SetSegmentName(pWord->GetVal());
			m_aryIntensities[nIntensity].GetSegmentList().InsertLast(pNewSeg);

			// get the next word which is the next segment
			pWord = pWord->Next();
		}

		// set the other values in our intensity
		m_aryIntensities[nIntensity].SetNumLoops(nLoop);
		m_aryIntensities[nIntensity].SetIntensityToSetAtFinish(nNextIntensity);

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

bool CLTDirectMusicMgr::ReadSecondarySegments(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "SECONDARYSEGMENT");
	CControlFileWord* pWord;

	// loop through all SECONDARYSEGMENT keys
	while (pKey != LTNULL)
	{
		// get the first value
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// load the segment
			LoadSegment(pWord->GetVal());

			// find the loaded segment in the master segment list
			CSegment* pMasterSegment = m_lstSegments.Find(pWord->GetVal());
			if (pMasterSegment != LTNULL)
			{
				// get the next word which is the first
				pWord = pWord->Next();
				if (pWord != LTNULL)
				{
					// set the enact time in the segment
					pMasterSegment->SetDefaultEnact(StringToEnactType(pWord->GetVal()));
				}
			}
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

bool CLTDirectMusicMgr::ReadMotifs(CControlFileMgr& controlFile)
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::ReadTransitions(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "TRANSITION");
	CControlFileWord* pWord;
	int nTransitionFrom;
	int nTransitionTo;
	LTDMEnactTypes m_nEnactTime;
	bool m_bManual;
	CTransition* transition;

	// loop through all TRANSITION keys
	while (pKey != LTNULL)
	{
		// get the first word which is the from intensity transition value
		pWord = pKey->GetFirstWord();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the value for the intensity
		nTransitionFrom = 0;
		pWord->GetVal(nTransitionFrom);

		// make sure the intensity number is valid
		if ((nTransitionFrom <= 0) || (nTransitionFrom > m_nNumIntensities))
		{
			// this is an invalid transition go on to the next one
			// TO DO!!! Add OutputDebug information here and other places in this function!
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which is the intensity transition to value
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the value for the intensity
		nTransitionTo = 0;
		pWord->GetVal(nTransitionTo);

		// make sure the intensity number is valid
		if ((nTransitionTo <= 0) || (nTransitionTo > m_nNumIntensities))
		{
			// this is an invalid transition go on to the next one
			// TO DO!!! Add OutputDebug information here and other places in this function!
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which is the when to enact the transition value
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// figure out which enact value user specified
		m_nEnactTime = StringToEnactType(pWord->GetVal());

		// make sure we got a valid enact time
		if (m_nEnactTime == LTDMEnactInvalid)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which defines if this is an automatic or manual transition
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// figure out which enact value user specified
		if (stricmp(pWord->GetVal(), "MANUAL") == 0) m_bManual = true;
		else
		{
			if (stricmp(pWord->GetVal(), "AUTOMATIC") == 0) m_bManual = false;
			else
			{
				pKey = pKey->NextWithSameName();
				continue;
			}
		}

		// get the next word which is the when to enact the transition value
		pWord = pWord->Next();

		// check if there is a segment name for this transition
		if (pWord != LTNULL)
		{
			// load in the segment
			LoadSegment(pWord->GetVal());

			// find the loaded segment in the master segment list
			CSegment* pMasterSegment = m_lstSegments.Find(pWord->GetVal());
			transition = GetTransition(nTransitionFrom, nTransitionTo);
			if (pMasterSegment != LTNULL)
			{
				transition->SetSoundInstances(pMasterSegment->GetSoundInstanceArray(),
					pMasterSegment->GetNumInstances());
			}
			else
			{
				transition->SetSoundInstances(LTNULL, 0);
			}
		}

		transition = GetTransition(nTransitionFrom, nTransitionTo);
		// set the other values in our transition
		transition->SetEnactTime(m_nEnactTime);
		transition->SetManual(m_bManual);

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

bool CLTDirectMusicMgr::CSegment::SetSegmentName(const char* sSegmentName)
{
	// if there was an old long segment name delete it
	if (m_sSegmentNameLong != LTNULL)
	{
		delete[] m_sSegmentNameLong;
		m_sSegmentNameLong = LTNULL;
	}

	// if new name is null
	if (sSegmentName == LTNULL)
	{
		// just set the segment name to nothing
		m_sSegmentName[0] = '\0';
	}

	// if new name is valid then proceed
	else
	{
		// find out the length of the string
		int nLen = (int)strlen(sSegmentName);

		// check if we need to use the long storage method
		if (nLen > 63)
		{
			// allocate the new string
			LT_MEM_TRACK_ALLOC(m_sSegmentNameLong = new char[nLen + 1], LT_MEM_TYPE_MUSIC);

			// if allocation failed then exit
			if (m_sSegmentNameLong == LTNULL) return false;

			// copy over the string contents
			strcpy(m_sSegmentNameLong, sSegmentName);

			// set the short name to nothing
			m_sSegmentName[0] = '\0';
		}

		// it is a short name so just copy it over
		else
		{
			LTStrCpy(m_sSegmentName, sSegmentName, sizeof(m_sSegmentName));
		}
	}

	// exit successfully
	return true;
}

const char* CLTDirectMusicMgr::CSegment::GetSegmentName()
{
	if (m_sSegmentNameLong == LTNULL) return m_sSegmentName;
	else return m_sSegmentNameLong;
}

CLTDirectMusicMgr::CSegment* CLTDirectMusicMgr::CSegmentList::Find(const char* sName)
{
	// if the name passed in is LTNULL just return LTNULL
	if (sName == LTNULL) return LTNULL;

	// get the first item in the segment list
	CSegment* pSegment = GetFirst();

	// loop through all segments
	while (pSegment != LTNULL)
	{
		// make sure the segment name is not null
		if (pSegment->GetSegmentName() != LTNULL)
		{
			// compare and see if this is the one
			if (stricmp(pSegment->GetSegmentName(), sName) == 0)
			{
				// we have found it exit
				return pSegment;
			}
		}

		// get the next segment
		pSegment = pSegment->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}

CLTDirectMusicMgr::CSegmentState* CLTDirectMusicMgr::CSegmentStateList::Find(const char* sName)
{
	// if the name passed in is LTNULL just return LTNULL
	if (sName == LTNULL) return LTNULL;

	// get the first item in the segment list
	CSegmentState* pSegmentState = GetFirst();

	// loop through all segment states
	while (pSegmentState != LTNULL)
	{
		// get the segmenet
		CSegment* pSegment = pSegmentState->GetSegment();

		// make sure segment is not LTNULL
		if (pSegment != LTNULL)
		{
			// make sure the segment name is not null
			if (pSegment->GetSegmentName() != LTNULL)
			{
				// compare and see if this is the one
				if (stricmp(pSegment->GetSegmentName(), sName) == 0)
				{
					// we have found it exit
					return pSegmentState;
				}
			}
		}

		// get the next segment
		pSegmentState = pSegmentState->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}

CLTDirectMusicMgr::CSegmentState* CLTDirectMusicMgr::CSegmentStateList::Find(const void* pDMSegmentState)
{
	// get the first item in the segment list
	CSegmentState* pSegmentState = GetFirst();

	// loop through all segments
	while (pSegmentState != LTNULL)
	{
		// compare and see if this is the one
		if (pDMSegmentState == pSegmentState->GetSoundInstanceArray())
		{
			// we have found it exit
			return pSegmentState;
		}

		// get the next segment
		pSegmentState = pSegmentState->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}

bool CLTDirectMusicMgr::CStyle::SetStyleName(const char* sStyleName)
{
	// if there was an old long segment name delete it
	if (m_sStyleNameLong != LTNULL)
	{
		delete[] m_sStyleNameLong;
		m_sStyleNameLong = LTNULL;
	}

	// if new name is null
	if (sStyleName == LTNULL)
	{
		// just set the style name to nothing
		m_sStyleName[0] = '\0';
	}

	// if new name is valid then proceed
	else
	{
		// find out the length of the string
		int nLen = (int)strlen(sStyleName);

		// check if we need to use the long storage method
		if (nLen > 63)
		{
			// allocate the new string
			LT_MEM_TRACK_ALLOC(m_sStyleNameLong = new char[nLen + 1], LT_MEM_TYPE_MUSIC);

			// if allocation failed then exit
			if (m_sStyleNameLong == LTNULL) return false;

			// copy over the string contents
			strcpy(m_sStyleNameLong, sStyleName);

			// set the short name to nothing
			m_sStyleName[0] = '\0';
		}

		// it is a short name so just copy it over
		else
		{
			strcpy(m_sStyleName, sStyleName);
		}
	}

	// exit successfully
	return true;
}

const char* CLTDirectMusicMgr::CStyle::GetStyleName()
{
	if (m_sStyleNameLong == LTNULL) return m_sStyleName;
	else return m_sStyleNameLong;
}

CLTDirectMusicMgr::CStyle* CLTDirectMusicMgr::CStyleList::Find(const char* sName)
{
	// if the name passed in is LTNULL just return LTNULL
	if (sName == LTNULL) return LTNULL;

	// get the first item in the segment list
	CStyle* pStyle = GetFirst();

	// loop through all segments
	while (pStyle != LTNULL)
	{
		// make sure the segment name is not null
		if (pStyle->GetStyleName() != LTNULL)
		{
			// compare and see if this is the one
			if (stricmp(pStyle->GetStyleName(), sName) == 0)
			{
				// we have found it exit
				return pStyle;
			}
		}

		// get the next segment
		pStyle = pStyle->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}

CLTDirectMusicMgr::CIntensity::~CIntensity()
{
	// free all of the segment classes that were specified
	// we don't have to release the directmusic segment here
	// because that is done in the global segment list in TermLevel
	CSegment* pSegment;
	while ((pSegment = m_lstSegments.GetFirst()) != LTNULL)
	{
		// make sure the segment name is de-allocated
		pSegment->SetSegmentName(LTNULL);

		// remove our Segment object from the Segment list
		m_lstSegments.Delete(pSegment);

		// delete our Segment object
		delete pSegment;
	}
}

void CLTDirectMusicMgr::ClearCommands()
{
	CCommandItem* pCommand;

	// loop through all the commands
	while ((pCommand = m_lstCommands.GetFirst()) != LTNULL)
	{
		// remove our Command object from the Command list
		m_lstCommands.Delete(pCommand);

		// delete our Command object
		delete pCommand;
	}

	// loop through all the commands in 2nd command queue
	while ((pCommand = m_lstCommands2.GetFirst()) != LTNULL)
	{
		// remove our Command object from the Command list
		m_lstCommands2.Delete(pCommand);

		// delete our Command object
		delete pCommand;
	}
}

CLTDirectMusicMgr::CTransition* CLTDirectMusicMgr::GetTransition(int nFrom, int nTo)
{
	int Index = nFrom + 1 + (nTo*(m_nNumIntensities + 1));

	if ((nFrom > m_nNumTransitions) || (nFrom <= 0) ||
		(nTo > m_nNumTransitions) || (nTo <= 0) ||
		(Index >= m_nNumTransitions) || (Index < 0))
	{
		return &m_aryTransitions[0];
	}
	else
	{

		return &m_aryTransitions[Index];
	}
}

void CLTDirectMusicMgr::CSegmentList::CleanupSegments(CLTDirectMusicMgr* pLTDMMgr, bool bOnlyIfNotPlaying)
{
	// get the first item in the secondary segment playing list
	CSegment* pSegment = GetFirst();
	CSegment* pNextSegment;

	// loop through all the seondary segments that are playing
	while (pSegment != LTNULL)
	{
		// get the next segment
		pNextSegment = pSegment->Next();

		// clean up the segment
		CleanupSegment(pLTDMMgr, pSegment, LTDMEnactDefault, bOnlyIfNotPlaying);

		// set segment to next segment
		pSegment = pNextSegment;
	}
}

void CLTDirectMusicMgr::CSegmentList::CleanupSegment(CLTDirectMusicMgr* pLTDMMgr, CSegment* pSegment, LTDMEnactTypes nStart, bool bOnlyIfNotPlaying)
{
	ILTSoundSys* sys = NULL;
	CSoundInstance* instance, **array;
	unsigned int i, num_instances;
	LHSAMPLE sample;
	int flags;

	// make sure parameters passed in are OK
	if ((pLTDMMgr != LTNULL) && (pSegment != LTNULL))
	{
		num_instances = pSegment->GetNumInstances();
		array = pSegment->GetSoundInstanceArray();
		for (i = 0; i  < num_instances; i++)
		{
			instance = array[i];
			// make sure buffer is not null
			if (instance != LTNULL)
			{
				// check if it is playing
				flags = instance->GetSoundInstanceFlags();

				bool bPlaying = true;

				// check if this segment is not playing or if we don't have to check
				if ((!bPlaying) || (bOnlyIfNotPlaying == false))
				{
					sys = GetSoundSys();
					sample = instance->GetSample();
					sys->ReleaseSampleHandle(sample);
					instance->SetSampleHandle(LTNULL);
					instance->Term();
					delete instance;
				}
			}
		}

		if (num_instances > 0)
		{
			delete [] array;
		}

		pSegment->SetSoundInstances(LTNULL, 0);

		// make sure the segment name is de-allocated
		pSegment->SetSegmentName(LTNULL);

		// remove our Segment object from the Segment list
		Delete(pSegment);

		// delete our Segment object
		delete pSegment;

	}
}

void CLTDirectMusicMgr::CSegmentStateList::CreateSegmentState(CSoundInstance** pSoundInstances, CSegment* pSegment, unsigned int num_instances)
{
	CSegmentState* pNewSegmentState;

	// make new direct music segment state is not NULL
	if (pSoundInstances != LTNULL)
	{
		// create a new secondary segment state item
		LT_MEM_TRACK_ALLOC(pNewSegmentState = new CSegmentState, LT_MEM_TYPE_MUSIC);

		// make sure allocation worked
		if (pNewSegmentState != LTNULL)
		{
			// set the directmusic segment inside the new segment state
			pNewSegmentState->SetSoundInstances(pSoundInstances, num_instances);

			// set the originating segment inside the new segment state
			pNewSegmentState->SetSegment(pSegment);

			// add the new segment to the list of secondary segments
			Insert(pNewSegmentState);
		}
	}
}

void CLTDirectMusicMgr::CSegmentStateList::CleanupSegmentStates(CLTDirectMusicMgr* pLTDMMgr, bool bOnlyIfNotPlaying)
{
	// get the first item in the segment playing list
	CSegmentState* pSegmentState = GetFirst();
	CSegmentState* pNextSegmentState;

	// loop through all the seondary segments that are playing
	while (pSegmentState != LTNULL)
	{
		// get the next segment
		pNextSegmentState = pSegmentState->Next();

		// clean up the segment
		CleanupSegmentState(pLTDMMgr, pSegmentState, LTDMEnactDefault, bOnlyIfNotPlaying);

		// set segment to next segment
		pSegmentState = pNextSegmentState;
	}
}

void CLTDirectMusicMgr::CSegmentStateList::CleanupSegmentState(CLTDirectMusicMgr* pLTDMMgr, CSegmentState* pSegmentState, LTDMEnactTypes nStart, bool bOnlyIfNotPlaying)
{
	int flags;
	CSoundInstance* instance, **array;
	unsigned int num_instances, i;
	ILTSoundSys* sys = NULL;
	LHSAMPLE sample;

	// make sure parameters passed in are OK
	if ((pLTDMMgr != LTNULL) && (pSegmentState != LTNULL))
	{
		num_instances = pSegmentState->GetNumInstances();
		array = pSegmentState->GetSoundInstanceArray();
		for (i = 0; i  < num_instances; i++)
		{

			instance = array[i];

			if (instance != LTNULL)
			{
				// check if it is playing
				flags = instance->GetSoundInstanceFlags();

				bool bPlaying = true;

				// check if this segment is not playing or if we don't have to check
				if ((!bPlaying) || (bOnlyIfNotPlaying == false))
				{
					// stop if it is playing
	//				This check does not work for some reason so just stop all the time.
	//				if (bPlaying)
					{
						// if the enact type is invalid then do not call stop on this segment state
						if (nStart != LTDMEnactInvalid)
						{
#ifdef ACQUIRE_SAMPLE_ON_PLAY
							instance->Stop();
#else
							flags = instance->GetSoundInstanceFlags();
							flags |= SOUNDINSTANCEFLAG_DONE;
							flags &= ~SOUNDINSTANCEFLAG_PLAYING;
							instance->SetSoundInstanceFlags(flags);
							sys = GetSoundSys();
							sample = instance->GetSample();
							sys->StopSample(sample);
#endif
						}
					}


				}
			}

		}

		pSegmentState->SetSoundInstances(LTNULL, 0);

		// remove our Segment object from the Segment list
		Delete(pSegmentState);

		// delete our Segment object
		delete pSegmentState;


	}
}

uint32 CLTDirectMusicMgr::EnactTypeToFlags(LTDMEnactTypes nEnactVal)
{
	uint32 nFlags = 0; //RKNSTUB
	return nFlags;
}

bool CLTDirectMusicMgr::SetReverbParameters(void* pParams)
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::EnableReverb()
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::DisableReverb()
{
	return true; //RKNSTUB
}

bool CLTDirectMusicMgr::InitReverb(CControlFileMgr& controlFile)
{
	return true;
}

bool CLTDirectMusicMgr::TermReverb()
{
	// check if reverb was on
	if (m_bUseReverb)
	{
		// Disable it if it was on
		return DisableReverb();
	}
	// we don't need to do anything if it wasn't on
	else return true;
}

bool CLTDirectMusicMgr::PlayInstance(CSoundInstance* instance, int reset_prev)
{
#ifdef ACQUIRE_SAMPLE_ON_PLAY
	instance->AcquireSample();
#endif
	if (instance->GetSample())
	{
		instance->SetTimer(instance->GetDuration());
		instance->UpdateOutput(0);
		instance->SetVolumeNoMultiplier(m_nVolume);
		if (reset_prev)
		{
			m_nPrevIntensity = 0;
		}
		return true;
	}

	return false;
}

void CLTDirectMusicMgr::EnactTypeToString(LTDMEnactTypes nType, char* sName)
{
	switch (nType)
	{
	case LTDMEnactInvalid:
	{
		strcpy(sName, "Invalid");
		break;
	}
	case LTDMEnactDefault:
	{
		strcpy(sName, "Default");
		break;
	}
	case LTDMEnactImmediately:
	{
		strcpy(sName, "Immediate");
		break;
	}
	case LTDMEnactNextBeat:
	{
		strcpy(sName, "Beat");
		break;
	}
	case LTDMEnactNextMeasure:
	{
		strcpy(sName, "Measure");
		break;
	}
	case LTDMEnactNextGrid:
	{
		strcpy(sName, "Grid");
		break;
	}
	case LTDMEnactNextSegment:
	{
		strcpy(sName, "Segment");
		break;
	}
	case LTDMEnactNextMarker:
	{
		strcpy(sName, "Marker");
		break;
	}
	default:
	{
		strcpy(sName, "");
		break;
	}
	}
}

LTDMEnactTypes CLTDirectMusicMgr::StringToEnactType(const char* sName)
{
	if (sName == LTNULL) return LTDMEnactInvalid;
	if (stricmp(sName, "Invalid") == 0) return LTDMEnactInvalid;
	if (stricmp(sName, "Default") == 0) return LTDMEnactDefault;
	if (stricmp(sName, "Immediatly") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "Immediately") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "Immediate") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "NextBeat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "NextMeasure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "NextGrid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "NextSegment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "NextMarker") == 0) return LTDMEnactNextMarker;
	if (stricmp(sName, "Beat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "Measure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "Grid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "Segment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "Marker") == 0) return LTDMEnactNextMarker;
	return LTDMEnactInvalid;
}

int CLTDirectMusicMgr::GetCurIntensity()
{
	return m_nCurIntensity;
};

int CLTDirectMusicMgr::GetNumIntensities()
{
	return m_nNumIntensities;
};

int CLTDirectMusicMgr::GetInitialIntensity()
{
	return m_nInitialIntensity;
};

int CLTDirectMusicMgr::GetInitialVolume()
{
	return m_nInitialVolume;
};

int CLTDirectMusicMgr::GetVolumeOffset()
{
	return m_nVolumeOffset;
};

void CLTDirectMusicMgr::Update(uint32 dt)
{
	uint32 timer;
	uint32 half;

	CSegmentState* pSegmentState = m_lstPrimairySegmentsPlaying.GetFirst();
	CSegmentState* pNextSegmentState;
	CSegment* pSegment;
	unsigned int instance_index, i;
	CSoundInstance* instance, **array;

	while (pSegmentState != LTNULL)
	{
		pNextSegmentState = pSegmentState->Next();
		instance_index = pSegmentState->GetCurrentInstance();
		pSegment = pSegmentState->GetSegment();
		array = pSegmentState->GetSoundInstanceArray();

		for (i = 0; i < instance_index; i++)
		{
			instance = array[i];
			timer = instance->GetTimer();
			instance->UpdateTimer(dt);
			half = instance->GetDuration() / 2;

			if (timer > half && instance->GetTimer() <= half)
			{
				if (instance_index < pSegmentState->GetNumInstances())
				{
					pSegmentState->AdvanceInstance();
					PlayInstance(array[instance_index], 0);
				}
				else
				{
					HandleSegmentNotification_SegAlmostEnd(array);
				}
			}

			if (instance->GetTimer() <= 0 && i == pSegmentState->GetCurrentInstance() - 1)
			{
				HandleSegmentNotification_SegEnd(array);
			}
		}

		pSegmentState = pNextSegmentState;
	}
}