#include "common.h"
#include "bean.h"
extern FILE* outfile;

void freeMsgHeader(struct MsgHeader* header)
{
  free(header->timestamp);
  free(header->type);
  free(header->function);
  free(header->serial);
  free(header);
}

void freeMsg(struct Msg* msg)
{
  freeMsgHeader(msg->header);
  free(msg->message);
  free(msg);
}

void freePair(struct Pair* pair)
{
  free(pair->key);
  if (pair->value != NULL)
    free(pair->value);
  free(pair);
}

void freePairMsg(struct PairMsg* pairMsg)
{
  freeMsgHeader(pairMsg->header);
  if (NULL != pairMsg->str)
    free(pairMsg->str);
  pairMsg->pairs->clear();
  delete pairMsg->pairs;
  free(pairMsg);
}
