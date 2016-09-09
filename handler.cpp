#include "common.h"
#include <unistd.h>
#include <errno.h>
#include "bean.h"
#include "handler.h"
#include "utfconv.h"
#include "multiProc.h"
#include "debug.h"

char* MsgPkg::cache = NULL;
int MsgPkg::cachePos = 0;

extern int forkPointer;
int serialno;
int serviceCount = 1;
long timeout;
extern int cacheSize;
extern int service;
extern FILE* errfile;
extern FILE* outfile;
extern int yylineno;
extern char* nscaPath;
extern char* nscaHost;
extern char* nscaConf;
extern char* outfilePath;
extern FILE* debugFile;
extern int keyCountFlag;
extern int forkCount;
char* logPath = "./tploaderCount";
extern int sendCount;

map<string, struct KeyRec> keyTable;
list<MsgPkg*> transQueue;
char* machineTag;

void checkTimeout(char* timeStr)
{
  list<MsgPkg*>::iterator pkgIter = transQueue.begin();
  while (pkgIter != transQueue.end())
  {
    if ((*pkgIter)->checkTimeout(timeStr))
    {
      char* pkgStr = (*pkgIter)->assemble(ASM_FLAG_TIMEOUT);
      (*pkgIter)->sendNsca(pkgStr);
      (*pkgIter)->clear();
      pkgIter = transQueue.erase(pkgIter);
      free(pkgStr);
    }
    pkgIter++;
  }
}

void msgProbe(struct Msg* msg)
{
  if (msg != NULL)
  {
    checkTimeout(msg->header->timestamp);

    if (msg->message != NULL)
      if (!strcmp(msg->message, "into tploader"))
      {
        MsgPkg* pkg = new MsgPkg(msg);
        transQueue.push_back(pkg);
      }
      else
        freeMsg(msg);
    else
      freeMsg(msg);
  }
}

void pairMsgProbe(struct PairMsg* pairMsg)
{
  checkTimeout(pairMsg->header->timestamp); 

  if (pairMsg->pairs->size() > 1)
  {
    if (pairMsg->str != NULL)
    {
      // response
      list<MsgPkg*>::reverse_iterator pkgIter = transQueue.rbegin();
      while (pkgIter != transQueue.rend())
      {
        if (!(*pkgIter)->matchRsp((*pkgIter)->getReqMsg(), pairMsg))
          pkgIter++;
        else
          break;
      }

      if (pkgIter != transQueue.rend())
      {
        string keyStr("PkgType");
        string valueStr("RspPkg");
        pairMsg->pairs->insert(map<string, string>::value_type(keyStr, valueStr));
        string timeKey("RspTime");
        string timeValue(pairMsg->header->timestamp);
        pairMsg->pairs->insert(map<string, string>::value_type(timeKey, timeValue));

        (*pkgIter)->addResponse(pairMsg);
        char* pkgStr = (*pkgIter)->assemble(ASM_FLAG_NORMAL);
        (*pkgIter)->sendNsca(pkgStr);
        free(pkgStr);
        (*pkgIter)->clear();
        transQueue.erase(--pkgIter.base());
      }
      else
      {
        fprintf(errfile, "Unmatched rsp package, yylineno: %d\n", yylineno-1);
        fflush(errfile);
        freePairMsg(pairMsg);
      }
    }
    else
    {
      // request
      list<MsgPkg*>::reverse_iterator pkgIter = transQueue.rbegin();
      while (pkgIter != transQueue.rend())
      {
        if (!(*pkgIter)->matchReq((*pkgIter)->getMsg(), pairMsg))
          pkgIter++;
        else
          break;
      }

      if (pkgIter != transQueue.rend())
      {
        string keyStr("PkgType");
        string valueStr("ReqPkg");
        pairMsg->pairs->insert(map<string, string>::value_type(keyStr, valueStr));

        (*pkgIter)->addRequest(pairMsg);
      }
      else
      {
        fprintf(errfile, "Unmatched req package, yylineno: %d\n", yylineno-1);
        fflush(errfile);
        freePairMsg(pairMsg);
      }
    }
  } // if pairs->size > 1
  else
    freePairMsg(pairMsg);
}

MsgPkg::MsgPkg(struct Msg* msg)
{
  this->msg = msg;
  reqMsg = NULL;
  rspMsg = NULL;
}

MsgPkg::~MsgPkg()
{
  clear(); 
}

void MsgPkg::clear()
{
  if (msg)
  {
    freeMsg(msg);
    // delete msg;
  }
  if (reqMsg)
  {
    freePairMsg(reqMsg);
    // delete reqMsg;
  }
  if (rspMsg)
  {
    freePairMsg(rspMsg);
    // delete rspMsg;
  }
  msg = NULL;
  reqMsg = NULL;
  rspMsg = NULL;
}

