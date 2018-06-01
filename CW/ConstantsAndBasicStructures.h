#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Windows.h>

#define K_BYTES 1024
#define M_BYTES 1024 * 1024
#define PERCENT 100

#define SIZE_OF_SECTOR 512
#define NUM_OF_ENTRIES 128

#define FULL_PATH 1024

enum ConsoleColor {
	black = 0,
	blue = 1,
	green = 2,
	cyan = 3,
	red = 4,
	magenta = 5,
	brown = 6,
	lightGray = 7,
	darkGray = 8,
	lightBlue = 9,
	lightGreen = 10,
	lightCyan = 11,
	lightRed = 12,
	lightMagenta = 13,
	yellow = 14,
	white = 15
};

typedef struct {
  DWORDLONG UsnJournalID;
  USN       FirstUsn;
  USN       NextUsn;
  USN       LowestValidUsn;
  USN       MaxUsn;
  DWORDLONG MaximumSize;
  DWORDLONG AllocationDelta;
  WORD      MinSupportedMajorVersion;
  WORD      MaxSupportedMajorVersion;
} USN_JOURNAL_DATA_V1_A, *PUSN_JOURNAL_DATA_V1_A;

#endif