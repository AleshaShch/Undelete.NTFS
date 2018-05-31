#ifndef MFT_RECORD_H
#define MFT_RECORD_H

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include "NTFSDrive.h"

#define BUFF_LEN 4096

/* Данный файл содержит структуры, которые описывают запись MFT */

/* Cтруктуру данных базовой записи MFT */
typedef struct {
	TCHAR signatureF[4]; // Сигнатура "FILE"; необязательное поле
	WORD testingMarkerOffset; // Cмещение массива проверочных маркеров
	WORD testingMarkerSize;
	LONGLONG logFileNum; // Номер LSN для $LogFile; необязательное поле
	WORD serialNum; // Необязательное поле
	WORD linksCount; // Необязательное поле
	WORD attrOffset; // Смещение первого атрибута
	WORD flags;
	DWORD realRecordSize;
	DWORD recordSize;
	LONGLONG baseMFTRec; // Адрес базовой записи; необязательное поле
	WORD nextAttrID; // Необязательное поле
} MFTBaseRecord;

/* Структура атрибута */
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
			LONGLONG attrSize; // Необязательное поле
			LONGLONG realAttrSize;
			LONGLONG initializedSize; // Необязательное поле
		}NonResident;

	}Attr;

}NTFSAttr;

/* Структура атрибута $FILE_NAME */
typedef struct {
	LONGLONG baseParentDir; // Необязательное поле
	LONGLONG createTime; // Необязательное поле
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