#ifndef callback_h
#define callback_h

// helper
void cpyStr(char** dest, char* source);
void scanStr(char* dest, char* source);
void cpyPara(void* dest, void* source, uint len);
void printMsg(FILE* outfile, struct Msg* msg);
void printPairMsg(FILE* outfile, struct PairMsg* pairMsg);
void printKey(FILE* outfile, struct PairMsg* pairMsg);

// callback
char* strCB(char* origin, char* ch);

struct Pair* itemCB(char* keyStr, char* valueStr);

map<string, string>* pairlistCB(map<string, string>* oldList,
    struct Pair* item);

struct Pair* pairCB(char* keyStr, char* valueStr);

struct MsgHeader* headerCB(char* timeStr,
    char* typeStr,
    char* funcStr,
    char* serialStr);

struct PairMsg* pairlineCB(struct MsgHeader* header, struct Pair* pair);

struct Msg* msgCB(struct MsgHeader* header, char* str);

struct PairMsg* pairMsgCB(struct MsgHeader* header,
    char* str,
    map<string, string>* pairList);

struct PairMsg* pairlineMsgCB(struct MsgHeader* header,
    struct Pair* pair);

#endif
