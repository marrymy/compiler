%option noyywrap

%{
	#include <string>
	#include "node.cpp"
	#include "parser.cpp.h"

	#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)

extern IdentifierList stack_symbol;

int line = 1;

%}

%%

"+"						{ return '+'; }
"*"						{ return '*'; }
"/"						{ return '/'; }
"%"						{ return '%'; }
"-"						{ return '-'; }
"="						{ return '='; }

"!="					{ return CP_NOT_EQ; }
"=="					{ return CP_EQ; }
">="					{ return CP_GE; }
">"						{ return CP_G; }
"<="					{ return CP_LE; }
"<"						{ return CP_L; }

"("						{ return '('; }
")"						{ return ')'; }
"{"						{ 
							stack_symbol.push_back(new NIdentifier("0",false));
							return '{';
						}
						"}"						{ 
							IdentifierList::const_iterator it;
							it = stack_symbol.end()-1;					
							while((**it).name.compare("0")!=0){
								stack_symbol.pop_back();
								it = stack_symbol.end()-1;
							}
							stack_symbol.pop_back();
							return '}'; 
						}

"long"					{ return LONG; }
"for"					{ return FOR; }
"while"					{ return WHILE; }
"loop"					{ return LOOP; }
"if"					{ return IF; }
"else"					{ return ELSE; }

"showHex"				{ return showHex; }
"showDec"				{ return showDec; }
$r2[0-6]  { SAVE_TOKEN; return IDENTIFIER;  }
$r1[0-9]  { SAVE_TOKEN; return IDENTIFIER;  }
$r[1-9]  { SAVE_TOKEN; return IDENTIFIER;  }
0x[0-9a-f]+ 			{ SAVE_TOKEN; return HEX; }
[0-9]+ 					{ SAVE_TOKEN; return NUM; }

";" 					{ return ';';}
[\n]					{ line++; }
[ \t\v\f]				{  }

.						{ return UNKNOW;}

%%