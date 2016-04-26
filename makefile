tploaderParser: parser.y scanner.l callback.cpp bean.cpp handler.cpp utfconv.cpp debug.cpp superReader.cpp multiProc.cpp
	bison -d parser.y
	flex scanner.l
	g++ -g -o parser callback.cpp bean.cpp handler.cpp utfconv.cpp debug.cpp superReader.cpp multiProc.cpp lex.yy.c parser.tab.c -lfl
