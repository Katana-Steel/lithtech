#include "iltsound.h"

typedef sint16	S16;
typedef uint16	U16;
typedef sint32	S32;
typedef uint32	U32;
typedef lpvoid	PTR;

//! CSDLSoundSys

class CSDLSoundSys : public ILTSoundSys
{
    char error[1024];
public:
	CSDLSoundSys( ) {
        memset(error, 0, sizeof(error));
        strncpy(error, "no error", sizeof(error));
    }
	virtual ~CSDLSoundSys( ) {}

public:
	virtual bool		Init( );
	virtual void		Term( );

public:
	virtual void*		GetDDInterface( uint uiDDInterfaceId );

public:
	// system wide functions
	virtual void		Lock( void );
	virtual void		Unlock( void );
	virtual S32			Startup( void );
	virtual void		Shutdown( void );
	virtual U32			MsCount( void );
	virtual S32			SetPreference( U32 uiNumber, S32 siValue );
	virtual S32			GetPreference( U32 uiNumber );
	virtual void		MemFreeLock( void* ptr );
	virtual void*		MemAllocLock( U32 uiSize );
	virtual char*	    LastError( void );

	// digital sound driver functions
	virtual S32			WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut, S32 siDeviceId, void* pWaveFormat );
	virtual void		WaveOutClose( LHDIGDRIVER hDriver );
	virtual void		SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume );
	virtual S32			GetDigitalMasterVolume( LHDIGDRIVER hDig );
	virtual S32			DigitalHandleRelease( LHDIGDRIVER hDriver );
	virtual S32			DigitalHandleReacquire( LHDIGDRIVER hDriver );

	// 3d sound provider functions
    virtual void        Set3DProviderMinBuffers(uint32 uiMinBuffers){};
	virtual sint32      Open3DProvider( LHPROVIDER hLib );
	virtual void		Close3DProvider( LHPROVIDER hLib );
	virtual void		Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal );
	virtual void		Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal );
	virtual S32			Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, char** psName);
	virtual S32			Get3DRoomType( LHPROVIDER hLib );
	virtual void		Set3DRoomType( LHPROVIDER hLib, S32 siRoomType );

	// 3d listener functions
	virtual LH3DPOBJECT	Open3DListener( LHPROVIDER hLib );
	virtual void		Close3DListener( LH3DPOBJECT hListener );
	virtual void		SetListenerDoppler( LH3DPOBJECT hListener, float fDoppler ) {};
	virtual void		CommitDeferred() {};

	// 3d sound object functions
	virtual void		Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ);
	virtual void		Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms );
	virtual void		Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up );
	virtual void		Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, uintptr_t siValue );
	virtual void		Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ);
	virtual void		Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms );
	virtual void		Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up );
	virtual S32			Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex);

	// 3d sound sample functions
	virtual LH3DSAMPLE	Allocate3DSampleHandle( LHPROVIDER hLib );
	virtual void		Release3DSampleHandle( LH3DSAMPLE hS );
	virtual void		Stop3DSample( LH3DSAMPLE hS );
	virtual void		Start3DSample( LH3DSAMPLE hS );
	virtual void		Resume3DSample( LH3DSAMPLE hS );
	virtual void		End3DSample( LH3DSAMPLE hS );
	virtual sint32		Init3DSampleFromAddress( LH3DSAMPLE hS, void* pStart, uint32 uiLen, void* pWaveFormat, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData  ) {return 0;};
	virtual sint32		Init3DSampleFromFile( LH3DSAMPLE hS, void* pFile_image, sint32 siBlock, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData )  {return 0;};
	virtual sint32		Get3DSampleVolume( LH3DSAMPLE hS ) {return 0;};
	virtual void		Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume );
	virtual U32			Get3DSampleStatus( LH3DSAMPLE hS );
	virtual void		Set3DSampleMsPosition( LHSAMPLE hS, sint32 siMilliseconds ) {};
	virtual S32			Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo );
	virtual void		Set3DSamplePlaybackRate( LH3DSAMPLE hS, S32 siPlayback_rate );
	virtual void		Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist );
	virtual void		Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal );
	virtual void		Set3DSampleLoopBlock( LH3DSAMPLE hS, sint32 siLoop_start_offset, sint32 siLoop_end_offset, bool bEnable ) {};
	virtual void		Set3DSampleLoop( LH3DSAMPLE hS, bool bLoop ) {};
	virtual void		Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction );
	virtual float		Get3DSampleObstruction( LH3DSAMPLE hS );
	virtual void		Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion );
	virtual float		Get3DSampleOcclusion( LH3DSAMPLE hS );

	// 2d sound sample functions
	virtual LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig );
	virtual void		ReleaseSampleHandle( LHSAMPLE hS );
	virtual void		InitSample( LHSAMPLE hS );
	virtual sint32		InitSampleFromAddress( LHSAMPLE hS, void* pStart, uint32 uiLen, void* pWaveFormat, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData ) {return 0;};
	virtual sint32		InitSampleFromFile( LHSAMPLE hS, void* pFile_image, sint32 siBlock, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData ) {return 0;};
	virtual void		StopSample( LHSAMPLE hS );
	virtual void		StartSample( LHSAMPLE hS );
	virtual void		ResumeSample( LHSAMPLE hS );
	virtual void		EndSample( LHSAMPLE hS );
	virtual void		SetSampleVolume( LHSAMPLE hS, S32 siVolume );
	virtual void		SetSamplePan( LHSAMPLE hS, S32 siPan );
	virtual sint32		GetSampleVolume( LHSAMPLE hS );
	virtual sint32		GetSamplePan( LHSAMPLE hS );
	virtual void		SetSampleUserData( LHSAMPLE hS, U32 uiIndex, uintptr_t siValue );
	virtual void		GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB );
	virtual void		SetSampleType( LHSAMPLE hS, S32 siFormat, U32 uiFlags );
	virtual void		SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time );
	virtual void		SetSampleAddress( LHSAMPLE hS, void* pStart, U32 uiLen );
	virtual sint32		SetSampleFile( LHSAMPLE hS, void* pFile_image, S32 siBlock );
	virtual void		SetSamplePlaybackRate( LHSAMPLE hS, S32 siPlayback_rate );
	virtual void		SetSampleLoopBlock( LHSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset, bool bEnable);
	virtual void		SetSampleLoop( LHSAMPLE hS, bool bLoop ) {};
	virtual void		SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds );
	virtual sint32		GetSampleUserData( LHSAMPLE hS, U32 uiIndex );
	virtual uint32		GetSampleStatus( LHSAMPLE hS ) {return 0;};

	// old 2d sound stream functions
	virtual LHSTREAM	OpenStream( char* sFilename, uint32 nOffset, LHDIGDRIVER hDig, char* sStream, S32 siStream_mem );
	virtual void		SetStreamLoop( LHSTREAM hStream, bool bLoop ){};
	virtual void		SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate );
	virtual void		SetStreamMsPosition( LHSTREAM hS, S32 siMilliseconds );
	virtual void		SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue);
	virtual S32			GetStreamUserData( LHSTREAM hS, U32 uiIndex);

	// new 2d sound stream functions
	virtual void		CloseStream( LHSTREAM hStream );
	virtual void		StartStream( LHSTREAM hStream );
	virtual void		PauseStream( LHSTREAM hStream, sint32 siOnOff );
	virtual void		ResetStream( LHSTREAM hStream );
	virtual void		SetStreamVolume( LHSTREAM hStream, sint32 siVolume );
	virtual void		SetStreamPan( LHSTREAM hStream, sint32 siPan );
	virtual sint32		GetStreamVolume( LHSTREAM hStream );
	virtual sint32		GetStreamPan( LHSTREAM hStream );
	virtual uint32		GetStreamStatus( LHSTREAM hStream );
	virtual sint32		GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam );
	virtual void		ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue = true);
	virtual bool		QueueStreamData( LHSTREAM, uint8* pucPCMSoundData, uint uiNumBytes );
	virtual	sint32		GetStreamDataQueueSize( LHSTREAM hStream );

	// wave file decompression functons
	virtual S32			DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, U32* puiOutSize );
	virtual S32			DecompressASI( void* pInData, U32 uiInSize, char* sFilename_ext, void** ppWav, U32* puiWavSize, LTLENGTHYCB fnCallback );

	// Gets the ticks spent in sound thread.
	virtual uint32		GetThreadedSoundTicks( ) {return 0;};

	virtual bool		HasOnBoardMemory( ){return false;};
