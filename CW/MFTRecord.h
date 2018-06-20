#ifndef MFT_RECORD_H
#define MFT_RECORD_H

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <locale.h>
#include "NTFSDrive.h"

#define BUFF_LEN 4096

/* Данный файл содержит структуры, которые описывают запись MFT */

typedef enum {
	MFT_RECORD_NOT_USED = 1,
	MFT_RECORD_IN_USE = 2,
	MFT_RECORD_IS_DIRECTORY = 3,
}MFT_RECORD_FLAGS;

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

/* Структура атрибута */
typedef struct {
	ATTR_TYPES type;
	DWORD size;
	BYTE nonResFlag;
	BYTE nameLength;
	WORD nameOffset;
	WORD flags;
	WORD ID;
	
	/* Данные поля не используются в текущей версии программы, однако далее будут необходимы */
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
			LONGLONG attrSize; // Необязательное поле
			LONGLONG realAttrSize;
			LONGLONG initializedSize; // Необязательное поле
		}NonResident;

	}Attr;*/

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