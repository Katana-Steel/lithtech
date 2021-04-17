#include "StdAfx.h"
#include <stdio.h>
#include "WinUtil.h"
#include <sys/stat.h>
#ifndef __LINUX
#include <time.h>
#include <direct.h>
#include <IO.h>
#else
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <csignal>
#endif
#include "ltbasedefs.h"

BOOL CWinUtil::GetMoviesPath (char* strPath)
{
	if (strPath == nullptr)
		return FALSE;
    // never derefence a raw pointer before checking if it's valid
	strPath[0] = '\0';

	char strTemp[256];
	if (_getcwd (strTemp, 255))
	{
		char strFile[270];
		if (strTemp[strlen(strTemp) - 1] != '\\') strcat (strTemp, "\\");
		SAFE_STRCPY(strFile, strTemp);
		strcat (strFile, "intro.smk");

		if (FileExist (strFile))
		{
			SAFE_STRCPY(strPath, strTemp);
			return TRUE;
		}
	}
#ifndef __LINUX
	char chDrive   = 'A';

	while (chDrive <= 'Z')
	{
		sprintf(strPath, "%c:\\", chDrive);

		if (GetDriveType(strPath) == DRIVE_CDROM)
		{
			strcat(strPath, "Movies\\");
			if (DirExist (strPath)) return TRUE;
		}

		chDrive++;
	}

	strPath[0] = '\0';

#endif
	return FALSE;
}

BOOL CWinUtil::DirExist (char const* strPath)
{
	if (!strPath || !*strPath) return FALSE;

	BOOL bDirExists = FALSE;
	char szPath[MAX_PATH];
	SAFE_STRCPY( szPath, strPath );
    char last = szPath[strlen(szPath) - 1];
	if (last == '\\' || last == '/')
	{
		szPath[strlen(szPath) - 1] = '\0';
	}

#ifndef __LINUX
	UINT oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	struct _stat64 statbuf;
	int error = _stat64(szPath, &statbuf);
	SetErrorMode (oldErrorMode);
	if (error != -1) bDirExists = TRUE;
#else
	struct stat64 statbuf;
	int error = stat64(szPath, &statbuf);
	if (error != -1 && S_ISDIR(statbuf.st_mode)) bDirExists = TRUE;
#endif

	return bDirExists;
}

