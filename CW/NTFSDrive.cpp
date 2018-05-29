#include "NTFSDrive.h"
#include "MFTRecord.h"
#include "Constants.h"

NTFSDrive init(NTFSDrive entrie, HANDLE drive, LARGE_INTEGER offset) {
	
	entrie.driveOffset = offset;
	entrie.hDrive = drive;
	entrie.created = 1;

	return entrie;
}

int isNTFS(NTFSDrive &entrie) {
	BYTE sector[SIZE_OF_SECTOR];
	DWORD numOfBytesRead;
	NTFS_BS *bs; 
	BIOS_PARAMETER_BLOCK *biosBlock;
	LARGE_INTEGER bPerCluster, offsetMFT;
	BYTE rec[1024];

	DWORD dwPtr = SetFilePointer(entrie.hDrive, entrie.driveOffset.LowPart, &entrie.driveOffset.HighPart, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) {
		entrie.NTFS = 0;
		printf("Error %d", GetLastError());
		return -1;
	}
	
	ReadFile(entrie.hDrive, sector, sizeof(sector), &numOfBytesRead, 0);

	/* ƒанные в структру BIOS_PARAMETER_BLOCK в таком виде занос€тс€ некорректно. Ћишь некоторый пол€ содержат верную информацию */  
	LARGE_INTEGER *tempSize, *tempMFToffset, *tempVolumeNum;

	bs = (NTFS_BS*)sector; 
	biosBlock = (BIOS_PARAMETER_BLOCK*)(sector + 0x0B);

	/* Ёти действи€ помогают решить проблему, изложенную выше */
	tempSize = (LARGE_INTEGER*)(sector + 0x28);
	tempMFToffset = (LARGE_INTEGER*)(sector + 0x30);
	tempVolumeNum = (LARGE_INTEGER*)(sector + 0x48);
		
	if (memcmp(bs->oem_ID, "NTFS", sizeof("NTFS") - 1) == 0) {
		entrie.NTFS = 1;
		entrie.bPerSector = biosBlock->bPerSector;
		entrie.sectorPerCluster = biosBlock->sectorPerCluster;
		entrie.totalSector = *tempSize;
		entrie.startClusterOfMFT = *tempMFToffset;
		entrie.volumeSerialNum = *tempVolumeNum; 
	}
	
	bPerCluster.QuadPart = biosBlock->bPerSector * biosBlock->sectorPerCluster;
	offsetMFT.QuadPart = entrie.startClusterOfMFT.QuadPart * bPerCluster.QuadPart;
	entrie.volumeOffsetOfMFT = offsetMFT;
	offsetMFT.QuadPart += entrie.driveOffset.QuadPart;

	DWORD dwPtr1 = SetFilePointer(entrie.hDrive, offsetMFT.LowPart, &offsetMFT.HighPart, FILE_BEGIN);
	if (dwPtr1 == INVALID_SET_FILE_POINTER) {
		entrie.NTFS = 0;
		printf("Error %d", GetLastError());
		return -1;
	}
	
	findDriveLetterForVolume(entrie);

	//offsetMFT.QuadPart = volumeInfo.BytesPerCluster * volumeInfo.MftStartLcn.QuadPart;
	//offsetMFT.QuadPart += entrie.driveOffset.QuadPart;

	//DWORD dwPtr1 = SetFilePointer(entrie.hDrive, offsetMFT.LowPart, &offsetMFT.HighPart, FILE_BEGIN);
	//if (dwPtr1 == INVALID_SET_FILE_POINTER) {
	//	entrie.NTFS = 0;
	//	printf("Error %d", GetLastError());
	//	return entrie;
	//}
	//
	//if(!ReadFile(entrie.hDrive, rec, sizeof(rec), &numOfBytesRead, 0)){
	//	printf("Error %d", GetLastError());
	//	//return -1;
	//}

	//MFTBaseRecord *baseRec;
	//baseRec = (MFTBaseRecord *) rec;
	//
	//ReadFile(entrie.hDrive, rec, sizeof(rec), &numOfBytesRead, 0);
	//ReadFile(entrie.hDrive, rec, sizeof(rec), &numOfBytesRead, 0);
	//ReadFile(entrie.hDrive, rec, sizeof(rec), &numOfBytesRead, 0);

	return 0;
}

