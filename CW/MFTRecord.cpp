#include "MFTRecord.h"
#include "ConstantsAndBasicStructures.h"

int readUSNJournal(NTFSDrive entrie, delFileInfoUsn **begin) {
	HANDLE hVol;
	CHAR buff[BUFF_LEN], tempPath[MAX_PATH];
	int fl = 1, flForExample = 1;
	delFileInfoUsn *temp, *curr;
	curr = *begin;

	/* USN_JOURNAL_DATA_V1_A journalData; */
	USN_JOURNAL_DATA journalData;
	READ_USN_JOURNAL_DATA readData = {0, USN_REASON_FILE_DELETE, FALSE, 0, 0};
	PUSN_RECORD usnRecord;

	DWORD dwBytesRead, dwRetBytes;
	
	setlocale(LC_ALL,"Russian");

	sprintf(tempPath, "\\\\.\\%c:", entrie.driveLetter);
	hVol = CreateFile( tempPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hVol == INVALID_HANDLE_VALUE) {
		printf("Error %d", GetLastError());
		return -1;
	}

	if (!DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &dwBytesRead, NULL)) {
		printf("Query journal failed - %d\n", GetLastError());
		
		/* Если журнал изменения тома неактивен, то происходит его создание */
		if (GetLastError() == ERROR_JOURNAL_NOT_ACTIVE) {
			printf("The volume change journal is not active.\n");
			printf("Now this journal will be create with maximum size == 32 MB and allocation delta == 8 MB\n");
			printf("If you want to make any changes, please visit msdn.microsoft.com");

			CREATE_USN_JOURNAL_DATA newUsn = {0x2000000, 0x800000};
			if (!DeviceIoControl(hVol, FSCTL_CREATE_USN_JOURNAL, &newUsn, sizeof(newUsn) , NULL, 0, &dwRetBytes, 0)) {
				printf("Error &d", GetLastError());
				
				return -3;
			}
		}

		return -2;
	}
		
	readData.UsnJournalID = journalData.UsnJournalID;
	
	while(fl) {
		memset( buff, 0, BUFF_LEN);

		if (!DeviceIoControl(hVol, FSCTL_READ_USN_JOURNAL, &readData, sizeof(readData), &buff, BUFF_LEN, &dwBytesRead, NULL)) {
			printf("Read journal failed - %d \n", GetLastError());
			return -2;
		}

		dwRetBytes = dwBytesRead - sizeof(USN);
		
		if (dwRetBytes == 0) break;

		usnRecord = (PUSN_RECORD)(((PUCHAR)buff) + sizeof(USN));

		while(dwRetBytes > 0) {
			
			if (flForExample) {
				printf("USN: %I64x \n", usnRecord->Usn);
				printf("File name: %.*S \n", usnRecord->FileNameLength / 2, usnRecord->FileName);
				printf("Reason: %x \n", usnRecord->Reason);
				printf("Parent director: %s \n", getFullPath(hVol, usnRecord->ParentFileReferenceNumber));
				printf("\n");
				flForExample = 0;
			}

			usnRecord->ParentFileReferenceNumber;
			dwRetBytes -= usnRecord->RecordLength;

			if (!(temp = (delFileInfoUsn*)malloc(sizeof(delFileInfoUsn))))
				return -4;

			temp->fileName = usnRecord->FileName;
			temp->fileNameLen = usnRecord->FileNameLength;
			temp->parentFileReferenceNumber = usnRecord->ParentFileReferenceNumber;
			temp->usn = usnRecord->Usn;

			if (curr == NULL) {
				temp->next = *begin;
				*begin = temp;
				curr = *begin;
			}
			else {
				curr->next = temp;
				temp->next = NULL;
				curr = curr->next;
			}

			usnRecord = (PUSN_RECORD)(((PUCHAR)usnRecord) + usnRecord->RecordLength);
		}

		readData.StartUsn = *(USN *)&buff;
	}

	CloseHandle(hVol);
	return 0;
}

char* getFullPath(HANDLE hVol, DWORDLONG fileRefNum) {
	FILE_ID_DESCRIPTOR fID;
	char buff[FULL_PATH];

	ZeroMemory(&fID, sizeof(fID));
	fID.FileId.QuadPart = fileRefNum;
	fID.dwSize = sizeof(fID);
	fID.Type = FileIdType;

	HANDLE hFile = OpenFileById(hVol, &fID, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		sprintf(buff, "Error %d", GetLastError());
		return buff;
	}

	if(!GetFinalPathNameByHandle(hFile, buff, FULL_PATH, 0)) {
		sprintf(buff, "Error %d", GetLastError());
		return buff;
	}

	return buff;
}