/*
void MsgPkg::sendNsca(char* message)
{
  char* nscaCmd = NULL;
  char* cmd = NULL;

  nscaCmd = (char*)malloc(1024);
  nscaCmd[0] = '\0';
  strcat(nscaCmd, nscaPath);
  strcat(nscaCmd, " -H ");
  strcat(nscaCmd, nscaHost);
  strcat(nscaCmd, " -d \"~\" -c ");
  strcat(nscaCmd, nscaConf);

  cmd = (char*)malloc(8192);
  strcat(cmd, "echo \"");
  strcat(cmd, message);
  strcat(cmd, "\" | ");
  strcat(cmd, nscaCmd);

  printf("%s\n", cmd);
  if (0 >= system(cmd))
    fprintf(errfile, "Failed to send message through nsca: %s\n", cmd);
  cache[0] = '\0';
  cachePos = 0;

  // printf("%s\n", cmd);
  if (nscaCmd)
    free(nscaCmd);
  if (cmd)
    free(cmd);
  return;
}
*/

void MsgPkg::sendNsca(char* message)
{
  char* nscaCmd = NULL;
  char* cmd = NULL;
  int msgLen = strlen(message)+1;
  char* utfBuffer = (char*)malloc(16384);
  int utfLen = g2u(message, msgLen, utfBuffer, 16384);

  if (utfLen <= 0)
  {
    // printf("UTF buffer:%d errno:%d %s\n", utfLen, errno, utfBuffer);
    fprintf(errfile, "Failed to convert %s to UTF-8.\n", message);
    return;
  }

  if (cacheSize > 0)
  {
    if (cachePos < cacheSize)
    {
      fprintf(outfile, "%s\n", utfBuffer);
      cachePos += utfLen;
    }
    else
    {
      fprintf(outfile, "%s\n", utfBuffer);
      cachePos += utfLen;

      fclose(outfile);

      nscaCmd = (char*)malloc(1024);
      nscaCmd[0] = '\0';
      strcat(nscaCmd, nscaPath);
      strcat(nscaCmd, " -H ");
      strcat(nscaCmd, nscaHost);
      strcat(nscaCmd, " -d \"~\" -c ");
      strcat(nscaCmd, nscaConf);

      cmd = (char*)malloc(2048);
      cmd[0] = '\0';
      strcat(cmd, "cat ");
      strcat(cmd, outfilePath);
      strcat(cmd, " | ");
      strcat(cmd, nscaCmd);
      cachePos = 0;
    }
  }
  else
  {
    nscaCmd = (char*)malloc(1024);
    nscaCmd[0] = '\0';
    strcat(nscaCmd, nscaPath);
    strcat(nscaCmd, " -H ");
    strcat(nscaCmd, nscaHost);
    strcat(nscaCmd, " -d \"~\" -c ");
    strcat(nscaCmd, nscaConf);

    cmd = (char*)malloc(17408);
    cmd[0] = '\0';
    strcat(cmd, "echo \"");
    strcat(cmd, utfBuffer);
    strcat(cmd, "\" | ");
    strcat(cmd, nscaCmd);
  }

  while (forkPointer >= forkCount)
    usleep(0);

  pid_t pid = fork();
  if (pid < 0)
    fprintf(errfile, "Failed to send message through nsca: %s\n", cmd);
  else if (0 == pid)
  {
    // this is child process
    system(cmd);

    if (utfBuffer)
      free(utfBuffer);
    if (nscaCmd)
      free(nscaCmd);
    if (cmd)
      free(cmd);
    
    exit(0);
  }
  else
  {
    forkPointer++;
    // this is parent process
    sendCount++;

    printf("%s\n", utfBuffer);

    FILE* logfile = fopen(logPath, "a+");
    fprintf(logfile, "%d\n", sendCount);
    fclose(logfile);
  } 

  if (cacheSize > 0)
    outfile = fopen(outfilePath, "w");

  fprintfDebug(debugFile, "%s\n", cmd);

  if (utfBuffer)
    free(utfBuffer);
  if (nscaCmd)
    free(nscaCmd);
  if (cmd)
    free(cmd);

  return;
}

void MsgPkg::initCache()
{
  if (cache != NULL)
    free(cache);

  cache = (char*)malloc(cacheSize);
  cache[0] = '\0';
  return;
}

bool MsgPkg::matchReq(struct Msg* msg, struct PairMsg* reqMsg)
{
  if (!msg)
    return false;

  struct MsgHeader* oriHeader = msg->header;
  struct MsgHeader* newHeader = reqMsg->header;
  if ( !strcmp(oriHeader->serial, newHeader->serial) )
      return true;
  else
    return false;

}

