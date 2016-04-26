#include "common.h"
#include "superReader.h"

int lineLen;
int currPos;
int eof;
char buffer[SUPER_BUFFER_LEN];

int GetNextLine(void)
{
  char* p;

  eof = 0;  
  
  p = fgets(buffer, SUPER_BUFFER_LEN, stdin);
  if (p == NULL)
  {
    if (ferror(stdin))
      return -1;
    eof = 1;
    return 1;
  }

  lineLen = strlen(buffer);
  currPos = 0;

  return 0;
}

int GetNextChar(char* b, int maxBuffer)
{
  while (currPos >= lineLen)
  {
    GetNextLine();
  }

  b[0] = buffer[currPos];
  currPos++;

  if (b[0] == 0)
    return 0;
  else
    return 1;
}

