// *********************************************************************** //
//
//	MODULE:		UTILS (.CPP)
//
//	PURPOSE:	Gernal purpose utilities
//
//	HISTORY:	02/15/95 [blg] This file was created
//				08/20/01 [mas] Updated to support Windows 2000
//
//	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
//
// *********************************************************************** //


// Includes...

#include <windows.h>
#include "stdafx.h"
#include "utils.h"
#include "malloc.h"
#include "tlhelp32.h"
#include "sys\stat.h"


// Typedefs...

typedef BOOL	(WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef HANDLE	(WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL	(WINAPI *MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

// Prototypes

BOOL	GetProcessModule(DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32, DWORD cbMe32);

// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExistProcess
//
//	PURPOSE:	Determines if the given EXE (or full path, with EXE)
//				is a currently active process.
//
// ----------------------------------------------------------------------- //

BOOL ExistProcess(const char* sExe, int thresh, HANDLE* phProcess)
{
	OSVERSIONINFO	osVer;
	HINSTANCE		hInstLib;
	PROCESSENTRY32	pe32 = {0};
	LPDWORD			lpdwPIDs;
	DWORD			dwSize, dwSize2, dwIndex;
	HMODULE			hMod;
	HANDLE			hProcess;
	char			szFileName[ MAX_PATH ];
	int				count = 0;

	// Sanity checks..

	ASSERT(sExe);
	if (sExe == NULL) return(FALSE);
	if (sExe[0] == '\0') return(FALSE);

	if (phProcess) *phProcess = NULL;


	// Determine if we were given a full path or just an exe...

	BOOL bFullPath = FALSE;

	if (strstr(sExe, "\\"))
	{
		bFullPath = TRUE;
	}

	// Toolhelp32 Pointers (Windows 9x)
	CREATESNAPSHOT	pCreateToolhelp32Snapshot = NULL;
	PROCESSWALK		pProcess32First = NULL;
	PROCESSWALK		pProcess32Next  = NULL;

	// PSAPI Function Pointers (Windows 2000)
	BOOL (WINAPI *pEnumProcesses)(DWORD *, DWORD cb, DWORD *);
	BOOL (WINAPI *pEnumProcessModules)(HANDLE, HMODULE *, DWORD, LPDWORD);
	DWORD (WINAPI *pGetModuleFileNameEx)(HANDLE, HMODULE, LPTSTR, DWORD);
	DWORD (WINAPI *pGetModuleBaseName)(HANDLE, HMODULE, LPTSTR, DWORD);

////////////////////////////////////////////////////////////////
// Windows 2k stuff
////////////////////////////////////////////////////////////////

	// Check to see if were running under Windows 9x or NT
	osVer.dwOSVersionInfoSize = sizeof(osVer);
	GetVersionEx(&osVer);
	
	// If Windows NT
	if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		// Load library and get the procedures explicitly. We do this so that we 
		// don't have to worry about modules using this code failing to load under 
		// Windows 9x, because it can't resolve references to the PSAPI.DLL.
		hInstLib = LoadLibraryA("PSAPI.DLL");
		if (hInstLib == NULL)
            return FALSE;

		// Get procedure addresses.
		pEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))
			GetProcAddress( hInstLib, "EnumProcesses" );
		pEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD))
			GetProcAddress( hInstLib, "EnumProcessModules" );
		pGetModuleFileNameEx =(DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD )) 
			GetProcAddress( hInstLib, "GetModuleFileNameExA" );
		pGetModuleBaseName =(DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD )) 
			GetProcAddress( hInstLib, "GetModuleBaseNameA" );

		if (pEnumProcesses == NULL ||
            pEnumProcessModules == NULL ||
            pGetModuleFileNameEx == NULL ||
			pGetModuleBaseName == NULL )
		{
			FreeLibrary(hInstLib);
			return FALSE;
		}
		
		// Call the PSAPI function EnumProcesses to get all of the
		// ProcID's currently in the system.
		dwSize2 = 256 * sizeof(DWORD);
		lpdwPIDs = NULL;
		do
		{
            if(lpdwPIDs)
            {
				HeapFree(GetProcessHeap(), 0, lpdwPIDs);
				dwSize2 *= 2;
            }

            lpdwPIDs = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, dwSize2);
            if (lpdwPIDs == NULL)
            {
				FreeLibrary(hInstLib);
				return FALSE;
            }

            if (!pEnumProcesses(lpdwPIDs, dwSize2, &dwSize))
            {
				HeapFree(GetProcessHeap(), 0, lpdwPIDs);
				FreeLibrary(hInstLib);
				return FALSE;
            }

		} while (dwSize == dwSize2);
		
		// How many ProcID's did we get?
		dwSize /= sizeof(DWORD);

		// Loop through each ProcID.
		for (dwIndex = 0; dwIndex < dwSize; dwIndex++)
		{
            szFileName[0] = 0;
            // Open the process (if we can... security does not
            // permit every process in the system).
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, lpdwPIDs[ dwIndex ]);

			if (hProcess != NULL)
            {
				// Here we call EnumProcessModules to get only the first module in 
				// the process this is important, because this will be the .EXE module 
				// for which we will retrieve the full path name in a second.
				if (pEnumProcessModules(hProcess, &hMod,
					sizeof(hMod), &dwSize2))
				{
					// Get Full pathname:
					if (bFullPath)
					{
						if (!pGetModuleFileNameEx(hProcess, hMod,
							szFileName, sizeof(szFileName)))
						{
							szFileName[0] = 0;
						}
					}
					else
					{
						if (!pGetModuleBaseName(hProcess, hMod, 
							szFileName, sizeof(szFileName)))
						{
							szFileName[0] = 0;
						}
					}

					if (stricmp(szFileName, sExe) == 0)
					{
						count++;
						if (count == 1 && phProcess)
						{
							*phProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, lpdwPIDs[dwIndex]);
						}

						if (count >= thresh)
						{
				            FreeLibrary(hInstLib);
							CloseHandle(hProcess);
							return TRUE;
						}
					}
				}
				CloseHandle(hProcess);
            }
		
		}

		HeapFree(GetProcessHeap(), 0, lpdwPIDs);
		FreeLibrary(hInstLib);
		return FALSE;
	}