bool MsgPkg::matchRsp(struct PairMsg* reqMsg, struct PairMsg* rspMsg)
{
  if (!reqMsg)
    return false;

  struct MsgHeader* oriHeader = reqMsg->header;
  struct MsgHeader* newHeader = rspMsg->header;
  if ( !strcmp(oriHeader->serial, newHeader->serial) )
      return true;
  else
    return false;

}

void MsgPkg::addRequest(struct PairMsg* reqMsg)
{
  this->reqMsg = reqMsg;
}

void MsgPkg::addResponse(struct PairMsg* rspMsg)
{
  this->rspMsg = rspMsg;
}

long MsgPkg::parseTime(char* timeStr)
{
  long result = 0;
  char* buffer = (char*)malloc(strlen(timeStr)+1);
  memcpy(buffer, timeStr, strlen(timeStr));
  buffer[strlen(timeStr)] = '\0';

  // struct tm* time;
  char* tok;
  vector<char*> tokList(0);

  tok = strtok(buffer, "- :");
  while (tok != NULL)
  {
    tokList.push_back(tok);
    tok = strtok(NULL, "- :");
  }

  result = atol(tokList[2])*3600000000l;
  result += atol(tokList[3])*60000000l;
  result += atol(tokList[4])*1000000l;
  result += atol(tokList[5]);

  tokList.clear();
  free(buffer);
  return result;

  // drop this method since it depends on assertion
  /*
  time->tm_mon = atoi(tokList[0]);
  time->tm_mday = atoi(tokList[1]);
  time->tm_hour = atoi(tokList[2]);
  time->tm_min = atoi(tokList[3]);
  time->tm_sec = atoi(tokList[4]);
  */

  /*
  // Since there's no year in log, we have to handle this with some trick
  // we should make sure that client and our system date diffs within 24 hours!
  time_t now;
  time(&now);
  struct tm* localNow = localtime(&now);
  if ( (localNow->mon == 12) && (localNow->mday == 31) )
  {
    if ( (time->tm_mon == 12) && (time->tm_mday == 31) )
      time->tm_year = localNow->year;
    else if ( (time->tm_mon == 1) && (time->mday == 1) )
      time->tm_year = localNow->year + 1;
  }
  else if ( (localNow->mon == 1) && (localNow->mday == 1) )
  {
    if ( (time->tm_mon == 12) && (time->tm_mday == 31) )
      time->tm_year = localNow->year - 1;
    else if ( (time->tm_mon == 1) && (time->mday == 1) )
      time->tm_year = localNow->year;
  }
  else
  {
    time->tm_year = localNow->tm_year;
  }
  */

  //return mktime(time);
}

bool MsgPkg::checkTimeout(char* timeStr)
{
  long timespan = 0l;
  long oldTime = parseTime(msg->header->timestamp);
  long newTime = parseTime(timeStr);
  if (oldTime > newTime)
    timespan = 23l*3600000000l+59l*60000000l+59l*1000000l+999999l - oldTime + newTime;
  else
    timespan = newTime - oldTime;

  if (timespan > timeout)
  {
    return true;
  }
  else
    return false;
}

char* MsgPkg::assemble(uint flag)
{
  char* result = (char*)malloc(8192);
  char* body = (char*)malloc(8192);
  char* tempResult = (char*)malloc(8192);

  result[0] = '\0';
  body[0] = '\0';
  tempResult[0] = '\0';

  switch (flag)
  {
    case ASM_FLAG_NORMAL:
      if (reqMsg)
      {
        assemblePairMsg(body, reqMsg, NULL);
        result = strcat(result, "tploader OK - ");
        // char* serialStr = (char*)malloc(8192);
        // serialStr[0] = '\0';
        // sprintf(serialStr, "SERIAL_NO=%d ", serialno);
        // strcat(serialStr, body);
        createHeader(result, body, flag);
        // free(serialStr);
        body[0] = '\0';
        // serialno++;
      }
      if (msg && rspMsg)
      {
        assemblePairMsg(body, rspMsg, msg->header->timestamp);
        // createHeader(tempResult, body, flag);
        // strcat(tempResult, body);
        strcat(result, body);
        result[strlen(result)-1] = '$';
      }
      break;
    case ASM_FLAG_TIMEOUT:
      if (reqMsg)
        assemblePairMsg(body, reqMsg, msg->header->timestamp);
      result = strcat(result, "tploader OK - ");
      createHeader(result, body, flag);
      strcat(result, "$");
      break;
  }

  free(tempResult);
  if (body)
    free(body);

  char* header = (char*)malloc(8192);
  if (service > 0)
  {
    if (serviceCount > service)
      serviceCount = 1;
    else
      serviceCount++;

    header[0] = '\0';
    strcat(header, "localhost~TPLOADER");
    char* serviceStr = (char*)malloc(10);
    sprintf(serviceStr, "%d", serviceCount);
    strcat(header, serviceStr);
    strcat(header, "~0~");
    strcat(header, result);
    free(result);
  }
  else
  {
    header[0] = '\0';
    strcat(header, "localhost~TPLOADER~0~");
    strcat(header, result);
    free(result);
  }

  return header;
}