int findDriveLetterForVolume(NTFSDrive &entrie) {
	int i;
	TCHAR buffForStrings[MAX_PATH], tempPath[MAX_PATH], letter;
	NTFS_VOLUME_DATA_BUFFER	volumeInfo;
	DWORD numOfBytesRead;

	GetLogicalDriveStrings(sizeof(buffForStrings), buffForStrings);

	for (i = 0; ; i += LENGTH_OF_VOLUME_PATH) {
		  
		letter = buffForStrings[i];
		sprintf(tempPath, "\\\\.\\%c:", letter);

		HANDLE hDrive = CreateFile(tempPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,  FILE_FLAG_NO_BUFFERING, 0); 
		if (hDrive == INVALID_HANDLE_VALUE) { 
			/*printf("Error %d", GetLastError());*/ 
			return 1;
		}

		if (!DeviceIoControl(hDrive, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &volumeInfo, sizeof(NTFS_VOLUME_DATA_BUFFER), &numOfBytesRead, 0)) {
			/*printf("Error %d", GetLastError());*/
			return 1;
		}

		if (volumeInfo.VolumeSerialNumber.QuadPart == entrie.volumeSerialNum.QuadPart) entrie.driveLetter = letter; 

	}

	return 0;
}

int getInfoAboutVolume(NTFSDrive entrie) {
	TCHAR buffForStrings[MAX_PATH], tempPath[MAX_PATH];
	NTFS_VOLUME_DATA_BUFFER	volumeInfo;
	DWORD numOfBytesRead;

	sprintf(tempPath, "\\\\.\\%c:", entrie.driveLetter);

	HANDLE hDrive = CreateFile(tempPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,  FILE_FLAG_NO_BUFFERING, 0); 
	if (hDrive == INVALID_HANDLE_VALUE) { 
		/*printf("Error %d", GetLastError());*/ 
		return 1;
	}

	if (!DeviceIoControl(hDrive, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &volumeInfo, sizeof(NTFS_VOLUME_DATA_BUFFER), &numOfBytesRead, 0)) {
		/*printf("Error %d", GetLastError());*/
		return 1;
	}

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		printf("Error %d", GetLastError()); 
		return -1;
	}

	/* typedef в данной ситуации не поможет */
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| lightBlue)); 
	printf("General Information:\n");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));
	
	printf("Bytes Per Cluster - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| lightRed));
	printf("%d B\n", volumeInfo.BytesPerCluster);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("Bytes Per Sector - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| lightRed));
	printf("%d B\n", volumeInfo.BytesPerSector);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));
	
	printf("Unique number for partition - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| lightRed));
	printf("%I64d\n", volumeInfo.VolumeSerialNumber);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| green));
	printf("Information about volume size:\n");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	//----------------------------------------------
	printf("Volume Size - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| yellow));
	printf("%I64d MB\n", (volumeInfo.TotalClusters.QuadPart * volumeInfo.BytesPerCluster)/ M_BYTES);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("Free cluster - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| yellow));
	printf("%I64d \n", volumeInfo.FreeClusters);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("Free space - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| yellow));
	printf("%I64d MB (%I64d %% of drive) \n", (volumeInfo.FreeClusters.QuadPart * volumeInfo.BytesPerCluster) / M_BYTES, (volumeInfo.FreeClusters.QuadPart * PERCENT) / volumeInfo.TotalClusters.QuadPart);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("Number of sectors - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| yellow));
	printf("%I64d\n", volumeInfo.NumberSectors);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("Number of clusters - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| yellow));
	printf("%I64d \n", volumeInfo.TotalClusters);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	//----------------------------------------------
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| magenta));
	printf("MFT Info:\n");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("MFT start cluster - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| brown));
	printf("%I64d \n", volumeInfo.MftStartLcn);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("MFT Mirror start cluster - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| brown));
	printf("%I64d \n",volumeInfo.Mft2StartLcn);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("MFT size - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| brown));
	printf("%I64d MB (%I64d %% of drive) \n", volumeInfo.MftValidDataLength.QuadPart / M_BYTES, (volumeInfo.MftValidDataLength.QuadPart / volumeInfo.BytesPerCluster * PERCENT) / volumeInfo.TotalClusters.QuadPart);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("MFT zone clusters - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| brown));
	printf("%I64d - %I64d \n", volumeInfo.MftZoneStart, volumeInfo.MftZoneEnd);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| black));

	printf("MFT zone size - ");
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| brown));
	printf("%I64d MB (%I64d %% of drive)\n", (volumeInfo.MftZoneEnd.QuadPart - volumeInfo.MftZoneStart.QuadPart) * volumeInfo.BytesPerCluster / M_BYTES, (volumeInfo.MftZoneEnd.QuadPart - volumeInfo.MftZoneStart.QuadPart) / volumeInfo.TotalClusters.QuadPart * PERCENT);
	SetConsoleTextAttribute(hConsole, (WORD)((lightGray << 4)| brown));
	
	return 0;
}