////////////////////////////////////////////////////////////////
// Windows 9x section
////////////////////////////////////////////////////////////////
	else if( osVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
		hInstLib = LoadLibraryA("KERNEL32.DLL");
		if (hInstLib == NULL)
            return FALSE;
		
		// Get procedure addresses.
		// We are linking to these functions of Kernel32 explicitly, because otherwise 
		// a module using this code would fail to load under Windows 2000, which does 
		// not have the Toolhelp32 functions in the Kernel 32.

		pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hInstLib, "CreateToolhelp32Snapshot");
		pProcess32First = (PROCESSWALK)GetProcAddress(hInstLib, "Process32First");
		pProcess32Next = (PROCESSWALK)GetProcAddress(hInstLib, "Process32Next");

		if (pProcess32Next == NULL ||
            pProcess32First == NULL ||
            pCreateToolhelp32Snapshot == NULL)
		{
            FreeLibrary(hInstLib);
            return FALSE;
		}
		
		// Get a handle to a Toolhelp snapshot of the systems
		// processes.
		hProcess = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (hProcess == INVALID_HANDLE_VALUE)
		{
            FreeLibrary(hInstLib);
            return FALSE;
		}
		
		// Get the first process' information.
		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32);
		
		if (pProcess32First(hProcess, &pe32))
		{
			MODULEENTRY32 me32 = {0};
			do 
			{
				if (pe32.th32ProcessID == 0) 
					continue;
				
				if (GetProcessModule(pe32.th32ProcessID, pe32.th32ModuleID, &me32, sizeof(MODULEENTRY32)))
				{
					if (bFullPath)
					{
						if (stricmp(me32.szExePath, sExe) == 0)
						{
							count++;
							if (count == 1 && phProcess)
							{
								*phProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, me32.th32ProcessID);
							}
							
							if (count >= thresh)
							{
					            FreeLibrary(hInstLib);
								return TRUE;
							}
						}
					}
					else
					{
						if (stricmp(me32.szModule, sExe) == 0)
						{
							count++;
							if (count == 1 && phProcess)
							{
								*phProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, me32.th32ProcessID);
							}
							
							if (count >= thresh)
							{
					            FreeLibrary(hInstLib);
								return TRUE;
							}
						}
					}
				}
			}
			
			while (pProcess32Next(hProcess, &pe32));
		}

	}
	else
		return FALSE;
	
	// Free the library.
	FreeLibrary(hInstLib);
	CloseHandle(hProcess);

	return FALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetProcessModule
