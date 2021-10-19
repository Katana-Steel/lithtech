#include "iltsound.h"
#include "SDL.h"
#include "wave.h"
#ifdef _LINUX
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif
#include <mpg123.h>
#include <vector>

typedef sint16	S16;
typedef uint16	U16;
typedef sint32	S32;
typedef uint32	U32;
typedef lpvoid	PTR;

#define CHUNK_ID_RIFF			0x46464952	// 'RIFF'
#define CHUNK_ID_WAVE			0x45564157	// 'WAVE'
#define CHUNK_ID_FACT			0x74636166	// 'fact'
#define CHUNK_ID_DATA			0x61746164	// 'data'
#define CHUNK_ID_WAVH			0x68766177	// 'wavh'
#define CHUNK_ID_FMT			0x20746d66	// 'fmt'
#define CHUNK_ID_GUID			0x64697567	// 'guid'

enum
{
	PROVIDER_OPENAL = 0,
	NUM_PROVIDERS
};

#define MAX_USER_DATA_INDEX 7
#define	MAX_WAVE_STREAMS		16

#if 0
#define PRINTSTUB printf("STUB: %s in %s:%i\n", __FUNCTION__, __FILE__, __LINE__);
#else
#define PRINTSTUB
#endif

//ADPCM decoding from https://github.com/dbry/adpcm-xq/
//see adpcm_license.txt for details.

/* step table */
static const uint16_t step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14,
    16, 17, 19, 21, 23, 25, 28, 31,
    34, 37, 41, 45, 50, 55, 60, 66,
    73, 80, 88, 97, 107, 118, 130, 143,
    157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658,
    724, 796, 876, 963, 1060, 1166, 1282, 1411,
    1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
    3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
    7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
    32767
};

/* step index tables */
static const int index_table[] = {
    /* adpcm data size is 4 */
    -1, -1, -1, -1, 2, 4, 6, 8
};

#define CLIP(data, min, max) \
if ((data) > (max)) data = max; \
else if ((data) < (min)) data = min;

int adpcm_decode_block (int16_t *outbuf, const uint8_t *inbuf, size_t inbufsize, int channels)
{
    int ch, samples = 1, chunks;
    int32_t pcmdata[2];
    int8_t index[2];

    if (inbufsize < (uint32_t) channels * 4)
        return 0;

    for (ch = 0; ch < channels; ch++) {
        *outbuf++ = pcmdata[ch] = (int16_t) (inbuf [0] | (inbuf [1] << 8));
        index[ch] = inbuf [2];

        if (index [ch] < 0 || index [ch] > 88 || inbuf [3])     // sanitize the input a little...
            return 0;

        inbufsize -= 4;
        inbuf += 4;
    }

    chunks = inbufsize / (channels * 4);
    samples += chunks * 8;

    while (chunks--) {
        int ch, i;

        for (ch = 0; ch < channels; ++ch) {

            for (i = 0; i < 4; ++i) {
                int step = step_table [index [ch]], delta = step >> 3;

                if (*inbuf & 1) delta += (step >> 2);
                if (*inbuf & 2) delta += (step >> 1);
                if (*inbuf & 4) delta += step;
                if (*inbuf & 8) delta = -delta;

                pcmdata[ch] += delta;
                index[ch] += index_table [*inbuf & 0x7];
                CLIP(index[ch], 0, 88);
                CLIP(pcmdata[ch], -32768, 32767);
                outbuf [i * 2 * channels] = pcmdata[ch];

                step = step_table [index [ch]], delta = step >> 3;

                if (*inbuf & 0x10) delta += (step >> 2);
                if (*inbuf & 0x20) delta += (step >> 1);
                if (*inbuf & 0x40) delta += step;
                if (*inbuf & 0x80) delta = -delta;

                pcmdata[ch] += delta;
                index[ch] += index_table [(*inbuf >> 4) & 0x7];
                CLIP(index[ch], 0, 88);
                CLIP(pcmdata[ch], -32768, 32767);
                outbuf [(i * 2 + 1) * channels] = pcmdata[ch];

                inbuf++;
            }

            outbuf++;
        }

        outbuf += channels * 7;
    }

    return samples;
}

static char* adpcm_decode_data(void* p, int num_channels,
	size_t num_samples, int block_size)
{
    int samples_per_block = (block_size - num_channels * 4) * (num_channels ^ 3) + 1;
    char* pcm_block = new char[num_samples * num_channels * 2];
    unsigned char* adpcm_block = new unsigned char[block_size];
    int full_size = 0;
    int ret_samples;
    char* pcm_ptr = pcm_block;

    if (!pcm_block || !adpcm_block)
    {
        return NULL;
    }

    while (num_samples)
    {
		int this_block_adpcm_samples = samples_per_block;
		int this_block_pcm_samples = samples_per_block;

		if (this_block_adpcm_samples > num_samples)
		{
			this_block_adpcm_samples = ((num_samples + 6) & ~7) + 1;
			block_size = (this_block_adpcm_samples - 1) / (num_channels ^ 3) + (num_channels * 4);
			this_block_pcm_samples = num_samples;
		}

		memcpy(adpcm_block, p, block_size);
		p = (char*)p + block_size;

		ret_samples = adpcm_decode_block ((int16_t*)pcm_ptr,
			(uint8_t*)adpcm_block, block_size, num_channels);
		if (ret_samples != this_block_adpcm_samples)
		{
			return NULL;
		}

		pcm_ptr += (this_block_pcm_samples * 2);
		num_samples -= this_block_pcm_samples;

    }

    delete [] adpcm_block;
    return (char*)pcm_block;
}

#define OUTBUFF 1024
static char* mpeg3_decode(char* p, unsigned int length,
	unsigned int* uncompressedSize, WAVEFORMATEX* wfmt)
{
	mpg123_handle* handle;

	size_t size;
	char* newbuf;
	std::vector<char> test;
	unsigned char out[OUTBUFF]; /* output buffer */
	size_t len;
	int ret;
	size_t in = 0, outc = 0;
	unsigned int i;
	handle = mpg123_new(NULL, &ret);
	long rate;
	int channels, enc;
	if(handle == NULL)
	{
		printf("Unable to create mpg123 handle: %s\n", mpg123_plain_strerror(ret));
		return NULL;
	}

	mpg123_param(handle, MPG123_FLAGS, MPG123_QUIET, 0.0);
	mpg123_open_feed(handle);

	if(handle == NULL)
	{
		return NULL;
	}

	/* Feed input chunk and get first chunk of decoded audio. */
	ret = mpg123_decode(handle,(unsigned char*)p,length,out,OUTBUFF,&size);
	if(ret == MPG123_NEW_FORMAT)
	{
		mpg123_getformat(handle, &rate, &channels, &enc);
		//printf( "New format: %li Hz, %i channels, encoding value %i\n", rate, channels, enc);
		wfmt->nChannels = channels;
		if (enc == MPG123_ENC_SIGNED_16)
		{
			wfmt->wBitsPerSample = 16;
		}
		else
		{
			//assume invalid for now
			wfmt->wBitsPerSample = 0;
		}
		wfmt->nSamplesPerSec = rate;
	}
	outc += size;

	while(ret != MPG123_ERR && ret != MPG123_NEED_MORE)
	{
		ret = mpg123_decode(handle,NULL,0,out,OUTBUFF,&size);
		for (i = 0; i < size; i++)
		{
			test.push_back(out[i]);
		}
		outc += size;
	}

	if(ret == MPG123_ERR)
	{
		printf("mpg123 error: %s", mpg123_strerror(handle));
		return NULL;
	}

	if (test.size() == 0)
	{
		return NULL;
	}

	newbuf = new char[test.size()];
	memcpy(newbuf, test.data(), test.size() * sizeof(char));

	*uncompressedSize = outc;
	mpg123_delete(handle);

	return newbuf;
}

class COpenALSoundSys;

//! CSample

class CSample
{
public:
	CSample( );
	virtual ~CSample( );
	void Reset( );
	bool Init( int& hResult, void* pDS, uint32 uiNumSamples,
		bool b3DBuffer, WAVEFORMATEX* pWaveFormat = NULL, LTSOUNDFILTERDATA* pFilterData = NULL );
	void Term( );
	void Restore( );
	bool Fill( );
	bool UsesLoopingBlock() { return m_bLoopBlock; }

//	===========================================================================
//	Incorporation of DSMStrm* required functionality
public:
	bool IsPlaying( );
	virtual void SetLooping( COpenALSoundSys* pSoundSys, bool bLoop );
	bool IsLooping( ) { return m_bLooping; }

