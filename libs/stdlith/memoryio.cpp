//------------------------------------------------------------------
//
//	FILE	  : MemoryIO.cpp
//
//	PURPOSE	  : Implements the CMemoryIO class.
//
//	CREATED	  : July 25 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include "memoryio.h"



CMemoryIO::CMemoryIO()
{
	m_bRanOutOfMemory = 0;
	m_Pos = 0;
	m_Data.SetCacheSize( 200 );
}



LTBOOL CMemoryIO::Write(const void *pBlock, uint32 blockSize )
{
	uint32		i, initialSize;

	
	initialSize = m_Data.GetSize();
	if( (m_Pos+blockSize) > initialSize )
	{
		for( i=0; i < ((m_Pos+blockSize)-initialSize); i++ )
		{
			if( !m_Data.Append(0) )
			{
				m_bRanOutOfMemory = 1;
				MaybeThrowIOException( MoWriteError );
				return 0;
			}
		}
	}

	memcpy( &(m_Data.GetArray()[m_Pos]), pBlock, blockSize );
	m_Pos += blockSize;
	
	return 1;
}


LTBOOL CMemoryIO::Read( void *pBlock, uint32 blockSize )
{
	if( (m_Pos+blockSize) > m_Data.GetSize() )
	{
		MaybeThrowIOException( MoReadError );
		return 0;
	}

	memcpy( pBlock, &(m_Data.GetArray()[m_Pos]), blockSize );
	m_Pos += blockSize;
	
	return 1;
}


uint32 CMemoryIO::GetCurPos()
{
	return m_Pos;
}

uint32 CMemoryIO::GetLen()
{
	return m_Data.GetSize();
}

LTBOOL CMemoryIO::SeekTo( uint32 pos )
{
	if( pos > m_Data.GetSize() )
	{
		MaybeThrowIOException( MoSeekError );
		return 0;
	}

	m_Pos = pos;
	return 1;
}


void CMemoryIO::Clear()
{
	m_Data.Term();
	m_Pos=0;
	m_bRanOutOfMemory = 0;
}
