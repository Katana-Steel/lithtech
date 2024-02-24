// *********************************************************************** //
//
//	MODULE:		MemoryUtils.cpp
//
//	PURPOSE:	General purpose memory utilities
//				(originally written by Brian Goble)
//
//	HISTORY:	9/12/00 [alm] This file was created
//
//	NOTICE:		Copyright (c) 2000, MONOLITH, Inc.
//
// *********************************************************************** //

#include "stdafx.h"
#include "MemoryUtils.h"
#include "malloc.h"

// *********************************************************************** //

int CheckHeap(LTBOOL bWalkIfErr)
{
	int val = _heapchk();

	OutputDebugString("Checking heap...\n");
	OutputHeapReturnValue(val);

	if(bWalkIfErr && val != _HEAPOK)
	{
		_HEAPINFO	hi;
		memset(&hi, 0, sizeof(_HEAPINFO));

		int val = _heapwalk(&hi);
		int heapstatus;

		OutputDebugString("Walking heap...\n");

		hi._pentry = NULL;
		while((heapstatus = _heapwalk(&hi)) == _HEAPOK)
		{

		}

		char	buf[80];
		sprintf(buf, "HEAP: %6s block at %Fp of size %4.4X\n", (hi._useflag == _USEDENTRY ? "USED" : "FREE"), hi._pentry, hi._size);
		OutputDebugString(buf);

		OutputHeapReturnValue(heapstatus);
		OutputDebugString("Finished walking heap.");
	}

	return(val);
}

// *********************************************************************** //

void OutputHeapReturnValue(int val)
{
	switch (val)
	{
		case _HEAPBADBEGIN:
		{
			::OutputDebugString("Heap return value: _HEAPBADBEGIN\n");
			break;
		}

		case _HEAPBADNODE:
		{
			::OutputDebugString("Heap return value: _HEAPBADNODE\n");
			break;
		}

		case _HEAPBADPTR:
		{
			::OutputDebugString("Heap return value: _HEAPBADPTR\n");
			break;
		}

		case _HEAPEMPTY:
		{
			::OutputDebugString("Heap return value: _HEAPEMPTY\n");
			break;
		}

		case _HEAPOK:
		{
			::OutputDebugString("Heap return value: _HEAPOK\n");
			break;
		}

		default:
		{
			::OutputDebugString("Heap return value: Unknown return value!\n");
			break;
		}
	}
}

// *********************************************************************** //

int HeapStats()
{
	int val = _heapchk();

	OutputDebugString("Getting heap statistics...");
	OutputHeapReturnValue(val);

	uint32	dwTotal = 0;
	uint32	dwUsed  = 0;
	uint32	dwFree  = 0;

	if (val == _HEAPOK)
	{
		_HEAPINFO	hi;
		memset(&hi, 0, sizeof(_HEAPINFO));

		int val = _heapwalk(&hi);
		int heapstatus;

		hi._pentry = NULL;
		while((heapstatus = _heapwalk(&hi)) == _HEAPOK)
		{
			dwTotal += hi._size;

			if (hi._useflag == _USEDENTRY) dwUsed += hi._size;
			else dwFree += hi._size;
		}
	}

	char buf[128];
	sprintf(buf, "Heap stats: Total = %lu, Free = %lu, Used = %lu\n");
	OutputDebugString(buf);

	return(val);
}