//
//	PURPOSE:	Given a Process ID and module ID, return its module info
//
// ----------------------------------------------------------------------- //

BOOL GetProcessModule(DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32, DWORD cbMe32)
{   
    BOOL          bRet        = FALSE;
    BOOL          bFound      = FALSE;
    HANDLE        hModuleSnap = NULL;
    MODULEENTRY32 me32        = {0};

    // Obtain a module handle to KERNEL so that we can get the addresses of 
    // the 32-bit Toolhelp functions we need.

    HMODULE hKernel = GetModuleHandle("KERNEL32.DLL");
	if (!hKernel) return(FALSE);

	CREATESNAPSHOT	pCreateToolhelp32Snapshot = NULL;
	MODULEWALK		pModule32First = NULL;
	MODULEWALK		pModule32Next = NULL;

	pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
	if (!pCreateToolhelp32Snapshot) return(FALSE);

	pModule32First = (MODULEWALK)GetProcAddress(hKernel, "Module32First");
	if (!pModule32First) return(FALSE);

	pModule32Next = (MODULEWALK)GetProcAddress(hKernel, "Module32Next");
	if (!pModule32Next) return(FALSE);


    // Take a snapshot of all modules in the specified process...

    hModuleSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == (HANDLE)-1) return(FALSE);


    // Size of the MODULEENTRY32 structure must be initialized before use!

    me32.dwSize = sizeof(MODULEENTRY32);


    // Walk the module list of the process and find the module we are 
    // interested in.  Then, copy the information to the buffer pointed to
    // by lpMe32 so that we can return it to the caller.

    if (pModule32First(hModuleSnap, &me32))
    {
        do 
        {
            if (me32.th32ModuleID == dwModuleID)
            {
                CopyMemory (lpMe32, &me32, cbMe32);
                bFound = TRUE;
            }
        }
        while (!bFound && pModule32Next(hModuleSnap, &me32));

        bRet = bFound;   // If this sets bRet to FALSE, then dwModuleID 
                         // no longer exsists in the specified process.
    }
    else
        bRet = FALSE;           // Couldn't walk module list.


    // Don't forget to clean up the snapshot object...
    CloseHandle (hModuleSnap);

    return (bRet);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDllVersion
//
//	PURPOSE:	Safely calls a DLL's DllGetVersion function
//
// ----------------------------------------------------------------------- //
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;
	
    hinstDll = LoadLibrary(lpszDllName);
		
	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		
		pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");
		
		/*	Because some DLLs may not implement this function, you
			must test for it explicitly. Depending on the particular 
			DLL, the lack of a DllGetVersion function may
			be a useful indicator of the version.
		*/
		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;
			
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			
			hr = (*pDllGetVersion)(&dvi);
			
			if(SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}
		
		FreeLibrary(hinstDll);
	}
	return dwVersion;
}

BOOL DirExist( char const* strPath )
{
	if (!strPath || !*strPath) return FALSE;

	BOOL bDirExists = FALSE;

	char szPath[MAX_PATH] = {0};
	strcpy( szPath, strPath );

	if (szPath[strlen(szPath) - 1] == '\\')
	{
		szPath[strlen(szPath) - 1] = '\0';
	}

	UINT oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	struct stat statbuf;
	int error = stat (szPath, &statbuf);
	SetErrorMode (oldErrorMode);
	if (error != -1) bDirExists = TRUE;

	return bDirExists;
}