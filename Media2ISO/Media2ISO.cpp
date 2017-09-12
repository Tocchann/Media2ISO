// Media2ISO.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include <locale.h>

#include <ShlObj.h>
#include <Shlwapi.h>

#pragma comment(lib,"shlwapi.lib")

#include <atlfile.h>
#include <atlpath.h>
#include <atlalloc.h>


int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR	szLocale[MAX_PATH];
	GetLocaleInfo( GetThreadLocale(), LOCALE_SABBREVLANGNAME, szLocale, MAX_PATH );
	_tsetlocale( LC_ALL, szLocale );

	if( argc < 3 ){
		std::wcout << L"Madia2ISO <�h���C�u���^�[> <iso�t�@�C���o�͐�t�H���_>" << std::endl;
		return 0;
	}
	TCHAR	outputDir[MAX_PATH+1];
	if( GetFullPathName( argv[2], MAX_PATH, outputDir, nullptr ) ){
		SHCreateDirectory( nullptr, outputDir );
		if( !PathIsDirectory( outputDir ) ){
			std::wcout << outputDir << std::endl;
			std::wcout << L"�o�͐�t�H���_������������܂���B" << std::endl;
		}
	}
	else{
		auto result = GetLastError();
		std::wcout << L"GetFullPathName(" << argv[2] << L") �̎��s:" << result << std::endl;
		return result;
	}


	TCHAR	chDrv = argv[1][0];
	CString	rootDir;
	rootDir.Format( _T("\\\\.\\%c:"), chDrv );
	CAtlFile	file;
	HRESULT	hRes = file.Create( rootDir, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING );
	if( SUCCEEDED( hRes ) ){
		//
		//	�{�����[�����x�����x�[�X�ɂ��āAISO�t�@�C�������쐬����
		//
		wchar_t	volumeLabel[MAX_PATH+1];
		CPath	isoFilePath;
		DWORD	serialNo;
		if( GetVolumeInformationByHandleW( file, volumeLabel, MAX_PATH, &serialNo, nullptr, nullptr, nullptr, 0 ) ){
			if( lstrlen( volumeLabel ) == 0 ){
				wsprintf( volumeLabel, _T("%08X"), serialNo );
			}
		}
		else{
			wsprintf( volumeLabel, _T("Drive_%c"), chDrv );
		}
		int		retry = 0;
		do{
			isoFilePath.Combine( outputDir, volumeLabel );
			if( retry != 0 ){
				CString	add;
				add.Format( _T("(%d).iso"), retry );
				isoFilePath.AddExtension( add );
			}
			else{
				isoFilePath.AddExtension( _T(".iso") );
			}
			if( !isoFilePath.FileExists() ){
				break;
			}
			++retry;
		}while( retry < 9999 );
		if( isoFilePath.FileExists() ){
			std::wcout << L"����ȏ�t�@�C����p�ӂł��܂���B" << std::endl;
			std::wcout << (LPCWSTR)isoFilePath << std::endl;
			return ERROR_MORE_DATA;
		}
		std::wcout << chDrv << L": (" << volumeLabel << L") ����" << std::endl << (LPCWSTR)isoFilePath << std::endl << L"�ɓ]����..." << std::endl;
		CAtlFile	fileDst;
		hRes = fileDst.Create( isoFilePath, GENERIC_ALL, 0, CREATE_NEW );
		if( SUCCEEDED( hRes ) ){
			UINT64	size = 0;
			DWORD	readed;
			const int bufSize = 1024*1024;
			CHeapPtr<BYTE>	value;
			if( value.Allocate( bufSize ) )
			{
				while( SUCCEEDED( hRes ) )
				{
					if( SUCCEEDED( (hRes=file.Read( value, bufSize, readed )) ) )
					{
						if( readed > 0 )
						{
							size += readed;
							hRes = fileDst.Write( value, readed );
							//	�������T�C�Y��傫���m�ۂ���悤�ɕύX�����̂ŁA����\��
							wchar_t	fmtSize[MAX_PATH];
							StrFormatByteSize64( size, fmtSize, MAX_PATH );
							std::wcout << fmtSize << L" ����" << std::endl;
						}
						else
						{
							break;
						}
					}
				}
			}
		}
	}
	std::wcout << L"�I�����܂����B(" << hRes << L")" << std::endl;
	return hRes;
}