BOOL CWinUtil::CreateDir (char const* strPath)
{
	if (DirExist (strPath)) return TRUE;
	if (strPath[strlen(strPath) - 1] == ':') return FALSE;		// special case

	char strPartialPath[MAX_PATH];
	strPartialPath[0] = '\0';

	// Make a copy of the input since we're going to change it.
	char szPath[MAX_PATH];
	SAFE_STRCPY( szPath, strPath );

#ifndef __LINUX
	const char *path_sep = "\\";
#else
	const char *path_sep = "/";
#endif

	char* token = strtok (szPath, "\\/");
	while (token)
	{
		strcat (strPartialPath, token);
		if (!DirExist (strPartialPath) && strPartialPath[strlen(strPartialPath) - 1] != ':')
		{
#ifdef __LINUX
			if (!mkdir(strPartialPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) return FALSE;
#else
			if (!CreateDirectory(strPartialPath, NULL)) return FALSE;
#endif
		}
		strcat (strPartialPath, path_sep);
		token = strtok (NULL, "\\/");
	}

	return TRUE;
}

BOOL CWinUtil::FileExist (char const* strPath)
{
#if defined(_WIN32)
	OFSTRUCT ofs;
	HFILE hFile = OpenFile(strPath, &ofs, OF_EXIST);
	if (hFile == HFILE_ERROR) return FALSE;

	return TRUE;
#elif defined (__LINUX)
	BOOL bFileExist = FALSE;

	struct stat64 statbuf;
	int error = stat64(strPath, &statbuf);

	if (error != -1 && S_ISREG(statbuf.st_mode)) bFileExist = TRUE;

	return bFileExist;
#else
#error Please define your platform.
#endif
}

BOOL CWinUtil::CopyDir( char const* pSrc, char const* pDest )
{
	if( !pSrc || !pDest ) return FALSE;

	if( !DirExist( pSrc )) return FALSE;

	// CreateDir does not preserve the path...
	char szDir[MAX_PATH] = {0};
	strcpy( szDir, pDest );

	if( !DirExist( szDir ))
		CreateDir( szDir );

	char szDestFile[MAX_PATH] = {0};
	char szFiles[MAX_PATH] = {0};
	sprintf(szFiles, "%s\\*.*", pSrc);
#ifndef __LINUX
	struct _finddata_t file;
	intptr_t hFile;
	StringSet fileList;

	// find first file
	if((hFile = _findfirst(szFiles, &file)) != -1L)
	{
		do
		{
			sprintf( szFiles, "%s\\%s", pSrc, file.name );
			if( FileExist( szFiles ))
				fileList.insert(file.name);
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);

	StringSet::iterator iter = fileList.begin();
	while (iter != fileList.end())
	{
		sprintf(szFiles,"%s\\%s",pSrc,iter->c_str());
		sprintf(szDestFile, "%s\\%s", pDest, iter->c_str());
		
		if( !CopyFile( szFiles, szDestFile, FALSE ))
			return FALSE;

		iter++;
	}
#endif
	
	return TRUE;
}

BOOL CWinUtil::EmptyDir( char const* pDir )
{
	if( !pDir ) return FALSE;

	if( !DirExist( pDir )) return FALSE;

	char szDelFile[MAX_PATH] = {0};
	char szFiles[MAX_PATH] = {0};
	sprintf( szFiles, "%s\\*.*", pDir);
#ifndef __LINUX
	struct _finddata_t file;
	intptr_t hFile;
	StringSet fileList;

	// find first file...
	if( (hFile = _findfirst( szFiles, &file )) != -1L)
	{
		do
		{
			sprintf( szFiles, "%s\\%s", pDir, file.name );
			fileList.insert( szFiles );
		}
		while (_findnext( hFile, &file ) == 0 );
	}
	_findclose( hFile );

	// Remove each file...

	StringSet::iterator iter = fileList.begin();
	while( iter != fileList.end() )
	{
		remove( iter->c_str() );
		iter++;
	}
#endif
	return TRUE;
}

BOOL CWinUtil::RemoveDir( char const* pDir )
{
	if( !pDir ) return FALSE;

	// Make sure it exists and is empty...

	if( !DirExist( pDir )) return FALSE;
	if( !EmptyDir( pDir )) return FALSE;

	// Ok, delete it...
	_rmdir( pDir );

	return TRUE;
}

DWORD CWinUtil::WinGetPrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, DWORD nSize, const char* lpFileName)
{
#ifndef __LINUX
	return GetPrivateProfileString (lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
#else
	std::string lookup{lpKeyName};
	std::string line;
	std::ifstream conf{lpFileName};
	if (!conf)
	{
		return 2;
	}
	while(!conf.eof()) {
		conf >> line;
		if(line.substr(0, lookup.size()) ==  lookup) {
			auto b = line.find("=") + 1;
			lookup = line.substr(b);
			if(lookup.length() < nSize) {
				strcpy(lpReturnedString, lookup.c_str());
				return 0;
			} else {
				return 2;
			}
		}
	}
	strcpy(lpReturnedString, lpDefault);
	return 1;
#endif
}

DWORD CWinUtil::WinWritePrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpString, const char* lpFileName)
{
#ifndef __LINUX
	return WritePrivateProfileString (lpAppName, lpKeyName, lpString, lpFileName);
#else
    // format app.key=str
	if (lpAppName == NULL || lpKeyName == NULL || lpString == NULL)
	{
		return 1;
	}
	std::string lookup{lpAppName};
	lookup += ".";
	lookup += lpKeyName;
	std::string line;
	std::fstream conf{lpFileName, std::ios_base::app|std::ios_base::in|std::ios_base::out};
	while(!conf.eof()) {
		conf >> line;
		if(line.substr(0, lookup.size()) ==  lookup) {
		    conf.seekp(conf.tellg());
		    conf.seekp(-line.size(), std::ios_base::cur);
			break;
		}
	}
	conf << lookup << "=" << lpString << '\n';
	conf.flush();
	return 0;
#endif
}

void CWinUtil::DebugOut (char const* str)
{
#ifndef __LINUX
	OutputDebugString (str);
#else
	std::cerr << str << '\n';
#endif
}

void CWinUtil::DebugBreak()
{
#ifndef __LINUX
	::DebugBreak();
#else
	std::raise(SIGTRAP);
#endif
}

float CWinUtil::GetTime()
{
	return (float)SDL_GetTicks() / 1000.0f;
}

char* CWinUtil::GetFocusWindow()
{
	static char strText[128];
#ifndef __LINUX
	HWND hWnd = GetFocus();
	if (!hWnd)
	{
		hWnd = GetForegroundWindow();
		if (!hWnd) return NULL;
	}

	GetWindowText (hWnd, strText, 127);
#else
	strText[0] = '\0';
#endif
	return strText;
}

void CWinUtil::WriteToDebugFile (char const* strText)
{
#ifndef __LINUX
	FILE* pFile = fopen ("c:\\shodebug.txt", "a+t");
	if (!pFile) return;

	time_t seconds;
	time (&seconds);
	struct tm* timedate = localtime (&seconds);
	if (!timedate) return;

	char strTimeDate[128];
	sprintf (strTimeDate, "[%02d/%02d/%02d %02d:%02d:%02d]  ", timedate->tm_mon + 1, timedate->tm_mday, (timedate->tm_year + 1900) % 100, timedate->tm_hour, timedate->tm_min, timedate->tm_sec);
	fwrite (strTimeDate, strlen(strTimeDate), 1, pFile);

	fwrite (strText, strlen(strText), 1, pFile);
	fwrite ("\n", 1, 1, pFile);

	fclose (pFile);
#else
	std::ofstream file{"~/shodebug.log", std::ios_base::app};
	auto timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	file << '[' << std::put_time(std::localtime(&timestamp), "%F %T") << "]  ";
    file << strText << '\n';
#endif
}

#ifdef __LINUX
void getLevelName(const std::string& path, char* worldname)
{
  const std::string& szWorldTitle = split(split(path,'/').back(), '.').front();
  strcpy(worldname, szWorldTitle.c_str());	
}
#endif