int readMFTRecord(NTFSDrive entrie, delFileInfoMFT **begin) {
	LARGE_INTEGER offsetMFT;
	BYTE rec[SIZE_OF_REC];
	TCHAR tempPath[MAX_PATH], tempName[5] = "FILE";
	DWORD numOfBytesRead, nextAttr;
	int fl = 1, flForAttr = 1, i, lengthName, flForOutput = 0, flForExit = 0, k = 0;
	MFTBaseRecord *baseRec;
	NTFSAttr *attr;
	FileName *fileName;
	delFileInfoMFT *temp, *curr;
	
	curr = *begin;
	offsetMFT.QuadPart = entrie.volumeOffsetOfMFT.QuadPart;
	
	sprintf(tempPath, "\\\\.\\%c:", entrie.driveLetter);
	HANDLE hVol = CreateFile( tempPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hVol == INVALID_HANDLE_VALUE) {
		printf("Error %d", GetLastError());
		return -1;
	}

	DWORD dwPtr = SetFilePointer(hVol, offsetMFT.LowPart, &offsetMFT.HighPart, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) {
		printf("Error %d", GetLastError());
		return -1;
	}
	
	while (fl) {
	memset(rec, 0, sizeof(rec));

	if(!ReadFile(hVol, rec, sizeof(rec), &numOfBytesRead, 0)){
		printf("Error %d", GetLastError());
		return -2;
	}	
	
	baseRec = (MFTBaseRecord *) rec;
	nextAttr = 0;

	for (i = 0; i < sizeof(baseRec->signatureF); i++) {
		if (tempName[i] != baseRec->signatureF[i]) flForExit = 1;
	}

	if (flForExit == 1 && baseRec->flags != 0) 
		break;
	else flForExit = 0;

		if (baseRec->flags == MFT_RECORD_NOT_USED) {
			k++;
			do{
	
				//memcpy(&attr, rec + (baseRec->attrOffset + nextAttr), sizeof(NTFSAttr));
				if (baseRec->attrOffset + nextAttr > baseRec->recordSize) break;
				attr = (NTFSAttr *)(rec + baseRec->attrOffset + nextAttr);
				switch(attr->type) {
				case 0x00: { flForAttr = 0; break; }
				case 0x10: break; 
				case 0x20: break;
		
				case 0x30: {

					/* Если атрибут резидентный */
					//if (attr.nonResFlag == 0) {}
					//memcpy(&fileName, &rec[baseRec->attrOffset + nextAttr + attr.nameOffset], sizeof(FileName));
					fileName = (FileName *)(rec + baseRec->attrOffset + nextAttr + attr->nameOffset);
					lengthName = (attr->size - attr->nameOffset - ( sizeof(FileName) - MAX_PATH)) / 2;
					if (lengthName > MAX_PATH) break;

					if (!(temp = (delFileInfoMFT*)malloc(sizeof(delFileInfoMFT))))
						return -4;

					if (flForOutput < 10) printf("File Name: ");
					for (i = 0; i < lengthName; i++) {
						if (flForOutput < 10) 
							printf("%c", fileName->fileName[i]);

						
						temp->fileName[i] = fileName->fileName[i];
						}

					if (flForOutput < 10) printf("\nParentDirectory: %s \n", getFullPath(hVol, fileName->baseParentDir));

					flForOutput++;

					temp->fileNameLen = lengthName;
					temp->parentFileReferenceNumber = fileName->baseParentDir;

					if (curr == NULL) {
						temp->next = *begin;
						*begin = temp;
						curr = *begin;
					}
					else {
						curr->next = temp;
						temp->next = NULL;
						curr = curr->next;
					}
				}

				case 0x40: break;
				case 0x50: break;
				case 0x60: break;
				case 0x70: break;
				case 0x80: break;
				case 0x90: break;
				case 0xA0: break;
				case 0xB0: break;
				case 0xC0: break;
				case 0xD0: break;
				case 0xE0: break;
				case 0xF0: break;
				case 0x100: break;
				case 0xFFFFFFFFFFFFFFFF: { flForAttr = 0; break; }
									
				default: break;
				}
		
				nextAttr += attr->size;

			}while(flForAttr);
		}
	
	flForAttr = 1;
	}

	return 0;
}