	bool GetCurrentPosition( unsigned int* pdwPlayPos, unsigned int* pdwWritePos );
	virtual bool SetCurrentPosition( unsigned int dwStartOffset );
	bool Play( );
	bool Stop( bool bReset = true );
	bool Lock( unsigned int dwStartOffset, unsigned int dwLockAmount, void** ppChunk1, unsigned int* pdwChunkSize1,
		void** ppChunk2, unsigned int* pdwChunkSize2, unsigned int dwFlags );
	void Unlock( void* pChunk1, unsigned int dwChunkSize1, void* pChunk2, unsigned int dwChunkSize2 );
	void HandleLoop( COpenALSoundSys* pSoundSys );

public:
	unsigned int m_dwPlayFlags;
//	===========================================================================

public:
	WAVEFORMATEX			m_waveFormat;
	ALuint					source;
	ALuint					buffer;
	char*					m_pSoundData;
	uint32					m_uiSoundDataLen;
	S32						m_userData[ MAX_USER_DATA_INDEX + 1 ];
	bool					m_bAllocatedSoundData;
	S32						m_nLoopStart;
	S32						m_nLoopEnd;
	bool					m_bLoopBlock;
	uint32					m_nLastPlayPos;
	bool					m_bLooping;

	static 	LTLink			m_lstSampleLoopHead;
};

LTLink CSample::m_lstSampleLoopHead;

CSample::CSample( )
{
	memset( &m_userData, 0, sizeof( m_userData ));
	m_bLoopBlock = false;
	Reset( );
}

CSample::~CSample( )
{
	Term( );
}

void CSample::Reset( )
{
	m_pSoundData = NULL;
	m_uiSoundDataLen = 0;
	m_bAllocatedSoundData = false;
	m_dwPlayFlags = 0;
	m_nLastPlayPos = 0;
	m_bLooping = false;
	source = 0;
	buffer = 0;
}

void CSample::Term( )
{
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);

	if( m_bAllocatedSoundData && m_pSoundData != NULL )
		delete[] m_pSoundData;

	Reset( );
}

bool CSample::Init( int& hResult, void* pDS, uint32 uiNumSamples,
		bool b3DBuffer, WAVEFORMATEX* pWaveFormat, LTSOUNDFILTERDATA* pFilterData )
{
	bool bUseFilter;

	Term( );

	if( pWaveFormat == NULL )
	{
		m_waveFormat.nBlockAlign = ( m_waveFormat.nChannels * m_waveFormat.wBitsPerSample ) >> 3;
		m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
	}
	else
	{
		memcpy( &m_waveFormat, pWaveFormat, sizeof( WAVEFORMATEX ) );
	}

	if (source == 0)
	{
		alGenSources(1, &source);
	}

	if (buffer == 0)
	{
		alGenBuffers(1, &buffer);
	}

	return true;
}

bool CSample::IsPlaying( )
{
	int status;
	if( source == 0 )
		return false;

	alGetSourcei(source, AL_SOURCE_STATE, &status);
	if (status == AL_PLAYING)
	{
		return true;
	}
	return false;
}

void CSample::SetLooping( COpenALSoundSys* pSoundSys, bool bLoop )
{
	if( bLoop != m_bLooping )
	{
		if( IsPlaying( ))
		{
			Stop( false );
			Play( );
		}
	}

	m_bLooping = bLoop;
	alSourcei(source, AL_LOOPING, m_bLooping ? AL_TRUE : AL_FALSE);
}

bool CSample::GetCurrentPosition( unsigned int* pdwPlayPos, unsigned int* pdwWritePos )
{
	if( source == 0 )
		return false;

	PRINTSTUB;
	return true;
}

bool CSample::SetCurrentPosition( unsigned int dwStartOffset )
{
	if( source == 0 )
		return false;

	alSourcef(source, AL_BYTE_OFFSET, dwStartOffset);

	// Set the last play pos to zero, because doing this on a playing sound
	// seems to not really set it to exactly dwStartOffset anyway.
	m_nLastPlayPos = 0;

	return true;
}

bool CSample::Play( )
{
	if( source == 0 )
		return false;

	alSourcei(source, AL_BUFFER, buffer);
	alSource3f(source, AL_POSITION, 0, 0, 0);
	alSourcePlay(source);
	return true;
}

bool CSample::Stop( bool bReset )
{
	if( source == 0 )
		return false;

	alSourceStop(source);

	if( bReset )
	{
		alSourcef(source, AL_SEC_OFFSET, 0.0f);
	}

	return true;
}

void CSample::Restore( )
{
	if( source == 0 )
		return;

	Fill( );
}

bool CSample::Fill( )
{
	ALenum fmt;
	if( m_pSoundData == NULL)
	{
		printf("Error in Fill - data is null!\n");
		return false;
	}
	if( m_uiSoundDataLen == 0)
	{
		printf("Error in Fill - m_uiSoundDataLen is zero!\n");
		return false;
	}
	if(source == 0 )
	{
		printf("Error in Fill - source is zero!\n");
		return false;
	}

	// Make sure we're stopped.  Shouldn't get here, since buffer's are stopped
	// when the sound ends.
	if( IsPlaying( ))
	{
		ASSERT( !"CSample::Fill:  Buffer should have been stopped before reaching fill." );
	}

	switch (m_waveFormat.wBitsPerSample)
	{
	case 8:
		fmt = m_waveFormat.nChannels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		break;
	case 16:
		fmt = m_waveFormat.nChannels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		break;
	default:
		printf("Error in %s:%i: bps is invalid, got %i!\n", __FILE__, __LINE__, m_waveFormat.wBitsPerSample);
		return false;
	}

	alBufferData(buffer, fmt, m_pSoundData, m_uiSoundDataLen, m_waveFormat.nSamplesPerSec);

	return true;
}

//! I3DObject

class I3DObject
{
public:
	I3DObject( );
	virtual ~I3DObject( );
	void Reset( );
	bool Init( );
	void Term( );
	virtual void SetPosition( LTVector& pos );
	virtual void SetVelocity( LTVector& vel );
	virtual void SetOrientation( LTVector& up, LTVector& face );

public:
	LTVector	m_position;
	LTVector	m_velocity;
	LTVector	m_up;
	LTVector	m_face;
	S32			m_userData[ MAX_USER_DATA_INDEX + 1 ];
};

I3DObject::I3DObject( )
{
	Reset( );
	memset( &m_userData, 0, sizeof( m_userData ));
}

I3DObject::~I3DObject( )
{
	Term( );
}

void I3DObject::Reset( )
{
	m_position = LTVector( 0.0f, 0.0f, 0.0f );
	m_velocity = LTVector( 0.0f, 0.0f, 0.0f );
	m_up = LTVector( 0.0f, 0.0f, 0.0f );
	m_face = LTVector( 0.0f, 0.0f, 0.0f );
//	for( S32 i = 0; i < MAX_USER_DATA_INDEX + 1; m_userData[ i++ ] = 0 );
}

bool I3DObject::Init( )
{
	Term( );
	return true;
}

void I3DObject::Term( )
{
	Reset( );
}

void I3DObject::SetPosition( LTVector& pos )
{
	m_position = pos;
}

void I3DObject::SetVelocity( LTVector& vel )
{
	m_velocity = vel;
}

void I3DObject::SetOrientation( LTVector& up, LTVector& face )
{
	m_up = up;
	m_face = face;
}

//! C3DSample

class C3DSample : public I3DObject
{
public:
	C3DSample( );
	virtual ~C3DSample( );
	void Reset( );
	bool Init( int& hResult, void* pDS, uint32 uiNumSamples, WAVEFORMATEX* pWaveFormat, LTSOUNDFILTERDATA* pFilterData );

	void Term( );
	virtual void SetPosition( LTVector& pos );
	virtual void SetVelocity( LTVector& vel );
	virtual void SetOrientation( LTVector& up, LTVector& face );
	virtual void SetRadiusData( float& fInnerRadius, float& fOuterRadius );
	virtual void GetRadiusData( float* fInnerRadius, float* fInnerRadiusSquared );
	virtual void SetDSMinDist( float& fDSMinDist ) { m_DSMinDist = fDSMinDist; }
	virtual void GetDSMinDist( float* fDSMinDist ) { *fDSMinDist = m_DSMinDist; }
public:
	float					m_innerRadius;
	float					m_outerRadius;
	float					m_innerRadiusSquared;
	float					m_DSMinDist;
	U32						m_status;
	CSample					m_sample;
};

