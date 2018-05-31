#include "MFTRecord.h"
#include "ConstantsAndBasicStructures.h"
#include <WinIoCtl.h>

int readUSNJournal(NTFSDrive entrie) {
	HANDLE hVol;
	CHAR buff[BUFF_LEN], tempPath[MAX_PATH];

	/* USN_JOURNAL_DATA_V1_A journalData; */
	USN_JOURNAL_DATA journalData;
	READ_USN_JOURNAL_DATA readData = {0, USN_REASON_FILE_DELETE, FALSE, 0, 0};
	PUSN_RECORD usnRecord;

	DWORD dwBytesRead, dwRetBytes;
	
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
	
	for (; ;) {
		memset( buff, 0, BUFF_LEN);

		if (!DeviceIoControl(hVol, FSCTL_READ_USN_JOURNAL, &readData, sizeof(readData), &buff, BUFF_LEN, &dwBytesRead, NULL)) {
			printf("Read journal failed - %d \n", GetLastError());
			return -2;
		}

		dwRetBytes = dwBytesRead - sizeof(USN);

		usnRecord = (PUSN_RECORD)(((PUCHAR)buff) + sizeof(USN));

		while(dwRetBytes > 0) {
			printf("USN : %I64x \n", usnRecord->Usn);
			printf("File name: %.*S \n", usnRecord->FileNameLength / 2, usnRecord->FileName);
			printf("Reason: %x \n", usnRecord->Reason);
			printf("\n");
			usnRecord->ParentFileReferenceNumber;
			dwRetBytes -= usnRecord->RecordLength;

			usnRecord = (PUSN_RECORD)(((PUCHAR)usnRecord) + usnRecord->RecordLength);
		}

		readData.StartUsn = *(USN *)&buff;
	}

	CloseHandle(hVol);
	return 0;
}