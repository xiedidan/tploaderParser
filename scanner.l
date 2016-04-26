%option noyywrap yylineno
%s HEADER
%x STACK KEY
%x ENDLINE
%x XML

%{
#include "common.h"
#include "callback.h"
#include "parser.tab.h"
#include "tploaderParser.h"
#include "debug.h"
#include "superReader.h"

#define YY_INPUT(buf, result, max_size) {\
  result = GetNextChar(buf, max_size); \
  if (result <= 0) \
    result = YY_NULL; \
}

  extern FILE* outfile;
  int stack = 0;
  bool angFlag = false;
  bool valueFlag = false;
%}

%%

"|" {
  if (valueFlag)
  {
    cpyStr(&(yylval.ustr), yytext);
    return CHAR;
  }
  else
  {
    printfDebug(" |");
    return yytext[0];
  }
}

"<" {
  if (valueFlag)
  {
    cpyStr(&(yylval.ustr), yytext);
    return CHAR;
  }
  else
  {
    printfDebug(" <");
    angFlag = true;
    return yytext[0];
  }
}

">" {
  if (valueFlag || !angFlag)
  {
    cpyStr(&(yylval.ustr), yytext);
    return CHAR;
  }
  else
  {
    printfDebug(" >");
    angFlag = false;
    return yytext[0];
  }
}

"<->" { cpyStr(&(yylval.ustr), yytext); return CHAR; }

"<?xml version=\"1.0\" encoding=\"GBK\"?>" { printfDebug( " XMLHEADER"); BEGIN XML; return XMLHEADER; }

<XML>"|\n" { printfDebug("EOM"); BEGIN INITIAL; return EOM; }

<XML>. { cpyStr(&(yylval.ustr), yytext); return CHAR;}

"," {
  if (true) // (valueFlag)
  {
    cpyStr(&(yylval.ustr), yytext);
    return CHAR;
  }
  else
  {
    printfDebug(" ,");
    return yytext[0];
  }
}

[0-9]{2}"-"[0-9]{2}" "[0-9]{2}":"[0-9]{2}":"[0-9]{2}":"[0-9]{6} { 
  printfDebug(" TIME");
  cpyStr(&(yylval.ustr), yytext);
  BEGIN HEADER;
  return TIME;
}

<HEADER>[0-9 ]{4} { printfDebug(" LINENO"); cpyStr(&(yylval.ustr), yytext); return LINENO; }

<HEADER>[0-9a-zA-Z ]{9} { printfDebug(" MODULE"); cpyStr(&(yylval.ustr), yytext); return MODULE; }

<HEADER>[0-9]+ { 
  printfDebug(" PROCID");
  cpyStr(&(yylval.ustr), yytext);
  BEGIN INITIAL;
  return PROCID;
}

"</>" { printfDebug(" ENDITEM"); valueFlag = false; return ENDITEM; }

"KEY[" { printfDebug(" KEYSTR"); BEGIN KEY; return KEYSTR; }

"VAL[" { printfDebug(" VALSTR"); BEGIN STACK; return VALSTR; }

"\n" {
  printfDebug("EOL \n");
    return EOL;
}

"|\n" {
  printfDebug("EOM \n");
  return EOM;
}

<KEY>"]," { printfDebug(" ,"); BEGIN INITIAL; return ','; }
<KEY>. { cpyStr(&(yylval.ustr), yytext); return CHAR; }
<STACK>"]|" { printfDebug(" |"); BEGIN INITIAL; return '|'; }
<STACK>"]|\n" { printfDebug(" EOM\n"); BEGIN INITIAL; return EOM; }
<STACK>. { cpyStr(&(yylval.ustr), yytext); return CHAR; }

. {
  printfDebug(" %c", yytext[0]);
  cpyStr(&(yylval.ustr), yytext); 
  return CHAR; 
}

%%
/* empty */