C3DSample::C3DSample( )
{
	Reset( );
}

C3DSample::~C3DSample( )
{
	Term( );
}

void C3DSample::Reset( )
{
	I3DObject::Reset( );
	m_sample.Reset( );
	m_status = LS_DONE;
}

bool C3DSample::Init( int& hResult, void* pDS, uint32 uiNumSamples, WAVEFORMATEX* pWaveFormat, LTSOUNDFILTERDATA* pFilterData )
{
	Term( );

	if( !I3DObject::Init( ) )
	{
		return false;
	}

	if( !m_sample.Init( hResult, pDS, uiNumSamples, true, pWaveFormat, pFilterData ) )
	{
		return false;
	}

	return true;
}

void C3DSample::Term( )
{
	m_sample.Term( );
	I3DObject::Term( );

	Reset( );
}

void C3DSample::SetPosition( LTVector& pos )
{
	LTVector vPosInner;
//	float fRatioInnerToDist, fDistSquared;

	I3DObject::SetPosition(	pos );

/*
	// we want the relative position of the object to the inner
	// radius of the object
	fDistSquared = pos.MagSqr();
	fRatioInnerToDist = sqrtf(m_innerRadiusSquared)/sqrtf(fDistSquared);

	// if it's inside the inner radius
	if ( fRatioInnerToDist > 1.0f )
	{
		float fRatioInv = 1.0f / fRatioInnerToDist;
		pos = pos * fRatioInv * (m_DSMinDist/m_innerRadius);
	}
	else
	{
		// otherwise build a vector that is at the appropriate distance

		// get a vector to the inner radius in the appropriate direction
		vPosInner = pos * fRatioInnerToDist;
		// get the relative position of the object outside the inner radius
		pos = pos - vPosInner;
	}
*/
	PRINTSTUB;
}

void C3DSample::SetVelocity( LTVector& vel )
{
	I3DObject::SetVelocity( vel );
	PRINTSTUB;
}

void C3DSample::SetOrientation( LTVector& up, LTVector& face )
{
	I3DObject::SetOrientation( up, face );
	PRINTSTUB;
}

void C3DSample::SetRadiusData( float& fInnerRadius, float& fOuterRadius )
{
	m_innerRadius = fInnerRadius;
	m_outerRadius = fOuterRadius;
	m_innerRadiusSquared = fInnerRadius * fInnerRadius;
}

void C3DSample::GetRadiusData( float* fInnerRadius, float* fInnerRadiusSquared )
{
	*fInnerRadius = m_innerRadius;
	*fInnerRadiusSquared = m_innerRadiusSquared;
}

#define STR_BUFFER_SIZE			8192

class WaveFile
{
public:
    WaveFile (void);
    ~WaveFile (void);
	void Clear(void);
    bool Open (char* pszFilename, uint32 nFilePos);
	void Close(void);
    bool Cue (void);
	bool IsMP3() { return m_bMP3Compressed; }
    unsigned int Read (unsigned char * pbDest, unsigned int cbSize);
	unsigned int ReadCompressed( unsigned char* pbDest, unsigned int cbSize, unsigned char* pCompressedBuffer, unsigned char* pbDecompressedBuffer );
    unsigned int GetAvgDataRate (void) { return (m_nAvgDataRate); }
    unsigned int GetDataSize (void) { return (m_nDataSize); }
    unsigned int GetNumBytesRead (void) { return (m_nBytesRead); }
	unsigned int GetNumBytesCopied( ) { return m_nBytesCopied; }
	unsigned int GetMaxBytesRead( ) { return m_nMaxBytesRead; }
	unsigned int GetMaxBytesCopied( ) { return m_nMaxBytesCopied; }
    unsigned int GetDuration (void) { return (m_nDuration); }
    unsigned char GetSilenceData (void);
	void SetBytesPerSample( unsigned int nBytesPerSample ) { m_nBytesPerSample = nBytesPerSample; }

	unsigned int SeekFromStart( uint32 nBytes );
	unsigned int SeekFromStartCompressed( uint32 nBytes );

	// Integration functions
	bool IsActive()	{ return (m_hStream) ? true : false;  }
	void SetStream(LHSTREAM hStream) { m_hStream = hStream; }
	LHSTREAM GetStream() { return m_hStream;  }

	// decompression related functions
	void SetSrcBufferSize( unsigned long ulSrcBufferSize ) { m_ulSrcBufferSize = ulSrcBufferSize; }

    WAVEFORMATEX m_wfmt;

protected:
	uint32 m_nFilePos;			// Offset from the beginning of the file where the wav is.
	unsigned int m_nDuration;           // duration of sound in msec
	unsigned int m_nBlockAlign;         // wave data block alignment spec
	unsigned int m_nAvgDataRate;        // average wave data rate
	unsigned int m_nDataSize;           // size of data chunk
	unsigned int m_nBytesRead;			// Number of uncompressed bytes read.
	unsigned int m_nMaxBytesRead;		// Maximum number of bytes read, regardless of looping back.
	unsigned int m_nBytesCopied;		// Number of uncompressed bytes copied.
	unsigned int m_nMaxBytesCopied;		// Maximum number of bytes copied, regardless of looping back.
	unsigned int m_nBytesPerSample;		// number of bytes in each sample


	// Integration data
	LHSTREAM m_hStream;

	// for files that need to be decompressed
	bool m_bMP3Compressed;
	unsigned long m_ulSrcBufferSize;
	unsigned int m_nRemainderBytes;
	unsigned int m_nRemainderOffset;
	unsigned char m_RemainderBuffer[STR_BUFFER_SIZE];
};

// WaveFile class implementation
//
////////////////////////////////////////////////////////////


// Constructor
WaveFile::WaveFile (void)
{
	Clear();
}

// Destructor
WaveFile::~WaveFile (void)
{
	Close();
}


void WaveFile::Clear()
{
    // Init data members
    m_nBlockAlign= 0;
    m_nAvgDataRate = 0;
    m_nDataSize = 0;
    m_nBytesRead = 0;
	m_nMaxBytesRead = 0;
    m_nBytesCopied = 0;
	m_nMaxBytesCopied = 0;
    m_nDuration = 0;
	m_nBytesPerSample = 0;
	m_hStream = NULL;
	m_bMP3Compressed = false;
	m_ulSrcBufferSize = 0;
	m_nRemainderBytes = 0;
	m_nRemainderOffset = 0;
}


void WaveFile::Close()
{
	m_hStream = NULL;

	Clear( );
}

// Open
bool WaveFile::Open (char* pszFilename, uint32 nFilePos)
{
    return false;
}

// Cue
//
bool WaveFile::Cue (void)
{
   return true;
}

unsigned int WaveFile::Read (unsigned char* pbDest, unsigned int cbSize)
{
    return 0;
}

unsigned int WaveFile::SeekFromStart( uint32 nBytes )
{
	if( !Cue( ))
		return 0;

	return Read( NULL, nBytes );
}

unsigned int WaveFile::ReadCompressed( unsigned char* pbDest, unsigned int cbSize, unsigned char* pCompressedBuffer, unsigned char* pDecompressedBuffer )
{
	return 0;
}


unsigned int WaveFile::SeekFromStartCompressed( uint32 nBytes )
{
	return 0;
}


unsigned char WaveFile::GetSilenceData (void)
{
    return 0;
}

//! CStream

class CStream : public CSample
{
public:
	CStream( CStream* pPrev, CStream* pNext );
	virtual ~CStream( );
	void HandleUpdate( COpenALSoundSys* pSoundSys );
	uint32 FillBuffer( COpenALSoundSys* pSoundSys );
	virtual void SetLooping( COpenALSoundSys* pSoundSys, bool bLoop );

	// Indicates whether sound is finished playing.
	bool		IsDone( );

	void ReadStreamIntoBuffer( COpenALSoundSys* pSoundSys, unsigned char* pBuffer, int32 nBufferSize );

	bool SetCurrentPosition( unsigned int dwStartOffset );

public:
	streamBufferParams_t m_streamBufferParams;
	uint32		m_uiNextWriteOffset;
	uint32		m_uiBufferSize;
	uint32		m_uiLastPlayPos;
	uint32		m_uiTotalPlayed;
	CStream*	m_pPrev;
	CStream*	m_pNext;
	WaveFile*   m_pWaveFile;
	int8		m_nEventNum;
};