public:
	static CSDLSoundSys m_SDLSoundSys;
	static const char*	m_pcSDLSoundSysDesc;
};

bool CSDLSoundSys::Init( )
{
	return true;
}

void CSDLSoundSys::Term( )
{
}

void* CSDLSoundSys::GetDDInterface( uint uiDDInterfaceId )
{
	return NULL;
}

// system wide functions
void CSDLSoundSys::Lock( void ) 
{ 
}

void CSDLSoundSys::Unlock( void ) 
{ 
}

S32	CSDLSoundSys::Startup( void ) 
{ 
	return 0; 
}

void CSDLSoundSys::Shutdown( void ) 
{ 
}

U32	CSDLSoundSys::MsCount( void ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::SetPreference( U32 uiNumber, S32 siValue ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::GetPreference( U32 uiNumber ) 
{ 
	return 0; 
}

void CSDLSoundSys::MemFreeLock( void* ptr ) 
{ 
	LTMemFree( ptr );
}

void* CSDLSoundSys::MemAllocLock( U32 uiSize ) 
{ 
	void* p;
	LT_MEM_TRACK_ALLOC(p = LTMemAlloc( uiSize ),LT_MEM_TYPE_SOUND);
	return p;
}

char* CSDLSoundSys::LastError( void ) 
{ 
	return error;
}

// digital sound driver functions
S32	CSDLSoundSys::WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut, S32 siDeviceId, void* pWaveFormat ) 
{ 
	return 0; 
}

void CSDLSoundSys::WaveOutClose( LHDIGDRIVER hDriver ) 
{ 
}

void CSDLSoundSys::SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume ) 
{ 
}

