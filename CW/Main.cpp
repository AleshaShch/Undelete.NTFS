#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include "MBR.h"
#include "GPT.h"
#include "NTFSDrive.h"
#include "ConstantsAndBasicStructures.h"
#include "MFTRecord.h"

int main(int argc, char **argv) {
	char driveLetter[2];
	TCHAR s;
	TCHAR drive[] = TEXT("a:\\"), GUIDbuff[MAX_PATH];
	int flForRet, i;
	NTFSDrive logicalDrive[NUM_OF_ENTRIES];

	/* Работа с аргументами командной строки */
	if (argc != 2) {
		printf("Incorrect command line arguments");
		return -1;
	}

	if (argv[1][0] >= 'a' && argv[1][0] <= 'z') {
		argv[1][0] -= ' ';
		strcpy(driveLetter, argv[1]);
	}
	else if (argv[1][0] >= 'A' && argv [1][0] <= 'Z') {
		strcpy(driveLetter, argv[1]);
	}
		else {
			printf("Incorrect drive letter");
			return -1;
		}
	
	/* Установка цветов консоли и текста */
	system("color 70");

	/* Вывод GUID каждого тома */
	for (s = TEXT('a'); s < TEXT('z'); s++) {
		drive[0] = s;

		flForRet = GetVolumeNameForVolumeMountPoint(drive, GUIDbuff, sizeof(GUIDbuff));
		if (flForRet)   
			printf("The GUID of drive %s is %s \n", drive, GUIDbuff);	
	}
		
	printf("\n");
	scanPhysicalDrive(logicalDrive);

	printf("\n");
	for (i = 0; logicalDrive[i].driveLetter != driveLetter[0] && logicalDrive[i].NTFS == 1; i++);
	if (logicalDrive[i].NTFS != 1) {
		printf("Incorrect drive letter");
		system("pause");
		return -1;
	}
	else getInfoAboutVolume(logicalDrive[i]);
	
	printf("\n");
	delFileInfoUsn *lstUSN;
	lstUSN = NULL;
	readUSNJournal(logicalDrive[i], &lstUSN);
	
	printf("\n");
	delFileInfoMFT *lstMFT;
	lstMFT = NULL;
	readMFTRecord(logicalDrive[i], &lstMFT);

	system("pause");
	return 0;
}