CStream::CStream( CStream* pPrev, CStream* pNext ) :
	CSample( ), m_pPrev( pPrev ), m_pNext( pNext )
{
	if( m_pPrev != NULL )
	{
		m_pNext = m_pPrev->m_pNext;
		m_pPrev->m_pNext = this;
	}

	if( m_pNext != NULL )
	{
		m_pPrev = m_pNext->m_pPrev;
		m_pNext->m_pPrev = this;
	}

	m_uiBufferSize = 0;
	m_uiNextWriteOffset = 0;
	m_uiLastPlayPos = 0;
	m_uiTotalPlayed = 0;
	m_pWaveFile = NULL;
	m_nEventNum = -1;
}

CStream::~CStream( )
{
	if( m_pPrev != NULL )
		m_pPrev->m_pNext = m_pNext;

	if( m_pNext != NULL )
		m_pNext->m_pPrev = m_pPrev;
}


void CStream::ReadStreamIntoBuffer( COpenALSoundSys* pSoundSys, unsigned char* pBuffer, int32 nBufferSize )
{
	PRINTSTUB;
}


// stream in more data when we get an
// update event
void CStream::HandleUpdate( COpenALSoundSys* pSoundSys )
{
	//TODO: Maybe use mpg123
	PRINTSTUB;
}

bool CStream::IsDone( )
{
	// Looping sounds are never done.
	if( IsLooping( ))
		return false;

	if( m_uiTotalPlayed < m_pWaveFile->GetMaxBytesCopied( ))
		return false;

	return true;
}

// fills in stream at current update location
uint32 CStream::FillBuffer( COpenALSoundSys* pSoundSys )
{
    ALenum fmt;
	if( m_pSoundData == NULL)
	{
		printf("Error in Fill - data is null!\n");
		return false;
	}
	if( m_uiSoundDataLen == 0)
	{
		printf("Error in Fill - m_uiSoundDataLen is zero!\n");
		return false;
	}
	if(source == 0 )
	{
		printf("Error in Fill - source is zero!\n");
		return false;
	}

	// Make sure we're stopped.  Shouldn't get here, since buffer's are stopped
	// when the sound ends.
	if( IsPlaying( ))
	{
		ASSERT( !"CSample::Fill:  Buffer should have been stopped before reaching fill." );
	}

	switch (m_pWaveFile->m_wfmt.wBitsPerSample)
	{
	case 8:
		fmt = m_waveFormat.nChannels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		break;
	case 16:
		fmt = m_waveFormat.nChannels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		break;
	default:
		printf("Error in %s:%i: bps is invalid, got %i!\n", __FILE__, __LINE__, m_waveFormat.wBitsPerSample);
		return false;
	}

	alBufferData(buffer, fmt, m_pSoundData, m_uiSoundDataLen, m_pWaveFile->m_wfmt.nSamplesPerSec);

	return 1;
}

void CStream::SetLooping( COpenALSoundSys* pSoundSys, bool bLoop )
{
	m_bLooping = bLoop;
}

bool CStream::SetCurrentPosition( unsigned int dwStartOffset )
{
	if( source == 0 )
		return false;

	alSourcef(source, AL_BYTE_OFFSET, dwStartOffset);

	// Set the last play pos to zero, because doing this on a playing sound
	// seems to not really set it to exactly dwStartOffset anyway.
	m_nLastPlayPos = 0;

	m_uiNextWriteOffset = 0;
	m_uiLastPlayPos = 0;
	m_uiTotalPlayed = dwStartOffset;

	return true;
}

//! COpenALSoundSys

class COpenALSoundSys : public ILTSoundSys
{
    char error[1024];
public:
	COpenALSoundSys( ) {
        memset(error, 0, sizeof(error));
        strncpy(error, "no error", sizeof(error));
        alcontext = NULL;
        aldevice = NULL;
    }
	virtual ~COpenALSoundSys( ) {}

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
	virtual S32			WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut,
		S32 siDeviceId, void* pWaveFormat );
	virtual void		WaveOutClose( LHDIGDRIVER hDriver );
	virtual void		SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume );
	virtual S32			GetDigitalMasterVolume( LHDIGDRIVER hDig );
	virtual S32			DigitalHandleRelease( LHDIGDRIVER hDriver );
	virtual S32			DigitalHandleReacquire( LHDIGDRIVER hDriver );
#ifdef USE_EAX20_HARDWARE_FILTERS
	virtual bool		SetEAX20Filter(bool bEnable, LTSOUNDFILTERDATA* pFilterData) { return true; }
	virtual bool		SupportsEAX20Filter() { return true; }
	virtual bool		SetEAX20BufferSettings(LHSAMPLE hSample, LTSOUNDFILTERDATA* pFilterData) { return true; }
#endif

	// 3d sound provider functions
    virtual void        Set3DProviderMinBuffers(uint32 uiMinBuffers){};
	virtual sint32      Open3DProvider( LHPROVIDER hLib );
	virtual void		Close3DProvider( LHPROVIDER hLib );
	virtual void		Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal );
	virtual void		Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal );
	virtual S32			Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, const char** psName);
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
	virtual sint32		Init3DSampleFromAddress( LH3DSAMPLE hS, void* pStart,
		uint32 uiLen, void* pWaveFormat, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData  );
	virtual sint32		Init3DSampleFromFile( LH3DSAMPLE hS, void* pFile_image,
		sint32 siBlock, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData );
	virtual sint32		Get3DSampleVolume( LH3DSAMPLE hS ) {return 0;};
	virtual void		Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume );
	virtual U32			Get3DSampleStatus( LH3DSAMPLE hS );
	virtual void		Set3DSampleMsPosition( LHSAMPLE hS, sint32 siMilliseconds );
	virtual S32			Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo );
	virtual void		Set3DSamplePlaybackRate( LH3DSAMPLE hS, S32 siPlayback_rate );
	virtual void		Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist );
	virtual void		Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal );
	virtual void		Set3DSampleLoopBlock( LH3DSAMPLE hS, sint32 siLoop_start_offset, sint32 siLoop_end_offset, bool bEnable );
	virtual void		Set3DSampleLoop( LH3DSAMPLE hS, bool bLoop );
	virtual void		Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction );
	virtual float		Get3DSampleObstruction( LH3DSAMPLE hS );
	virtual void		Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion );
	virtual float		Get3DSampleOcclusion( LH3DSAMPLE hS );

	// 2d sound sample functions
	virtual LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig );
	virtual void		ReleaseSampleHandle( LHSAMPLE hS );
	virtual void		InitSample( LHSAMPLE hS );
	virtual sint32		InitSampleFromAddress( LHSAMPLE hS, void* pStart, uint32 uiLen, void* pWaveFormat, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData );
	virtual sint32		InitSampleFromFile( LHSAMPLE hS, void* pFile_image, sint32 siBlock, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData );
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
	virtual void		SetSampleLoop( LHSAMPLE hS, bool bLoop );
	virtual void		SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds );
	virtual sint32		GetSampleUserData( LHSAMPLE hS, U32 uiIndex );
	virtual uint32		GetSampleStatus( LHSAMPLE hS );

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
	bool SetSampleNotify( CSample* pSample, bool bEnable );
	static COpenALSoundSys m_OpenALSoundSys;
	static const char*	m_pCOpenALSoundSysDesc;
	WAVEFORMATEX		sys_waveFormat;
	ALCdevice* aldevice;
	ALCcontext* alcontext;
	int m_hResult;
	WaveFile			m_WaveStream[MAX_WAVE_STREAMS];
	CStream*			m_pStreams;
};

bool COpenALSoundSys::Init( )
{
	LT_MEM_TRACK_ALLOC(m_pStreams = new CStream( NULL, NULL ),LT_MEM_TYPE_SOUND);
	return true;
}

void COpenALSoundSys::Term( )
{
}

void* COpenALSoundSys::GetDDInterface( uint uiDDInterfaceId )
{
	return NULL;
}

// system wide functions
void COpenALSoundSys::Lock( void )
{
}

void COpenALSoundSys::Unlock( void )
{
}

S32	COpenALSoundSys::Startup( void )
{
	int mpg_error;
	if (alcontext != NULL)
	{
		return 0;
	}

	aldevice = alcOpenDevice(0);
	alcontext = alcCreateContext(aldevice, 0);
	alcMakeContextCurrent(alcontext);

	mpg_error = mpg123_init();

	if (mpg_error != MPG123_OK)
	{
		printf("Couldn't init MPG123!\n");
	}

	return 0;
}

