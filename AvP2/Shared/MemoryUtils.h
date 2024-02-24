// *********************************************************************** //
//
//	MODULE:		MemoryUtils.h
//
//	PURPOSE:	General purpose memory utilities
//				(originally written by Brian Goble)
//
//	HISTORY:	9/12/00 [alm] This file was created
//
//	NOTICE:		Copyright (c) 2000, MONOLITH, Inc.
//
// *********************************************************************** //

#ifndef __MEMORY_UTILS_H__
#define __MEMORY_UTILS_H__


int 	CheckHeap(LTBOOL bWalkIfErr = LTFALSE);
void	OutputHeapReturnValue(int val);
int		HeapStats();


#endif

