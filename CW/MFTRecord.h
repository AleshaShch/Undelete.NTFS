#ifndef MFT_RECORD_H
#define MFT_RECORD_H

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include "NTFSDrive.h"

#define BUFF_LEN 4096

/* ������ ���� �������� ���������, ������� ��������� ������ MFT */

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
} MFTBaseRecord;

/* ��������� �������� */
typedef struct {
	DWORD type;
	DWORD size;
	BYTE nonResFlag;
	BYTE nameLength;
	WORD nameOffset;
	WORD flags;
	WORD ID;
	
	typedef union{

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

	}Attr;

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
	WORD fileName[512]; //?
} FILE_NAME;

typedef struct MFTRecord {
	HANDLE hDrive;
	//BYTE* MFTRecord;
	int recordSize;
	int startPosition;
	int bytesPerClusters;
	int currPosition;

	FILE_NAME fileName;
	BYTE* bufferForStream;

	// methods
} MFTRecord;

int readUSNJournal(NTFSDrive );

#endif