void COpenALSoundSys::Shutdown( void )
{
	alcMakeContextCurrent(0);
	if (alcontext)
	{
		alcDestroyContext(alcontext);
		alcontext = NULL;
	}
	if (aldevice)
	{
		alcCloseDevice(aldevice);
		aldevice = NULL;
	}

	while( m_pStreams != NULL )
	{
		CStream* pStream = m_pStreams;
		m_pStreams = m_pStreams->m_pNext;
		delete pStream;
	}

	mpg123_exit();
}

U32	COpenALSoundSys::MsCount( void )
{
	return SDL_GetTicks();
}

S32	COpenALSoundSys::SetPreference( U32 uiNumber, S32 siValue )
{
	return 0;
}

S32	COpenALSoundSys::GetPreference( U32 uiNumber )
{
	return 0;
}

void COpenALSoundSys::MemFreeLock( void* ptr )
{
	LTMemFree( ptr );
}

void* COpenALSoundSys::MemAllocLock( U32 uiSize )
{
	void* p;
	LT_MEM_TRACK_ALLOC(p = LTMemAlloc( uiSize ),LT_MEM_TYPE_SOUND);
	return p;
}

char* COpenALSoundSys::LastError( void )
{
	return error;
}

// digital sound driver functions
S32	COpenALSoundSys::WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut,
	S32 siDeviceId, void* pWaveFormat )
{
	*phDriver = alcontext;
	return 0;
}

void COpenALSoundSys::WaveOutClose( LHDIGDRIVER hDriver )
{
}

#define DSBVOLUME_MAX 0
#define DSBVOLUME_MIN -10000

#define MAX_MASTER_VOLUME	127.0f
#define MIN_MASTER_VOLUME	1.0f
#define DEL_MASTER_VOLUME	( float )( MAX_MASTER_VOLUME - MIN_MASTER_VOLUME )

#define DSBVOLUME_DEL		( float )( DSBVOLUME_MAX - DSBVOLUME_MIN )

#define LOG_ATTENUATION		( float )sqrt( MAX_MASTER_VOLUME )
#define LOG_INVATTENUATION	( 1.0f / LOG_ATTENUATION )

#define CONVERT_LIN_VOL_TO_LOG_VOL( linVol, logVol )	\
	{	\
		float t = ( 1.0f - ( MIN_MASTER_VOLUME / ( float )linVol ) ) * ( MAX_MASTER_VOLUME / DEL_MASTER_VOLUME );	\
		t = ( float )pow( t, LOG_ATTENUATION );	\
		logVol = DSBVOLUME_MIN + ( long )( t * DSBVOLUME_DEL );	\
	}	\

#define CONVERT_LOG_VOL_TO_LIN_VOL( logVol, linVol )	\
	{	\
		float t = ( float )( logVol - DSBVOLUME_MIN ) / DSBVOLUME_DEL;	\
		t = ( float )pow( t, LOG_INVATTENUATION );	\
		linVol = ( S32 )( ( MAX_MASTER_VOLUME * MIN_MASTER_VOLUME ) / ( MAX_MASTER_VOLUME - ( t * DEL_MASTER_VOLUME ) ) );	\
	}	\

void COpenALSoundSys::SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume )
{
}

S32	COpenALSoundSys::GetDigitalMasterVolume( LHDIGDRIVER hDig )
{
	return 0;
}

S32	COpenALSoundSys::DigitalHandleRelease( LHDIGDRIVER hDriver )
{
	return 0;
}

S32	COpenALSoundSys::DigitalHandleReacquire( LHDIGDRIVER hDriver )
{
	return 0;
}

// 3d sound provider functions
sint32 COpenALSoundSys::Open3DProvider( LHPROVIDER hLib )
{
	return LTTRUE;
}

void COpenALSoundSys::Close3DProvider( LHPROVIDER hLib )
{
}

void COpenALSoundSys::Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal )
{
}

void COpenALSoundSys::Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal )
{
	int *pnVal = (int*) pVal;
    if (strcmp(sName, "Max samples") == 0)
	{
		*pnVal =  64;
	}
}

S32	COpenALSoundSys::Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, const char** psName)
{
	int nCur = *phNext;
	phNext[0] += 1;

	if (nCur == NUM_PROVIDERS)
	{
		*psName = NULL;
		*phDest = 0;
		return 0;
	}

	switch (nCur)
	{
		case 0:
			//done to make CGameClientShell::InitSound happy and make CSoundMgr
			//think we have a 3D sound provider
			*psName = "DirectSound Hardware";
			*phDest = 1;
		break;
	}

	return 1;
}

S32	COpenALSoundSys::Get3DRoomType( LHPROVIDER hLib )
{
	return 0;
}

void COpenALSoundSys::Set3DRoomType( LHPROVIDER hLib, S32 siRoomType )
{
}

// 3d listener functions
LH3DPOBJECT	COpenALSoundSys::Open3DListener( LHPROVIDER hLib )
{
	return alcontext;
}

void COpenALSoundSys::Close3DListener( LH3DPOBJECT hListener )
{
}

// 3d sound object functions
void COpenALSoundSys::Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ)
{
	if (hObj == alcontext)
	{
		alListener3f(AL_POSITION, fX, fY, fZ);
	}
	else
	{
		//printf("STUB: Set3DPosition for a non-listener\n");
	}
}

void COpenALSoundSys::Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms )
{
}

void COpenALSoundSys::Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up )
{
	ALfloat ori[6];
	ori[0] = fX_face;
	ori[1] = fY_face;
	ori[2] = fZ_face;
	ori[3] = fX_up;
	ori[4] = fY_up;
	ori[5] = fZ_up;
	alListenerfv(AL_ORIENTATION, ori);
}

void COpenALSoundSys::Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, uintptr_t siValue )
{
	if( hObj == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;
	p3DObject->m_userData[ uiIndex ] = siValue;
}

void COpenALSoundSys::Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ)
{
}

void COpenALSoundSys::Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms )
{
}

void COpenALSoundSys::Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up )
{
}

S32	COpenALSoundSys::Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex)
{
	if( hObj == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return 0;

	I3DObject* p3DObject = ( I3DObject* )hObj;
	return p3DObject->m_userData[ uiIndex ];
}

// 3d sound sample functions
LH3DSAMPLE COpenALSoundSys::Allocate3DSampleHandle( LHPROVIDER hLib )
{
	C3DSample* p3DSample;
	LT_MEM_TRACK_ALLOC(p3DSample = new C3DSample,LT_MEM_TYPE_SOUND);

	// 3d sounds must be mono
	CSample* pSample = &p3DSample->m_sample;
	pSample->m_waveFormat.nChannels = 1;

    // Create buffer.
	p3DSample->Init( m_hResult, NULL, 44100, &pSample->m_waveFormat, NULL );


	return p3DSample;
}

void COpenALSoundSys::Release3DSampleHandle( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	delete p3DSample;
}

void COpenALSoundSys::Stop3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	CSample* pSample = &p3DSample->m_sample;
	StopSample(pSample);
}

void COpenALSoundSys::Start3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	CSample* pSample = &p3DSample->m_sample;
	ResumeSample(pSample);
}

void COpenALSoundSys::Resume3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	p3DSample->m_sample.Play( );
	p3DSample->m_status = LS_PLAYING;
	char* m_pcLastError = LastError( );
}

void COpenALSoundSys::End3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	if( p3DSample->m_sample.buffer != 0 )
	{
		m_hResult = p3DSample->m_sample.Stop( true );
	}
	p3DSample->m_status = LS_DONE;

	SetSampleNotify( &p3DSample->m_sample, false );
	char* m_pcLastError = LastError( );
}

sint32 COpenALSoundSys::Init3DSampleFromAddress( LH3DSAMPLE hS, void* pStart,
	uint32 uiLen, void* pWaveFormat, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData  )
{
	WAVEFORMATEX* cast = (WAVEFORMATEX*)pWaveFormat;
	if( hS == NULL || pStart == NULL || uiLen == 0 || pWaveFormat == NULL )
		return LTFALSE;

	C3DSample* p3DSample = ( C3DSample* )hS;
	CSample* pSample = &p3DSample->m_sample;

	// Modify the pitch.
	WAVEFORMATEX waveFormat = *cast;
	waveFormat.nSamplesPerSec = siPlaybackRate;
	waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;

	if ( !p3DSample->Init( m_hResult, NULL, uiLen, ( WAVEFORMATEX* )&waveFormat, pFilterData ) )
		return LTFALSE;

	pSample->m_pSoundData = (char*)pStart;
	pSample->m_uiSoundDataLen = uiLen;

	if( !pSample->Fill( ))
	{
		p3DSample->Term( );
		return LTFALSE;
	}

	// set up new looping, if appropriate
	if  ( !SetSampleNotify( pSample, true ) )
		return LTFALSE;

	return LTTRUE;
}

