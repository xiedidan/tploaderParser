#include "common.h"
#include "bean.h"
#include "callback.h"
extern FILE* outfile;
extern int yylineno;

void cpyStr(char** dest, char* source)
{
  uint len = strlen(source);
  *dest = (char*)malloc(len+1);
  (*dest)[len] = '\0';
  memcpy(*dest, source, len);
}

void cpyPara(void* dest, void* source, uint len)
{
  dest = malloc(len);
  memcpy(dest, source, len);
}

void printHeader(FILE* out, struct MsgHeader* header)
{
  fprintf(out, "line #: %d\n", yylineno-1);
  fprintf(out, "timestamp = %s\n", header->timestamp);
  fprintf(out, "type = %s\n", header->type);
  fprintf(out, "function = %s\n", header->function);
  fprintf(out, "serial = %s\n", header->serial);
}

void printMsg(FILE* out, struct Msg* msg)
{
  if (false)
  {
    printHeader(out, msg->header);
    fprintf(out, "message = %s\n\n", msg->message);
    fflush(out);
  }
  if (msg != NULL)
    freeMsg(msg);
}

/*
void printPairMsg(FILE* out, struct PairMsg* pairMsg)
{
  printHeader(out, pairMsg->header);
  
  if (!pairMsg->str)
    fprintf(out, "info = NULL\n");
  else
    fprintf(out, "info = %s\n", pairMsg->str);

  vector<struct Pair*> pairs = *(pairMsg->pairs);
  for (uint i=0; i<pairs.size(); i++)
  {
    struct Pair* pair = pairs[i];
    fprintf(out, "%s = %s\n", pair->key, pair->value);
  }

  fprintf(out, "\n");
  fflush(out);
  // freePairMsg(pairMsg);
}

void printKey(FILE *out, struct PairMsg* pairMsg)
{
  vector<struct Pair*> pairs = *(pairMsg->pairs);
  if (pairs.size() > 1)
  {
    for (uint i=0; i<pairs.size(); i++)
    {
      struct Pair* pair = pairs[i];
      if (pair->key != NULL)
      {
        // fprintf(out, "line: %d key: %s\n", yylineno, pair->key);
        // fflush(out);
        string str(pair->key);
        map<string, struct KeyRec>::iterator it;
        it = keyTable.find(str);
        if (it == keyTable.end())
        {
          struct KeyRec key;
          key.example = (char*)malloc(strlen(pair->value)+1);
          key.example[strlen(pair->value)] = '\0';
          memcpy(key.example, pair->value, strlen(pair->value));
          key.count = 1;
          keyTable.insert(map<string, struct KeyRec>::value_type(pair->key, key));
        }
        else
          it->second.count++;
      }
    }
  }

  freePairMsg(pairMsg);
}
*/

char* strCB(char* origin, char* ch)
{
  char* dest = NULL;

  if (ch == NULL)
  {
    dest = (char*)malloc(1);
    dest[0] = '\0';
  }
  else
  {
    if (origin == NULL)
    {
      dest = (char*)malloc(2);
      dest[0] = ch[0];
      dest[1] = '\0';
    }
    else
    {
      uint oriLen = strlen(origin);
      dest = (char*)malloc(oriLen+2);
      memcpy(dest, origin, oriLen);
      dest[oriLen] = ch[0];
      dest[oriLen+1] = '\0';
    }
  }

  if (ch != NULL)
    free(ch);
  if (origin != NULL)
    free (origin);

  return dest;
}

struct Pair* itemCB(char* keyStr, char* valueStr)
{
  uint keyLen;
  uint valueLen;


  struct Pair* item = (struct Pair*)malloc(sizeof(struct Pair)); 

  item->key = keyStr;
  item->value = valueStr;
  // cpyStr(&(pair->key), keyStr);
  // cpyStr(&(pair->value), valueStr);

  return item;
}

map<string, string>* pairlistCB(map<string, string>* oldList,
    struct Pair* item)
{
  map<string, string>* pairList;

  if (oldList != NULL)
    pairList = oldList;
  else
    pairList = new map<string, string>;

  string keyStr(item->key);
  string valueStr(item->value);
  pairList->insert(map<string, string>::value_type(keyStr, valueStr));

  freePair(item);
  return pairList;
}

struct Pair* pairCB(char* keyStr, char* valueStr)
{
  return itemCB(keyStr, valueStr);
}

struct MsgHeader* headerCB(char* timeStr,
    char* typeStr,
    char* funcStr,
    char* serialStr)
{
  struct MsgHeader* header = (struct MsgHeader*)malloc(sizeof(struct MsgHeader));

  header->timestamp = timeStr;
  header->type = typeStr;
  header->function = funcStr;
  header->serial = serialStr;

  return header;
}

struct PairMsg* pairlineCB(struct MsgHeader* header, struct Pair* pair)
{
  struct PairMsg* pairMsg = (struct PairMsg*)malloc(sizeof(struct PairMsg));

  pairMsg->header = header;
  pairMsg->str = NULL;
  pairMsg->pairs = new map<string, string>;
  string keyStr(pair->key);
  string valueStr(pair->value);
  pairMsg->pairs->insert(map<string, string>::value_type(keyStr, valueStr));

  freePair(pair);
  return pairMsg;
}

struct Msg* msgCB(struct MsgHeader* header, char* str)
{
  struct Msg* msg = (struct Msg*)malloc(sizeof(struct Msg));
  msg->header = header;
  msg->message = str;

  return msg;
}

struct PairMsg* pairMsgCB(struct MsgHeader* header,
    char* str,
    map<string, string>* pairList)
{
  struct PairMsg* pairMsg = (struct PairMsg*)malloc(sizeof(struct PairMsg));
  pairMsg->header = header;
  pairMsg->str = str;
  pairMsg->pairs = pairList;

  return pairMsg;
}

struct PairMsg* pairlineMsgCB(struct MsgHeader* header,
    struct Pair* pair)
{
  {
    struct PairMsg* pairMsg = (struct PairMsg*)malloc(sizeof(struct PairMsg));
    pairMsg->header = header;
    pairMsg->str = NULL;
    pairMsg->pairs = new map<string, string>;
    string keyStr(pair->key);
    string valueStr(pair->value);
    pairMsg->pairs->insert(map<string, string>::value_type(keyStr, valueStr));

    freePair(pair);
    return pairMsg;
  }
}
