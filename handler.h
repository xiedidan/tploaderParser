#ifndef HANDLER_H
#define HANDLER_H

#define ASM_FLAG_NORMAL 0
#define ASM_FLAG_TIMEOUT 1

extern long timeout;

class MsgPkg
{
  public:
    MsgPkg(struct Msg* msg);
    ~MsgPkg();
    void clear();

    bool matchReq(struct Msg* msg, struct PairMsg* reqMsg);
    bool matchRsp(struct PairMsg* reqMsg, struct PairMsg* rspMsg);
    void addRequest(struct PairMsg* reqMsg);
    void addResponse(struct PairMsg* rspMsg);

    long parseTime(char* timeStr);
    bool checkTimeout(char* timeStr); // return true if timeout is detected
    char* assemble(uint flag);
    void sendNsca(char* message);
    static void initCache();

    struct Msg* getMsg() { return msg; }
    struct PairMsg* getReqMsg() { return reqMsg; }
  private:
    struct Msg* msg;
    struct PairMsg* reqMsg;
    struct PairMsg* rspMsg;
    char* assemblePairMsg(char* result, struct PairMsg* pairMsg, char* startTime);
    char* createHeader(char* result, char* msgStr, uint flag);
    char* escapeChar(char* src);

    static int cachePos;
    static char* cache;
};

extern list<MsgPkg*> transQueue;
extern char* machineTag;

void msgProbe(struct Msg* msg);
void pairMsgProbe(struct PairMsg* pairMsg);

#endif