static bool ParseWaveFile( void* pWaveFileBlock, void*& rpWaveFormat, uint32& ruiWaveFormatSize,
	void*& rpSampleData, uint32& ruiSampleDataSize )
{

	ruiSampleDataSize = ruiWaveFormatSize = 0;
	rpSampleData = rpWaveFormat = NULL;

	uint8* pucFileBlock = ( uint8* )pWaveFileBlock;

	bool bFinished = false;
	while( !bFinished )
	{

		uint32 uiChunkId = *( ( uint32* )pucFileBlock );
		pucFileBlock += sizeof( uint32 );

		switch( uiChunkId )
		{
			case CHUNK_ID_RIFF:
			{
				uint32 uiRiffSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );
				break;
			}

			case CHUNK_ID_WAVE:
			{
				// skip this
				break;
//				uint32 uiFmtWord = *( ( uint32* )pucFileBlock );
//				pucFileBlock += sizeof( uint32 );

			}

			case CHUNK_ID_FACT:
			{
				uint32 uiFactSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				uint32* puiFactData = ( uint32* )pucFileBlock;
				pucFileBlock += uiFactSize;
				break;
			}

			case CHUNK_ID_DATA:
			{
				uint32 uiDataSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				rpSampleData = ( void* )pucFileBlock;
				ruiSampleDataSize = uiDataSize;
				pucFileBlock += uiDataSize;

				bFinished = true;
				break;
			}

			case CHUNK_ID_WAVH:
			case CHUNK_ID_GUID:
			{
				// just skip these
				uint32 uiSkipSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				uint32* pucSkipData = ( uint32* )pucFileBlock;
				pucFileBlock += uiSkipSize;
				break;
			}

			case CHUNK_ID_FMT:
			{
				uint32 uiFmtSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				rpWaveFormat = ( void* )pucFileBlock;
				ruiWaveFormatSize = uiFmtSize;
				pucFileBlock += uiFmtSize;
				break;
			}

			default:
			{
				return false;
			}
		}
	}

	bool bSuccess = ( ( rpWaveFormat != NULL ) && ( rpSampleData != NULL ) );
	return bSuccess;

}

#define DECOMPRESSION_BUFFER_PAD	16384

sint32	COpenALSoundSys::Init3DSampleFromFile( LH3DSAMPLE hS, void* pFile_image,
	sint32 siBlock, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData )
{
	if( hS == NULL )
		return LTFALSE;

	C3DSample* p3DSample = ( C3DSample* ) hS;
	CSample* pSample = &p3DSample->m_sample;
	WaveFile wave;
	char* PCM;
	unsigned int uncompressedSize;

	bool bSuccess = false;

	uint32 uiWaveFormatSize = 0;
	uint32 uiSampleDataSize = 0;

	void* pWaveFormat = NULL;
	void* pSampleData = NULL;

	bSuccess = ParseWaveFile( pFile_image, pWaveFormat, uiWaveFormatSize, pSampleData, uiSampleDataSize );
	if( !bSuccess )
		return LTFALSE;

	// check if we're a PCM format and convert the compressed data
	// to PCM if not

	WAVEFORMATEX* pWaveFormatEx = ( WAVEFORMATEX* )pWaveFormat;

	// if we have more than one channel, we fail
	// 3D sounds can't be stereo
	if ( pWaveFormatEx->nChannels > 1 )
	{
		printf("3D sample has too many channels!\n");
		return LTFALSE;
	}

	if( pWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM )
	{
		return Init3DSampleFromAddress( hS, pSampleData, uiSampleDataSize, pWaveFormatEx, siPlaybackRate, pFilterData  );
	}
	else if (pWaveFormatEx->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
	{
		uint32 dwSamplesPerBlock = 4 << ( pWaveFormatEx->nChannels / 2 );
		uint32 dwSamples;
		dwSamplesPerBlock = 1 + ( pWaveFormatEx->nBlockAlign - dwSamplesPerBlock ) * 8 / dwSamplesPerBlock;
		dwSamples = (( uiSampleDataSize + pWaveFormatEx->nBlockAlign - 1 ) /
			pWaveFormatEx->nBlockAlign ) * dwSamplesPerBlock;

		PCM = adpcm_decode_data(pSampleData, pWaveFormatEx->nChannels, dwSamples, pWaveFormatEx->nBlockAlign);

		pWaveFormatEx->wBitsPerSample = 16;

		uiSampleDataSize = dwSamples * 2 * pWaveFormatEx->nChannels;
		pSampleData = PCM;

		if ( !Init3DSampleFromAddress( hS, pSampleData, uiSampleDataSize, pWaveFormatEx, siPlaybackRate, pFilterData ) )
		{
			delete [] PCM;
			return LTFALSE;
		}
		pSample->m_bAllocatedSoundData = true;
	}
	else if( pWaveFormatEx->wFormatTag == WAVE_FORMAT_MPEGLAYER3 )
	{
		PCM = mpeg3_decode ((char*)pFile_image, uiSampleDataSize, &uncompressedSize, pWaveFormatEx);

		if( !pSample->Init( m_hResult, NULL, uncompressedSize, false, pWaveFormatEx, pFilterData ) )
			return LTFALSE;

		pSample->m_pSoundData = PCM;
		pSample->m_bAllocatedSoundData = true;
		pSample->m_uiSoundDataLen = uncompressedSize;

		if( !pSample->Fill( ))
		{
			p3DSample->Term( );
			return LTFALSE;
		}

		// set up new looping, if appropriate
		if  ( !SetSampleNotify( pSample, true ) )
			return LTFALSE;
	}
	else
	{
		printf("error in Init3DSampleFromFile - format tag %i not handled yet!\n", pWaveFormatEx->wFormatTag);
	}

	return LTTRUE;
}

void COpenALSoundSys::Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	float vol;
	// p3DSample->m_sample.Restore( );

	if( siVolume < ( S32 )MIN_MASTER_VOLUME )
		siVolume = ( S32 )MIN_MASTER_VOLUME;

	if( siVolume > ( S32 )MAX_MASTER_VOLUME )
		siVolume = ( S32 )MAX_MASTER_VOLUME;

	long lDSVolume;
	CONVERT_LIN_VOL_TO_LOG_VOL( siVolume, lDSVolume );

	vol = 1.0f / 10000.0f;
	vol *= (lDSVolume + 10000.0f);

	alSourcef(p3DSample->m_sample.source, AL_GAIN, vol);
}

U32	COpenALSoundSys::Get3DSampleStatus( LH3DSAMPLE hS )
{
	C3DSample* p3DSample = (C3DSample*) hS;
	return ( p3DSample->m_sample.IsPlaying( ) ? LS_PLAYING : LS_STOPPED );
}

void COpenALSoundSys::Set3DSampleMsPosition( LHSAMPLE hS, sint32 siMilliseconds )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	SetSampleMsPosition(( LHSAMPLE )&p3DSample->m_sample, siMilliseconds );
}

S32	COpenALSoundSys::Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo )
{
	return 0;
}

void COpenALSoundSys::Set3DSamplePlaybackRate( LH3DSAMPLE hS, S32 siPlayback_rate )
{
}

void COpenALSoundSys::Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist )
{
}

void COpenALSoundSys::Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal )
{
}

void COpenALSoundSys::Set3DSampleLoopBlock( LH3DSAMPLE hS, sint32 siLoop_start_offset, sint32 siLoop_end_offset, bool bEnable )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	CSample* pSample = &p3DSample->m_sample;
	pSample->m_nLoopStart = siLoop_start_offset;
	pSample->m_nLoopEnd = siLoop_end_offset;
	pSample->m_bLoopBlock = bEnable;
}

void COpenALSoundSys::Set3DSampleLoop( LH3DSAMPLE hS, bool bLoop )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	p3DSample->m_sample.SetLooping( this, bLoop );
}


void COpenALSoundSys::Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction )
{
}

float COpenALSoundSys::Get3DSampleObstruction( LH3DSAMPLE hS )
{
	return 0;
}

