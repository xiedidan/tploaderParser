%{
#define YYERROR_VERBOSE 1
#include "common.h"
#include "bean.h"
#include "callback.h"
#include "tploaderParser.h"
#include "handler.h"
#include "debug.h"
#include "superReader.h"
#include "multiProc.h"

  extern int serialno;

  int forkCount;
  int cacheSize;
  int service;
  char* nscaPath;
  char* nscaHost;
  char* nscaConf;
  char lastErrorLine[SUPER_BUFFER_LEN];

  bool errorFileFlag;
  FILE* errorFile;
  FILE* keyCountFile;
  int keyCountFlag;
  char* outfilePath;
  FILE* outfile;
  FILE* debugFile;
  extern FILE* yyin;
  extern int yylex(void);
  extern int yylineno;
  FILE* errfile;
  extern bool valueFlag;
  int sendCount;
  extern map<string, struct KeyRec> keyTable;
%}

%union
{
  char* ustr;
  struct Pair* upair;
  map<string, string>* upairList;
  struct MsgHeader* uheader;
  struct PairMsg* upairMsg;
  struct Msg* umsg;
}

%token <ustr> TIME LINENO MODULE PROCID CHAR
%token KEYSTR
%token VALSTR
%token ENDITEM
%token EOL
%token EOM
%token XMLEND
%token XMLHEADER

%type <ustr> key str xmlmsg
%type <upair> pair item
%type <upairList> pairlist
%type <uheader> header
%type <upairMsg> pairmsg
%type <umsg> msg

%%

log:
| log msg { 
  if ((yylineno%1000) == 0)
  {
    fflush(stdout);
  }
  msgProbe($2);
}
| log pairmsg { 
  if ((yylineno%1000) == 0)
  {
    fflush(stdout);
  }
  pairMsgProbe($2);
}
| log error { 
  if (0 != strlen(lastErrorLine))
  {
    if (strcmp(lastErrorLine, buffer))
    {
      lastErrorLine[0] = '\0';
      strcat(lastErrorLine, buffer);
      if (errorFileFlag)
        fprintf(errorFile, "%s\n", buffer);
    }
  }
  else
  {
    lastErrorLine[0] = '\0';
    strcat(lastErrorLine, buffer);
    if (errorFileFlag)
      fprintf(errorFile, "%s\n", buffer);
  }
}
;

msg: header str EOM { $$ = msgCB($1, $2); }
//| header str EOL { $$ = msgCB($1, $2); }
| str EOM { $$ = NULL; free($1); /* nothing */ }
;

pairmsg: header pairlist EOM { $$ = pairMsgCB($1, NULL, $2); }
| header str pairlist EOM { $$ = pairMsgCB($1, $2, $3); }
| header pair EOM { $$ = pairlineMsgCB($1, $2); }
;

xmlmsg: header XMLHEADER str EOM { $$ = NULL; 
          freeMsgHeader($1);
          free($3);
          /* nothing */ }
;

header: TIME '|' LINENO '|' MODULE '|' PROCID '|' { $$ = headerCB($1, $3, $5, $7); }
;

pair: KEYSTR str ',' VALSTR str { $$ = pairCB($2, $5); }
;

pairlist: pairlist item { $$ = pairlistCB($1, $2); }
| item { $$ = pairlistCB(NULL, $1); }
;

item: key str ENDITEM { $$ = itemCB($1, $2); }
;

key: '<' str '>' { valueFlag = true; $$ = $2; }
;

str: /* nothing */ { $$ = strCB(NULL, NULL); }
| str CHAR { $$ = strCB($1, $2); }
| CHAR { $$ = strCB(NULL, $1); }
;

%%

main(int argc, char** argv)
{
  printf("*** tploader parser v1.2.2 - 2016.04.25 ***\n");

  if (argc > 11)
  {
    /*
    if (!(yyin = fopen(argv[1], "r")))
      return 1;
    outfile = fopen(argv[2], "w");
    yyin = stdin;
    */

    errorFileFlag = false;
    if (errorFileFlag)
    {
      errorFile = fopen("./errorFile", "w");
      if (!(errorFile))
        return 1;
    }

    cacheSize = atoi(argv[1]);

    outfilePath = (char*)malloc(strlen(argv[2])+1);
    outfilePath[0] = '\0';
    strcat(outfilePath, argv[2]);

    if (cacheSize > 0)
      if (!(outfile = fopen(outfilePath, "w")))
        return 1;

    nscaPath = (char*)malloc(strlen(argv[3])+1);
    nscaPath[0] = '\0';
    strcat(nscaPath, argv[3]);

    nscaHost = (char*)malloc(strlen(argv[4])+1);
    nscaHost[0] = '\0';
    strcat(nscaHost, argv[4]);

    nscaConf = (char*)malloc(strlen(argv[5])+1);
    nscaConf[0] = '\0';
    strcat(nscaConf, argv[5]);

    service = atoi(argv[6]);

    if (!(debugFile = fopen(argv[7], "w")))
      return 1;

    keyCountFlag = atoi(argv[8]);

    char* hostName = argv[9];

    timeout = atol(argv[10]);

    forkCount = atoi(argv[11]);

    signal(SIGCHLD, SigInt);

    errfile = fopen("./parserErr", "w");

    // MsgPkg::initCache();
    sendCount = 0;

    machineTag = (char*)malloc(80);
    memset(machineTag, '\0', 80);
    strcat(machineTag, hostName);
    strcat(machineTag, ":80");

    serialno = 0;

    yyparse();

    if (keyCountFlag == 1)
    {
      keyCountFile = fopen("./keyCountOut", "w");
      map<string, struct KeyRec>::iterator iter;
      for (iter = keyTable.begin();
          iter != keyTable.end();
          iter++)
        fprintf(keyCountFile, "key: %15s\t\tcount: %10d\t\teg. %s\n", (iter->first).c_str(), (iter->second).count, (iter->second).example);
      }

    free(nscaPath);
    free(nscaHost);
    free(nscaConf);

    fclose(errfile);

    return 0;
  }
  else
  {
    printf("tploader parser - usage : parser [cache size(byte)] [cache path] [nsca path] [nsca target ip] [nsca config path] [service count] [debug file path] [key count switch] [host tag] [timeout] [fork count]\n");
    printf("You need to specify cache path even when cache size is 0.\n");
    return 2;
  }
}

void yyerror(char* s)
{
  fprintf(errfile, "tploader parser error: %s (yylineno: %d)\n", s, yylineno-1);
}