char* MsgPkg::escapeChar(char* src)
{
  char esc[] = " =";
  char tgt[] = "@^";
  int len = strlen(esc);

  for (uint i=0; i<len; i++)
  {
    char* str = strchr(src, esc[i]);
    while (str != NULL)
    {
      *str = tgt[i];
      str = strchr(str+1, esc[i]);
    }
  }

  return src;
}

char* MsgPkg::assemblePairMsg(char* result,struct PairMsg* pairMsg, char* startTime)
{
  char* keyStr = (char*)malloc(1024);
  char* valueStr = (char*)malloc(1024);
  keyStr[0] = '\0';
  valueStr[0] = '\0';

  if (startTime)
  {
    // must be rsp
    result = strcat(result, "PkgStartTime=");
    startTime = escapeChar(startTime);
    result = strcat(result, startTime);
    result = strcat(result, " ");

    if (pairMsg->str != NULL)
    {
      char* buffer = (char*)malloc(strlen(pairMsg->str)+1);
      buffer[0] = '\0';
      strcat(buffer, pairMsg->str);
      char* tok = strtok(buffer, " ");
      if (tok != NULL)
      {
        tok = strtok(NULL, " ");
        if (tok != NULL)
        {
          result = strcat(result, "TransCode=");
          result = strcat(result, tok);
          result = strcat(result, " ");
        }
      }
      free(buffer);
    }
    else
    {
      fprintf(errfile, "No TransCode found. yylineno: %d\n", yylineno-1);
      fflush(errfile);
    }
  }

  map<string, string> pairs = *(pairMsg->pairs);
  map<string, string>::iterator it = pairs.begin();
  while (it != pairs.end())
  {
    if (keyCountFlag == 1)
    {
      string keyStr(it->first.c_str());
      map<string, struct KeyRec>::iterator keyIt;
      keyIt = keyTable.find(keyStr);
      if (keyIt == keyTable.end())
      {
        struct KeyRec key;
        key.example = (char*)malloc(strlen(it->first.c_str())+1);
        key.example[strlen(it->first.c_str())] = '\0';
        memcpy(key.example, it->first.c_str(), strlen(it->first.c_str()));
        key.count = 1;
        keyTable.insert(map<string, struct KeyRec>::value_type(it->first.c_str(), key));
      }
      else
        keyIt->second.count++;
    }

    strcat(keyStr, it->first.c_str());
    escapeChar(keyStr);
    result = strcat(result, keyStr);
    result = strcat(result, "=");

    strcat(valueStr, it->second.c_str());
    if (0 == strlen(it->second.c_str()))
      valueStr = strcat(valueStr, "{null}");
    escapeChar(valueStr);
    result = strcat(result, valueStr);
    result = strcat(result, " ");

    keyStr[0] = '\0';
    valueStr[0] = '\0';

    it++;
  }

  pairs.clear();
  free(keyStr);
  free(valueStr);
  return result;
}

char* MsgPkg::createHeader(char* result, char* msgStr, uint flag)
{
  char* timeStr = (char*)malloc(80);
  int timeLen = 0;
  time_t rawTime;
  struct tm* now;

  time(&rawTime);
  now = localtime(&rawTime);
  // strftime(timeStr, 80, "%F %T:000000", now);
  strftime(timeStr, 80, "%F %T", now);
  timeLen = strlen(timeStr);
  switch (flag)
  {
    case ASM_FLAG_NORMAL:
      result = strcat(result, timeStr);
      result = strcat(result, ";");
      result = strcat(result, machineTag);
      result = strcat(result, ";");
      result = strcat(result, "tploader;ACTIVE;");
      if (msgStr)
        result = strcat(result, msgStr);
      break;
    case ASM_FLAG_TIMEOUT:
      result = strcat(result, timeStr);
      result = strcat(result, ";");
      result = strcat(result, machineTag);
      result = strcat(result, ";");
      result = strcat(result, "tploader;WARNING;");
      if (msgStr)
        result = strcat(result, msgStr);
      break;
  }

  free(timeStr);
  return result;
}
