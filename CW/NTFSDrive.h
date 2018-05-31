#ifndef NTFS_DRIVE_H
#define NTFS_DRIVE_H

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "ConstantsAndBasicStructures.h"

#define SIZE_OF_SECTOR 512
#define LENGTH_OF_VOLUME_PATH 4

typedef struct NTFSDrive {
	HANDLE hDrive;
	TCHAR driveLetter;
	LARGE_INTEGER driveOffset;
	int created;
	int NTFS;

	WORD bPerSector;
	BYTE sectorPerCluster;

	LARGE_INTEGER volumeOffsetOfMFT;
	LARGE_INTEGER totalSector;
	LARGE_INTEGER startClusterOfMFT;
	LARGE_INTEGER volumeSerialNum;
	
}NTFSDrive;

typedef struct BIOS_PARAMETER_BLOCK {
	WORD bPerSector;
	BYTE sectorPerCluster;
	WORD reserverS;
	BYTE always0_1[3];
	WORD notUsedByNTFS1;
	BYTE mediaDescr;
	WORD always0_2;
	WORD sPerTrack;
	WORD numOfHeads;
	DWORD hiddenS;
	DWORD notUsedByNTFS2;
	/* Расширенная часть структуры */
	DWORD notUsedByNTFS3;
	LARGE_INTEGER totalSector;
	LARGE_INTEGER logicalClusterNumFor$MFT;
	LARGE_INTEGER logicalClusterNumFor$MFTMirr;
	TCHAR clusterPerFileRecSegment; // Может быть негативным. Тогда это степень 2.
	BYTE notUsedByNTFS4[3];
	TCHAR cPerIndexBuffer; // Может быть негативным. Тогда это степень 2.
	BYTE notUsedByNTFS5[3];
	LARGE_INTEGER volumeSerialNum;
	DWORD checkSum;
}BIOS_PARAMETER_BLOCK;

/* Структура описывает загрузочный сектор тома, использующего файловую систему NTFS */
typedef struct NTFS_BS {
	TCHAR jumpInstr[3];
	TCHAR oem_ID[8];
	BIOS_PARAMETER_BLOCK bpb;
	BYTE bootstrapCode[426];
	WORD endOfSectorMarker;
}NTFS_BS;

NTFSDrive init(NTFSDrive entrie, HANDLE drive, LARGE_INTEGER offset);
int isNTFS(NTFSDrive &entrie);
int findDriveLetterForVolume(NTFSDrive &);
int getInfoAboutVolume(NTFSDrive );

#endif