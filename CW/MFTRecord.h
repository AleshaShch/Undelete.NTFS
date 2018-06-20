#ifndef MFT_RECORD_H
#define MFT_RECORD_H

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <locale.h>
#include "NTFSDrive.h"

#define BUFF_LEN 4096

/* ������ ���� �������� ���������, ������� ��������� ������ MFT */

typedef enum {
	MFT_RECORD_NOT_USED = 1,
	MFT_RECORD_IN_USE = 2,
	MFT_RECORD_IS_DIRECTORY = 3,
}MFT_RECORD_FLAGS;

/* C�������� ������ ������� ������ MFT */
typedef struct {
	TCHAR signatureF[4]; // ��������� "FILE"; �������������� ����
	WORD testingMarkerOffset; // C������� ������� ����������� ��������
	WORD testingMarkerSize;
	LONGLONG logFileNum; // ����� LSN ��� $LogFile; �������������� ����
	WORD serialNum; // �������������� ����
	WORD linksCount; // �������������� ����
	WORD attrOffset; // �������� ������� ��������
	WORD flags;
	DWORD realRecordSize;
	DWORD recordSize;
	LONGLONG baseMFTRec; // ����� ������� ������; �������������� ����
	WORD nextAttrID; // �������������� ����
}MFTBaseRecord;

typedef enum {
	$STANDARD_INFORMATION = 0x10,
	$ATTRIBUTE_LIST = 0x20,
	$FILE_NAME = 0x30,
	/* $VOLUME_VERSION = 0x40 - NT */
	$OBJECT_ID = 0x40,
	$SECURITY_DESCRIPTOR	= 0x50,
	$VOLUME_NAME = 0x60,
	$VOLUME_INFORMATION = 0x70,
	$DATA = 0x80,
	$INDEX_ROOT = 0x90,
	$INDEX_ALLOCATION = 0xA0,
	$BITMAP = 0xB0,
	/* $SYMBOLIC_LINK = 0xC0 - NT */
	$REPARSE_POINT = 0xC0,
	$EA_INFORMATION = 0xD0,
	$EA = 0xE0, 
	$PROPERTY_SET = 0xF0, // - NT
	$LOGGED_UTILITY_STREAM = 0x100,
	$END = -1,
}ATTR_TYPES;

/* ��������� �������� */
typedef struct {
	ATTR_TYPES type;
	DWORD size;
	BYTE nonResFlag;
	BYTE nameLength;
	WORD nameOffset;
	WORD flags;
	WORD ID;
	
	/* ������ ���� �� ������������ � ������� ������ ���������, ������ ����� ����� ���������� */
	/*typedef union{

		typedef struct{
			DWORD size;
			WORD offset;
		}Resident;

		typedef struct{
			LONGLONG initialVCN;
			LONGLONG endVCN;
			WORD datarunOffset;
			WORD compressionSize;
			DWORD notUse;
			LONGLONG attrSize; // �������������� ����
			LONGLONG realAttrSize;
			LONGLONG initializedSize; // �������������� ����
		}NonResident;

	}Attr;*/

}NTFSAttr;

/* ��������� �������� $FILE_NAME */
typedef struct {
	LONGLONG baseParentDir; // �������������� ����
	LONGLONG createTime; // �������������� ����
	LONGLONG modifyFileTime;
	LONGLONG modifyMFTTime;
	LONGLONG accessTime;
	LONGLONG fileSize;
	LONGLONG realFileSize;
	DWORD flags;
	DWORD accessPoint;
	WORD nameLength;
	WORD attrNamespace;
	WORD fileName[MAX_PATH/2];
}FileName;

typedef struct delFileInfoUsn{
	USN usn;
	WCHAR *fileName;
	WORD fileNameLen;
	DWORDLONG parentFileReferenceNumber;
	
	delFileInfoUsn *next;
}delFileInfoUSN;

typedef struct delFileInfoMFT{
	WORD fileName[MAX_PATH];
	WORD fileNameLen;
	DWORDLONG parentFileReferenceNumber;

	delFileInfoMFT *next;
}delFileInfoMFT;

int readUSNJournal(NTFSDrive , delFileInfoUsn **);
int readMFTRecord(NTFSDrive , delFileInfoMFT **);
char* getFullPath(HANDLE , DWORDLONG );

#endif