S32	CSDLSoundSys::GetDigitalMasterVolume( LHDIGDRIVER hDig ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::DigitalHandleRelease( LHDIGDRIVER hDriver ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::DigitalHandleReacquire( LHDIGDRIVER hDriver ) 
{ 
	return 0; 
}

// 3d sound provider functions
sint32 CSDLSoundSys::Open3DProvider( LHPROVIDER hLib ) 
{ 
	return 0; 
}

void CSDLSoundSys::Close3DProvider( LHPROVIDER hLib ) 
{ 
}

void CSDLSoundSys::Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal ) 
{ 
}

void CSDLSoundSys::Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal ) 
{ 
}

S32	CSDLSoundSys::Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, char** psName) 
{ 
	return 0; 
}

S32	CSDLSoundSys::Get3DRoomType( LHPROVIDER hLib ) 
{ 
	return 0; 
}

void CSDLSoundSys::Set3DRoomType( LHPROVIDER hLib, S32 siRoomType ) 
{ 
}

// 3d listener functions
LH3DPOBJECT	CSDLSoundSys::Open3DListener( LHPROVIDER hLib ) 
{ 
	return NULL;
}

void CSDLSoundSys::Close3DListener( LH3DPOBJECT hListener ) 
{ 
}

// 3d sound object functions
void CSDLSoundSys::Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ) 
{ 
}

void CSDLSoundSys::Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms ) 
{ 
}

void CSDLSoundSys::Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up ) 
{ 
}

void CSDLSoundSys::Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, uintptr_t siValue ) 
{ 
}

void CSDLSoundSys::Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ) 
{ 
}

void CSDLSoundSys::Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms ) 
{ 
}

void CSDLSoundSys::Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up ) 
{ 
}

S32	CSDLSoundSys::Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex) 
{ 
	return 0; 
}

// 3d sound sample functions
LH3DSAMPLE CSDLSoundSys::Allocate3DSampleHandle( LHPROVIDER hLib ) 
{ 
	return NULL;
}

void CSDLSoundSys::Release3DSampleHandle( LH3DSAMPLE hS ) 
{ 
}

void CSDLSoundSys::Stop3DSample( LH3DSAMPLE hS ) 
{ 
}

void CSDLSoundSys::Start3DSample( LH3DSAMPLE hS ) 
{ 
}

void CSDLSoundSys::Resume3DSample( LH3DSAMPLE hS ) 
{ 
}

void CSDLSoundSys::End3DSample( LH3DSAMPLE hS ) 
{ 
}

void CSDLSoundSys::Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume ) 
{ 
}

U32	CSDLSoundSys::Get3DSampleStatus( LH3DSAMPLE hS ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo ) 
{ 
	return 0; 
}

void CSDLSoundSys::Set3DSamplePlaybackRate( LH3DSAMPLE hS, S32 siPlayback_rate ) 
{ 
}

void CSDLSoundSys::Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist ) 
{ 
}

void CSDLSoundSys::Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal ) 
{ 
}

void CSDLSoundSys::Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction ) 
{ 
}

float CSDLSoundSys::Get3DSampleObstruction( LH3DSAMPLE hS ) 
{ 
	return 0; 
}

void CSDLSoundSys::Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion ) 
{ 
}

float CSDLSoundSys::Get3DSampleOcclusion( LH3DSAMPLE hS ) 
{ 
	return 0; 
}

// 2d sound sample functions
LHSAMPLE CSDLSoundSys::AllocateSampleHandle( LHDIGDRIVER hDig ) 
{ 
	return NULL;
}

void CSDLSoundSys::ReleaseSampleHandle( LHSAMPLE hS ) 
{ 
}

void CSDLSoundSys::InitSample( LHSAMPLE hS ) 
{ 
}

void CSDLSoundSys::StopSample( LHSAMPLE hS ) 
{ 
}

