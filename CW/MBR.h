#ifndef MBR_H
#define MBR_H

#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include "NTFSDrive.h"
#include "GPT.h"

/* Описание типов разделов жёсткого диска */ 
#define EMPTY_PARTITON 0x00
#define FAT12 0x01
#define XENIX_ROOT 0x02
#define XENIX_USR 0x03
#define FAT16 0x04
#define EXTENDED_PARTITION_WITH_CHS 0x05
#define FAT16B 0x06
#define NTFS_OR_IFS_OR_HPPS_OR_exFAT 0x07
#define FAT32_WITH_CHS 0x0B
#define FAT32_WITH_LBA 0x0C
#define FAT16B_WITH_LBA 0x0E
#define EXTENDED_PARTITION_WITH_LBA	0x0F  

/* typedef enum {
	EMPTY_PARTITON  = 0x00,
	FAT12 = 0x01,
	XENIX_ROOT = 0x02,
	XENIX_USR = 0x03,
	FAT16 = 0x04,
	EXTENDED_PARTITION_WITH_CHS = 0x05,
	FAT16B = 0x06,
	NTFS_OR_IFS_OR_HPPS_OR_exFAT = 0x07,
	FAT32_WITH_CHS = 0x0B,
	FAT32_WITH_LBA = 0x0C,
	FAT16B_WITH_LBA = 0x0E,
	EXTENDED_PARTITION_WITH_LBA	= 0x0F
}PARTITION_ID; */

/* Структура, которая описывает раздел жёсткого диска */
typedef struct {
	BYTE bootInd;
	BYTE firstHead;
	BYTE firstSector;
	BYTE firstCylinder;
	BYTE partitionID;
	BYTE lastHead;
	BYTE lastSector;
	BYTE lastCylinder;
	DWORD absoluteNumberOfSector;
	DWORD numberOfSectors;
}PartitionMBR;

int scanPhysicalDrive(NTFSDrive *);

#endif