void COpenALSoundSys::Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion )
{
}

float COpenALSoundSys::Get3DSampleOcclusion( LH3DSAMPLE hS )
{
	return 0;
}

// 2d sound sample functions
LHSAMPLE COpenALSoundSys::AllocateSampleHandle( LHDIGDRIVER hDig )
{
	CSample* pSample;
	LT_MEM_TRACK_ALLOC(pSample = new CSample,LT_MEM_TYPE_SOUND);

    // Create buffer.
	pSample->Init( m_hResult, NULL, 44100, false );

	SetSampleNotify( pSample, true );

	return pSample;
}

void COpenALSoundSys::ReleaseSampleHandle( LHSAMPLE hS )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;

	delete pSample;
}

void COpenALSoundSys::InitSample( LHSAMPLE hS )
{
}

void COpenALSoundSys::StopSample( LHSAMPLE hS )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	// pSample->Restore( );
	m_hResult = pSample->Stop( true );

	SetSampleNotify( pSample, false );
	char* m_pcLastError = LastError( );
}

void COpenALSoundSys::StartSample( LHSAMPLE hS )
{
}

void COpenALSoundSys::ResumeSample( LHSAMPLE hS )
{
	//	LOG_WRITE( g_pLogFile, "ResumeSample( %x )\n", hS );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	m_hResult = pSample->Play( );
	char* m_pcLastError = LastError( );
}

void COpenALSoundSys::EndSample( LHSAMPLE hS )
{
}

static float lerp(float x, float x0, float x1, float y0, float y1)
{
	float ret = (y1 - y0) / (x1 - x0);
	ret *= (x - x0);
	ret += y0;
	return ret;
}

void COpenALSoundSys::SetSampleVolume( LHSAMPLE hS, S32 siVolume )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	// pSample->Restore( );
	float vol;

	if( siVolume < ( S32 )MIN_MASTER_VOLUME )
		siVolume = ( S32 )MIN_MASTER_VOLUME;

	if( siVolume > ( S32 )MAX_MASTER_VOLUME )
		siVolume = ( S32 )MAX_MASTER_VOLUME;

	long lDSVolume;
	CONVERT_LIN_VOL_TO_LOG_VOL( siVolume, lDSVolume );

	vol = 1.0f / 10000.0f;
	vol *= (lDSVolume + 10000.0f);

	alSourcef(pSample->source, AL_GAIN, vol);
}

void COpenALSoundSys::SetSamplePan( LHSAMPLE hS, S32 siPan )
{
}

S32	COpenALSoundSys::GetSampleVolume( LHSAMPLE hS )
{
	return 0;
}

S32	COpenALSoundSys::GetSamplePan( LHSAMPLE hS )
{
	return 0;
}

void COpenALSoundSys::SetSampleUserData( LHSAMPLE hS, U32 uiIndex, uintptr_t siValue )
{
	if( hS == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->m_userData[ uiIndex ] = siValue;
}

void COpenALSoundSys::GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB )
{
}

void COpenALSoundSys::SetSampleType( LHSAMPLE hS, S32 siFormat, U32 uiFlags )
{
}

void COpenALSoundSys::SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time )
{
}

void COpenALSoundSys::SetSampleAddress( LHSAMPLE hS, void* pStart, U32 uiLen )
{
}

S32	COpenALSoundSys::SetSampleFile( LHSAMPLE hS, void* pFile_image, S32 siBlock )
{
	return 0;
}

void COpenALSoundSys::SetSamplePlaybackRate( LHSAMPLE hS, S32 siPlayback_rate )
{
}

void COpenALSoundSys::SetSampleLoopBlock( LHSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset, bool bEnable )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->m_nLoopStart = siLoop_start_offset;
	pSample->m_nLoopEnd = siLoop_end_offset;
	pSample->m_bLoopBlock = bEnable;
}

void COpenALSoundSys::SetSampleLoop( LHSAMPLE hS, bool bLoop )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	pSample->SetLooping( this, bLoop );
}

void COpenALSoundSys::SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	if( siMilliseconds < 0 )
		siMilliseconds = 0;

	ALint sizeInBytes;
	ALint channels;
	ALint bits;
	ALint frequency;
	float durationInMilliseconds;
	float offset_frac;
	int byte_offset;
	int lengthInSamples;

	alGetBufferi(pSample->buffer, AL_SIZE, &sizeInBytes);
	alGetBufferi(pSample->buffer, AL_CHANNELS, &channels);
	alGetBufferi(pSample->buffer, AL_BITS, &bits);
	alGetBufferi(pSample->buffer, AL_FREQUENCY, &frequency);

	lengthInSamples = sizeInBytes * 8 / (channels * bits);
	durationInMilliseconds = ((float)lengthInSamples / (float)frequency) * 1000.0f;
	offset_frac = (float)siMilliseconds / durationInMilliseconds;
	byte_offset = offset_frac * sizeInBytes;

	m_hResult = pSample->SetCurrentPosition( byte_offset );

	char* m_pcLastError = LastError( );
}

S32	COpenALSoundSys::GetSampleUserData( LHSAMPLE hS, U32 uiIndex )
{
	if( hS == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return 0;

	CSample* pSample = ( CSample* )hS;
	return pSample->m_userData[ uiIndex ];
}

uint32		COpenALSoundSys::GetSampleStatus( LHSAMPLE hS )
{
	CSample* pSample = ( CSample* )hS;
	sint32 siStatus = ( pSample->IsPlaying( ) ? LS_PLAYING : LS_STOPPED );
	return siStatus;
}

// old 2d sound stream functions
LHSTREAM COpenALSoundSys::OpenStream( char* sFilename, uint32 nOffset, LHDIGDRIVER hDig, char* sStream, S32 siStream_mem )
{
	LHSTREAM hStream = NULL;
	WaveFile* pWaveFile;
	char* PCM;
	int i;
	bool bSuccess = false;
	unsigned int uncompressedSize;

	uint32 uiWaveFormatSize = 0;
	uint32 uiSampleDataSize = 0;

	void* pWaveFormat = NULL;
	void* pSampleData = NULL;

	bSuccess = ParseWaveFile( sStream, pWaveFormat, uiWaveFormatSize, pSampleData, uiSampleDataSize );
	if( !bSuccess )
		return NULL;

	WAVEFORMATEX* pWaveFormatEx = ( WAVEFORMATEX* )pWaveFormat;

	for(i = 0; i < MAX_WAVE_STREAMS; i++)
	{
		if(!m_WaveStream[i].IsActive())
		{
			PCM = NULL;
			if( pWaveFormatEx->wFormatTag == WAVE_FORMAT_MPEGLAYER3 )
			{
				PCM = mpeg3_decode (sStream, siStream_mem, &uncompressedSize, &m_WaveStream[i].m_wfmt);
			}
			else if( pWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM )
			{
				printf("error in OpenStream for %s - PCM not handled yet!\n", sFilename);
			}
			else if (pWaveFormatEx->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
			{
				printf("error in OpenStream for %s - ADPCM not handled yet!\n", sFilename);
			}
			else
			{
				printf("error in OpenStream for %s - format %i not handled yet!\n", sFilename, pWaveFormatEx->wFormatTag);
			}

			if(PCM == NULL)
			{
				return NULL;
			}
			else
			{
				break;
			}
		}
	}

	// Error: all streams are full (max = MAX_WAVE_STREAMS)
	if(i == MAX_WAVE_STREAMS)
		return NULL;

	pWaveFile = &m_WaveStream[i];

	CStream* pStream;
	LT_MEM_TRACK_ALLOC(pStream = new CStream( m_pStreams, NULL ),LT_MEM_TYPE_SOUND);

	pStream->Reset();

	pStream->Init( m_hResult, NULL, uncompressedSize, false, NULL );
	pStream->m_pSoundData = PCM;
	pStream->m_uiSoundDataLen = uncompressedSize;
	pStream->m_pWaveFile = pWaveFile;
	pStream->m_uiNextWriteOffset = 0;
	pStream->m_uiLastPlayPos = 0;
	pStream->m_uiTotalPlayed = 0;

	if ( !pStream->FillBuffer( this ) )
	{
		return NULL;
	}

	hStream = ( LHSTREAM )pStream;

	return hStream;
}

void COpenALSoundSys::SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate )
{
}

void COpenALSoundSys::SetStreamMsPosition( LHSTREAM hS, S32 siMilliseconds )
{
	if( hS == NULL )
		return;

	if( siMilliseconds < 0 )
		siMilliseconds = 0;

	CStream* pStream = ( CStream* )hS;
	ALint sizeInBytes;
	ALint channels;
	ALint bits;
	ALint frequency;
	float durationInMilliseconds;
	float offset_frac;
	int byte_offset;
	int lengthInSamples;

	alGetBufferi(pStream->buffer, AL_SIZE, &sizeInBytes);
	alGetBufferi(pStream->buffer, AL_CHANNELS, &channels);
	alGetBufferi(pStream->buffer, AL_BITS, &bits);
	alGetBufferi(pStream->buffer, AL_FREQUENCY, &frequency);

	lengthInSamples = sizeInBytes * 8 / (channels * bits);
	durationInMilliseconds = ((float)lengthInSamples / (float)frequency) * 1000.0f;
	offset_frac = (float)siMilliseconds / durationInMilliseconds;
	byte_offset = offset_frac * sizeInBytes;

	m_hResult = pStream->SetCurrentPosition( byte_offset );
	char* m_pcLastError = LastError( );
}

void COpenALSoundSys::SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue)
{
}

