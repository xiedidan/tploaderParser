#ifndef bean_h
#define bean_h

// include <vector> in any file using this header

void freeMsgHeader(struct MsgHeader* header);
void freeMsg(struct Msg* msg);
void freePair(struct Pair* pair);
void freePairMsg(struct PairMsg* pairMsg);

struct MsgHeader
{
  char* timestamp;
  char* type;
  char* function;
  char* serial;
};

struct Msg
{
  struct MsgHeader* header;
  char* message;
};

struct Pair
{
  char* key;
  char* value;
};

struct PairMsg
{
  struct MsgHeader* header;
  char* str;
  map<string, string>* pairs;
};

struct KeyRec
{
  char* example;
  uint count;
};

#endif
