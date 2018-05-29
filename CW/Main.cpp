#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include "MBR.h"
#include "GPT.h"
#include "NTFSDrive.h"

#define SIZE_OF_SECTOR 512
#define NUM_OF_ENTRIES 128 

int main(int argc, char **argv) {
	char driveLetter[2];
	TCHAR s;
	TCHAR drive[] = TEXT("a:\\"), GUIDbuff[MAX_PATH];
	int flForRet, i;
	NTFSDrive logicalDrive[NUM_OF_ENTRIES];

	/* –абота с аргументами командной строки */
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
	
	/* ¬ывод GUID каждого тома */
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
		return -1;
	}
	else getInfoAboutVolume(logicalDrive[i]);

	//HANDLE hDrive = CreateFile("\\\\.\\D:", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	//if (hDrive == INVALID_HANDLE_VALUE) { 
		//	printf("Error %d", GetLastError());
			//return -1;
		//}
	//ReadFile(hDrive, sector, SIZE_OF_SECTOR, &numOfBytesRead, 0);

	return 0;
}


	








	/* ¬ документации Windows говоритс€, что дл€ файлов метаданных файловой системы отводитс€ только первые 16 записей, но на практике выделение записей пользовател€м
	начинаетс€ с записи 24. */