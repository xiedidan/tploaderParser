#ifndef SUPERREADER_H
#define SUPERREADER_H

#define SUPER_BUFFER_LEN 16384

extern int lineLen;
extern int currPos;
extern int eof;

extern char buffer[SUPER_BUFFER_LEN];
extern int GetNextChar(char* b, int maxBuffer);

#endif
