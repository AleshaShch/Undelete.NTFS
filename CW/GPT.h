#ifndef GPT_H
#define GPT_H

#include <conio.h>
#include <windows.h>
#include <stdio.h>

typedef unsigned long long QWORD; 

/* Структура, описывающая записи таблицы разделов жёсткого диска */
typedef struct {
	TCHAR signature[8];
	DWORD revision;
	DWORD headerSz;
	DWORD CRC32_zlibOfHeader;
	DWORD reserved;
	QWORD currLBA;
	QWORD backupLBA;
	QWORD firstLBA;
	QWORD lastLBA;
	UCHAR diskGUID[16];
	QWORD startLBA;
	DWORD numberOfPartition;
	DWORD szPartition;
	DWORD CRC32_zlibOfPartArray;
	UCHAR reserved2;
}PartitionTableHeader;

typedef struct {
	BYTE partType[16]; // GUID of partition type. Шестнадцатеричная форма имени раздела 
	UCHAR uniqueGUID[16];
	QWORD firstLBA;
	QWORD lastLBA;
	QWORD attrFlags;
	UCHAR partName[72];
}PartitionEntrieGPT;

#endif