void CSDLSoundSys::StartSample( LHSAMPLE hS )
{
}

void CSDLSoundSys::ResumeSample( LHSAMPLE hS ) 
{ 
}

void CSDLSoundSys::EndSample( LHSAMPLE hS )
{
}

void CSDLSoundSys::SetSampleVolume( LHSAMPLE hS, S32 siVolume ) 
{ 
}

void CSDLSoundSys::SetSamplePan( LHSAMPLE hS, S32 siPan ) 
{ 
}

S32	CSDLSoundSys::GetSampleVolume( LHSAMPLE hS ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::GetSamplePan( LHSAMPLE hS ) 
{ 
	return 0; 
}

void CSDLSoundSys::SetSampleUserData( LHSAMPLE hS, U32 uiIndex, uintptr_t siValue ) 
{ 
}

void CSDLSoundSys::GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB ) 
{ 
}

void CSDLSoundSys::SetSampleType( LHSAMPLE hS, S32 siFormat, U32 uiFlags ) 
{ 
}

void CSDLSoundSys::SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time ) 
{ 
}

void CSDLSoundSys::SetSampleAddress( LHSAMPLE hS, void* pStart, U32 uiLen ) 
{ 
}

S32	CSDLSoundSys::SetSampleFile( LHSAMPLE hS, void* pFile_image, S32 siBlock ) 
{ 
	return 0; 
}

void CSDLSoundSys::SetSamplePlaybackRate( LHSAMPLE hS, S32 siPlayback_rate ) 
{ 
}

void CSDLSoundSys::SetSampleLoopBlock( LHSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset, bool bEnable ) 
{ 
}

void CSDLSoundSys::SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds ) 
{ 
}

S32	CSDLSoundSys::GetSampleUserData( LHSAMPLE hS, U32 uiIndex ) 
{ 
	return 0; 
}

// old 2d sound stream functions
LHSTREAM CSDLSoundSys::OpenStream( char* sFilename, uint32 nOffset, LHDIGDRIVER hDig, char* sStream, S32 siStream_mem ) 
{ 
	return NULL;
}

void CSDLSoundSys::SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate ) 
{ 
}

void CSDLSoundSys::SetStreamMsPosition( LHSTREAM hS, S32 siMilliseconds ) 
{ 
}

void CSDLSoundSys::SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue) 
{ 
}

S32	CSDLSoundSys::GetStreamUserData( LHSTREAM hS, U32 uiIndex) 
{ 
	return 0; 
}

// new 2d sound stream functions
void CSDLSoundSys::CloseStream( LHSTREAM hStream ) 
{ 
}

void CSDLSoundSys::StartStream( LHSTREAM hStream ) 
{ 
}

void CSDLSoundSys::PauseStream( LHSTREAM hStream, S32 siOnOff ) 
{ 
}

void CSDLSoundSys::ResetStream( LHSTREAM hStream )
{
}

void CSDLSoundSys::SetStreamVolume( LHSTREAM hStream, S32 siVolume ) 
{ 
}

void CSDLSoundSys::SetStreamPan( LHSTREAM hStream, S32 siPan ) 
{ 
}

S32	CSDLSoundSys::GetStreamVolume( LHSTREAM hStream ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::GetStreamPan( LHSTREAM hStream ) 
{ 
	return 0; 
}

uint32 CSDLSoundSys::GetStreamStatus( LHSTREAM hStream )
{
	return 0;
}

sint32 CSDLSoundSys::GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam )
{
	return 0;
}

void CSDLSoundSys::ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue )
{
}

bool CSDLSoundSys::QueueStreamData( LHSTREAM, uint8* pucPCMSoundData, uint uiNumBytes )
{
	return false;
}

sint32 CSDLSoundSys::GetStreamDataQueueSize( LHSTREAM hStream )
{
	return 0;
}

// wave file decompression functons
S32	CSDLSoundSys::DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, U32* puiOutSize ) 
{ 
	return 0; 
}

S32	CSDLSoundSys::DecompressASI( void* pInData, U32 uiInSize, char* sFilename_ext, void** ppWav, U32* puiWavSize, LTLENGTHYCB fnCallback ) 
{ 
	return 0; 
}

CSDLSoundSys CSDLSoundSys::m_SDLSoundSys;
const char* CSDLSoundSys::m_pcSDLSoundSysDesc = "SDL2 Mixer";

const char* csdl_GetSoundDesc()
{
    return CSDLSoundSys::m_pcSDLSoundSysDesc;
}

ILTSoundSys* csdl_MakeSoundSys()
{
    return &CSDLSoundSys::m_SDLSoundSys;
}

