#include "MBR.h"

int scanPhysicalDrive(NTFSDrive *logicalDrive) {
	char driveName[MAX_PATH];
	int i, j, k, l, count, n = 0, flForOutput = 1;
	int flForLBA = 0;
	BYTE sector[SIZE_OF_SECTOR];
	int numOfPartition;
	DWORD numOfBytesRead;
	LARGE_INTEGER tempOffset;
	char tempName[MAX_PATH];
	PartitionMBR *partitionRecMBR;
	PartitionTableHeader *partitionTblHeader;
	PartitionEntrieGPT* partEntrGPT;

	tempOffset.QuadPart = 0;

	for (i = 0; ; i++) {
		
		sprintf(driveName, "\\\\.\\PhysicalDrive%d", i);
		count = 0;

		HANDLE hDrive = CreateFile(driveName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0/*FILE_FLAG_NO_BUFFERING*//*FILE_ATTRIBUTE_NORMAL*/, 0); 
		if (hDrive == INVALID_HANDLE_VALUE) { 
			/* printf("Error %d", GetLastError()); */
			return 1;
		}
		
		/* ѕроисходит чтение нулевого сектора жЄсткого диска */
		if(!ReadFile(hDrive, sector, SIZE_OF_SECTOR, &numOfBytesRead, 0)) {
			printf("Error %d", GetLastError());
			return -1;
		}
	
		/* 0x1BE - смещение первой записи таблицы разделов относительно начала главной загрузочной записи */
		partitionRecMBR = (PartitionMBR*)(sector + 0x1BE);
		
		/* ≈сли условие верно, то примен€етс€ UEFI */
		if (partitionRecMBR->partitionID == 0xEE) {

			/* „тение первого сектора диска(LBA == 1) */
			ReadFile(hDrive, sector, SIZE_OF_SECTOR, &numOfBytesRead, 0);
			partitionTblHeader = (PartitionTableHeader*)sector;

			if (strncmp(partitionTblHeader->signature, "EFI PART", strlen(partitionTblHeader->signature) - 1)) {
				printf("Incorrect GPT header signature");
				return -1;
			}
			
			printf("Hard disk %d:\n", i);
			numOfPartition = partitionTblHeader->numberOfPartition;
			for (j = 0; j < numOfPartition; j++) {
			
				if (flForLBA == 1) {
					flForLBA = 0;
					break;
				}

				/* „тение LBA == 2 - 33. Ѕлоки содержат информацию о разделах жЄсткого диска */
				ReadFile(hDrive, sector, SIZE_OF_SECTOR, &numOfBytesRead, 0);
				partEntrGPT = (PartitionEntrieGPT*)sector;

				for (l = 0; l < sizeof(sector)/sizeof(PartitionEntrieGPT); l++) {
					if (partEntrGPT->firstLBA == 0) { 
						flForLBA = 1;
						break;
					}

					/* Ќа самом деле им€ раздела не может занимать более 72 мЅ, однако, чтобы не перегружать код константами, в цикле используетс€ размер записи раздела. */
					printf("%d Partition. ", count);
					printf("Name - ");
					for (k = 0; k < sizeof(PartitionEntrieGPT)/2; k+=2) {
						if (partEntrGPT->partName[k-1] == 0 && partEntrGPT->partName[k] == 0) break;
						printf("%c", partEntrGPT->partName[k]);
						
						/* «апоминаем им€ раздела дл€ дальнейшего поиска логического диска */
						tempName[k/2] = partEntrGPT->partName[k];
					}
					tempName[k/2] = 0;

					tempOffset.QuadPart = partEntrGPT->firstLBA * 512; 
					if (tempOffset.HighPart == 0) printf(". Offset - %d MB\n",tempOffset.QuadPart/(1024*1024));
					else printf(". Offset - %d GB\n",tempOffset.QuadPart/(1024*1024*1024));
							
					/* ѕоиск логического диска. ≈сли он найден, инициализируетс€ структура, описывающа€ такой диск. */
					if (!strcmp(tempName, "Basic data partition") && partEntrGPT->attrFlags == 0) 
						logicalDrive[n - 1] = init(logicalDrive[n++], hDrive, tempOffset);					 
					
					isNTFS(logicalDrive[n-1]);

					partEntrGPT++;
					count++;
				}
			}
		}
		else {
			if (flForOutput) { printf("Computer use MBR\n"); flForOutput = 0;}
			printf("Hard disk %d:\n", i);
			for (j = 0; j < 4; j++) {
				if (partitionRecMBR->numberOfSectors == 0) break;

				if (partitionRecMBR->bootInd == 0x80) printf("Partition %d is boot partition. ", count);
				else printf("Partition %d. ", n);

				switch(partitionRecMBR->partitionID) {
				
				case EMPTY_PARTITON: { printf("Empty Partitiom. "); break; }
				case FAT12: { printf("FAT12. "); break; }
				case XENIX_ROOT:{printf("Xenix root. "); break;}
				case XENIX_USR: { printf("Xenix user. "); break; }
				case FAT16: { printf("FAT16. "); break; }
				case EXTENDED_PARTITION_WITH_CHS: { printf("Extended partition with CHS. "); break; }
				case FAT16B: { printf("FAT16B. "); break; }
				case NTFS_OR_IFS_OR_HPPS_OR_exFAT: { printf("NTFS or IFS or HPPS or exFAT. "); break; }
				case FAT32_WITH_CHS: { printf("FAT32 with CHS. "); break; }
				case FAT32_WITH_LBA: { printf("FAT32 with LBA. "); break; }
				case FAT16B_WITH_LBA: { printf("FAT16B with LBA. "); break; }
				case EXTENDED_PARTITION_WITH_LBA: { printf("Extended partition with LBA. "); break; }

				default: break;
				}
				tempOffset.QuadPart = partitionRecMBR->absoluteNumberOfSector * SIZE_OF_SECTOR;
				if (tempOffset.HighPart == 0) printf("Offset - %d Mb \n", tempOffset.QuadPart/(M_BYTES));
				else printf("Offset - %d GB \n", tempOffset.QuadPart / (K_BYTES * M_BYTES));
				
				if(partitionRecMBR->partitionID == NTFS_OR_IFS_OR_HPPS_OR_exFAT)
					logicalDrive[n - 1] = init(logicalDrive[n++], hDrive, tempOffset);

				isNTFS(logicalDrive[n - 1]);
				
				partitionRecMBR++;
				count++;
			}
		}
	}
	return 0;
}