S32	COpenALSoundSys::GetStreamUserData( LHSTREAM hS, U32 uiIndex)
{
	return 0;
}

// new 2d sound stream functions
void COpenALSoundSys::CloseStream( LHSTREAM hStream )
{
	PauseStream( hStream, 1 );
	for(int i = 0; i < MAX_WAVE_STREAMS; i++)
	{
		if(m_WaveStream[i].GetStream() == hStream)
		{
			m_WaveStream[i].Close();
			break;
		}
	}
	CStream* pStream = ( CStream* )hStream;
	delete pStream;
}

void COpenALSoundSys::StartStream( LHSTREAM hStream )
{
	CStream* pStream = ( CStream* )hStream;
	pStream->Stop( true );
	pStream->Play( );
}

void COpenALSoundSys::PauseStream( LHSTREAM hStream, S32 siOnOff )
{
	CStream* pStream = ( CStream* )hStream;
	if( siOnOff == 1 )
		pStream->Stop( false );
	else
	{
		pStream->Play( );
	}
}

void COpenALSoundSys::ResetStream( LHSTREAM hStream )
{
}

void COpenALSoundSys::SetStreamVolume( LHSTREAM hStream, S32 siVolume )
{
}

void COpenALSoundSys::SetStreamPan( LHSTREAM hStream, S32 siPan )
{
}

S32	COpenALSoundSys::GetStreamVolume( LHSTREAM hStream )
{
	return 0;
}

S32	COpenALSoundSys::GetStreamPan( LHSTREAM hStream )
{
	return 0;
}

uint32 COpenALSoundSys::GetStreamStatus( LHSTREAM hStream )
{
	return 0;
}

sint32 COpenALSoundSys::GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam )
{
	return 0;
}

void COpenALSoundSys::ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue )
{
}

bool COpenALSoundSys::QueueStreamData( LHSTREAM, uint8* pucPCMSoundData, uint uiNumBytes )
{
	return false;
}

sint32 COpenALSoundSys::GetStreamDataQueueSize( LHSTREAM hStream )
{
	return 0;
}

// wave file decompression functons
S32	COpenALSoundSys::DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, U32* puiOutSize )
{
	return 0;
}

S32	COpenALSoundSys::DecompressASI( void* pInData, U32 uiInSize, char* sFilename_ext, void** ppWav, U32* puiWavSize, LTLENGTHYCB fnCallback )
{
	return 0;
}

bool COpenALSoundSys::SetSampleNotify( CSample* pSample, bool bEnable )
{

	return true;
}

sint32 COpenALSoundSys::InitSampleFromAddress( LHSAMPLE hS, void* pStart, uint32 uiLen, void* pWaveFormat, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData )
{
//	LOG_WRITE( g_pLogFile, "InitSampleFromAddress( %x, %x, %d )\n", hS, pStart, uiLen );
	WAVEFORMATEX* cast = (WAVEFORMATEX*)pWaveFormat;
	if( hS == NULL || pStart == NULL || uiLen == 0 || !pWaveFormat )
		return LTFALSE;

	// Modify the pitch.
	WAVEFORMATEX waveFormat = *cast;
	waveFormat.nSamplesPerSec = siPlaybackRate;
	waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
	CSample* pSample = ( CSample* )hS;
	if( !pSample->Init( m_hResult, NULL, uiLen, false, ( WAVEFORMATEX* )&waveFormat, pFilterData ))
		return LTFALSE;

	pSample->m_pSoundData = (char*)pStart;
	pSample->m_uiSoundDataLen = uiLen;

	if( !pSample->Fill( ))
	{
		pSample->Term( );
		return LTFALSE;
	}

	SetSampleNotify( pSample, true );

	return LTTRUE;
}

sint32 COpenALSoundSys::InitSampleFromFile( LHSAMPLE hS, void* pFile_image, sint32 siBlock, sint32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData )
{
	if( hS == NULL )
		return LTFALSE;

	CSample* pSample = ( CSample* ) hS;
	WaveFile wave;
	char* PCM;

	bool bSuccess = false;
	unsigned int uncompressedSize;

	uint32 uiWaveFormatSize = 0;
	uint32 uiSampleDataSize = 0;

	void* pWaveFormat = NULL;
	void* pSampleData = NULL;

	bSuccess = ParseWaveFile( pFile_image, pWaveFormat, uiWaveFormatSize, pSampleData, uiSampleDataSize );
	if( !bSuccess )
		return LTFALSE;

	WAVEFORMATEX* pWaveFormatEx = ( WAVEFORMATEX* )pWaveFormat;

	if( pWaveFormatEx->wFormatTag == WAVE_FORMAT_MPEGLAYER3 )
	{
		PCM = mpeg3_decode ((char*)pFile_image, uiSampleDataSize, &uncompressedSize, pWaveFormatEx);

		if( !pSample->Init( m_hResult, NULL, uncompressedSize, false, pWaveFormatEx, pFilterData ) )
			return LTFALSE;

		pSample->m_pSoundData = PCM;
		pSample->m_bAllocatedSoundData = true;
		pSample->m_uiSoundDataLen = uncompressedSize;

		if( !pSample->Fill( ))
		{
			pSample->Term( );
			return LTFALSE;
		}

		// set up new looping, if appropriate
		if  ( !SetSampleNotify( pSample, true ) )
			return LTFALSE;
	}
	else if( pWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM )
	{
		return InitSampleFromAddress(hS, pSampleData, uiSampleDataSize, pWaveFormatEx, siPlaybackRate, pFilterData);
	}
	else if (pWaveFormatEx->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
	{
		uint32 dwSamplesPerBlock = 4 << ( pWaveFormatEx->nChannels / 2 );
		uint32 dwSamples;
		dwSamplesPerBlock = 1 + ( pWaveFormatEx->nBlockAlign - dwSamplesPerBlock ) * 8 / dwSamplesPerBlock;
		dwSamples = (( uiSampleDataSize + pWaveFormatEx->nBlockAlign - 1 ) /
			pWaveFormatEx->nBlockAlign ) * dwSamplesPerBlock;

		PCM = adpcm_decode_data(pSampleData, pWaveFormatEx->nChannels, dwSamples, pWaveFormatEx->nBlockAlign);

		pWaveFormatEx->wBitsPerSample = 16;

		uiSampleDataSize = dwSamples * 2 * pWaveFormatEx->nChannels;
		pSampleData = PCM;

		if ( !InitSampleFromAddress( hS, pSampleData, uiSampleDataSize, pWaveFormatEx, siPlaybackRate, pFilterData ) )
		{
			delete [] PCM;
			return LTFALSE;
		}
		pSample->m_bAllocatedSoundData = true;
	}
	else
	{
		printf("error in InitSampleFromFile - format tag %i not handled yet!\n", pWaveFormatEx->wFormatTag);
		return LTFALSE;
	}

	return LTTRUE;
}

COpenALSoundSys COpenALSoundSys::m_OpenALSoundSys;
const char* COpenALSoundSys::m_pCOpenALSoundSysDesc = "OpenAL Soft";

const char* copenal_GetSoundDesc()
{
	return COpenALSoundSys::m_pCOpenALSoundSysDesc;
}

ILTSoundSys* copenal_MakeSoundSys()
{
	return &COpenALSoundSys::m_OpenALSoundSys;
}
