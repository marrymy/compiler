%{
#include "node.cpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <string>
// stuff from flex that bison needs to know about:

	extern int yylex();
	extern char* yytext;

	void yyerror (char const *);

long htol(std::string);	// hex from input char* to long long
int find(const std::string& name);
void test(std::string str);
bool findError(int);
bool check(std::string str);

	IdentifierList::const_iterator getiden;

ErrorList errorlist;
NBlock *programBlock; /* the top level root node of our final AST */

IdentifierList stack_symbol;
IdentifierList all_stack_symbol;

char **strList = NULL;
int count = 0;
int temp_num = 0;
int label_loop = 0;
int label_if = 0;

int last_label_if = 0;

int max_temp = 0;

extern int line;

void add_temp(int);
%}

/* Represents the many different ways we can access our data */

%union {
	NExpression *exp;
	NIdentifier *iden;
	NStatement *statement;
	NCompareOperator *compare;
	NBlock *block;
	NIfBlock *ifblock;
	std::string *string;
	int token;
}

%type <statement> statement else
%type <exp> exp
%type <iden> iden
%type <block> program stmts
%type <compare> compare
%type <ifblock> if else_if


/* BISON Declarations */

//operand token
%token <token> LONG
%token <string> NUM
%token <string> HEX
//%token <string> MSG
%token <string> IDENTIFIER
%token <token> UNKNOW
//command token
%token <token> IF
%token <token> ELSE
%token <token> WHILE
%token <token> FOR
%token <token> LOOP
%token <token> showHex
%token <token> showDec
%token <token> show
//operator token
%left <token> '-' '+'
%left <token> '*' '/' '%'
%right NEG     /* negation--unary minus */

%left <token> CP_NOT_EQ
%left <token> CP_EQ
%left <token> CP_GE
%left <token> CP_G
%left <token> CP_LE
%left <token> CP_L



/* Grammar follows */
%%

program : stmts { programBlock = $1; }
;

stmts : statement {temp_num = 0; $$ = new NBlock(); $$->statements.push_back($1); }
| stmts statement { temp_num = 0; $1->statements.push_back($2);}
;

//expression in each line
statement:     error															{
	if(!findError(line))errorlist.push_back(new codeError(Unknow,line));
	$$ = new NErrorStatement(Unknow,line);
}
|LONG iden ';'													{
	if(find($2->name)==-1){
		stack_symbol.push_back($2);
		all_stack_symbol.push_back($2);
		$$ = new NLongDeclaration(*$2);
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(sameDeclar,line));
		$$ = new NErrorStatement(sameDeclar,line);
	}
}

| LONG iden '=' exp ';'											{
	if(find($2->name)==-1){
		$2->valid = true;
		stack_symbol.push_back($2);
		all_stack_symbol.push_back($2);
		$$ = new NLongDeclaration(*$2,$4);
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(sameDeclar,line));
		$$ = new NErrorStatement(sameDeclar,line);
	}
}

| LONG iden error												{
	if(!findError(line))errorlist.push_back(new codeError(declareError,line));
	$$ = new NErrorStatement(declareError,line)
}

| iden '=' exp ';'												{
	int t = find($1->name);
	if(t!=-1) {
		getiden = stack_symbol.begin()+t;
		(**getiden).valid = true;
		$$ = new NAssignment((**getiden),*$3);
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(unDeclar,line));
		$$ = new NErrorStatement(unDeclar,line);
	}
}

| iden error													{
	if(!findError(line))errorlist.push_back(new codeError(assignError,line));
	$$ = new NErrorStatement(assignError,line);
}

| WHILE '(' compare ')' '{' stmts '}'							{
	label_loop+=2;
	$$ = new NWhileStatement(*$3,*$6,label_loop);
}

| WHILE error													{
	if(!findError(line))errorlist.push_back(new codeError(whileError,line));
	$$ = new NErrorStatement(whileError,line);
}

| FOR '(' statement compare ';' iden '=' exp ')' '{' stmts '}'
{
	int t = find($6->name);
	if(t!=-1) {
		label_loop+=2;
		getiden = stack_symbol.begin()+t;
		(**getiden).valid = true;
		NAssignment *tempAssign2 = new NAssignment((**getiden),*$8);
		$$ = new NForStatement(*$3,*$4,*tempAssign2,*$11,label_loop);
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(unDeclar,line));
		$$ = new NErrorStatement(unDeclar,line);
	}
}

| FOR error														{
	if(!findError(line))errorlist.push_back(new codeError(forError,line));
	$$ = new NErrorStatement(forError,line);
}

| LOOP '(' compare ')' '{' stmts '}'							{
	label_loop+=2;
	$$ = new NWhileStatement(*$3,*$6,label_loop);
}

| if															{$$ = $1; last_label_if+=1;}

| showHex iden ';'										{
	int t = find($2->name);
	if(t!=-1) {
		getiden = stack_symbol.begin()+t;
		if((**getiden).valid) $$ = new NFCallStatement(8888,*$2);
		else{
			if(!findError(line))errorlist.push_back(new codeError(notAssign,line));
			$$ = new NErrorStatement(notAssign,line);
		}
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(unDeclar,line));
		$$ = new NErrorStatement(unDeclar,line);
	}
}

| showDec iden ';'										{
	int t = find($2->name);
	if(t!=-1) {
		getiden = stack_symbol.begin()+t;
		if((**getiden).valid) $$ = new NFCallStatement(1111,*$2);
		else{
			if(!findError(line))errorlist.push_back(new codeError(notAssign,line));
			$$ = new NErrorStatement(notAssign,line);
		}
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(unDeclar,line));
		$$ = new NErrorStatement(unDeclar,line);
	}
}
| show iden ';' 	{
	if(check($2->name)){

		if(find($2->name)==-1){
			$2->valid = true;
			stack_symbol.push_back($2);
			all_stack_symbol.push_back($2);
			$$ = new NLongDeclaration(*$2,0);
		}
		else {
			if(!findError(line))errorlist.push_back(new codeError(sameDeclar,line));
			$$ = new NErrorStatement(sameDeclar,line);
		}
	}
	int t = find($2->name);
	if(t!=-1) {
		getiden = stack_symbol.begin()+t;
		if((**getiden).valid) {$$ = new NFCallStatement(9999,*$2);}
		else{
			if(!findError(line))errorlist.push_back(new codeError(notAssign,line));
			$$ = new NErrorStatement(notAssign,line);
		}
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(unDeclar,line));
		$$ = new NErrorStatement(unDeclar,line);
	}
}

;

if: 	IF '(' compare ')' '{' stmts '}'								{
	label_if+=1;
	$$ = new NIfBlock(*$3,*$6,label_if,last_label_if);
}
| if else_if													{ $1->elseStatement.push_back($2);}
| if else														{ $1->elseStatement.push_back($2);}

;


else:	ELSE '{' stmts '}'												{$$ = $3;}
| ELSE error													{
	if(!findError(line))errorlist.push_back(new codeError(elseError,line));
	$$ = new NErrorStatement(elseError,line);
}
;

else_if: ELSE if														{$2->elseif = true;$$ = $2;}
;


iden: IDENTIFIER	{$$ = new NIdentifier(*$1,false)}
;

exp:    NUM              {$$ = new NLong(atol(($1->c_str())))}
| HEX			 {$$ = new NLong(htol((*$1)))}
| IDENTIFIER 		{
	int t = find(*$1);
	if(t!=-1) {
		getiden = stack_symbol.begin()+t;
		if((**getiden).valid) $$ = new NIdentifier(*$1,true);
		else {
			if(!findError(line))errorlist.push_back(new codeError(notAssign,line));
			$$ = new NErrorExpression(notAssign,line);
		}
	}
	else {
		if(!findError(line))errorlist.push_back(new codeError(unDeclar,line));
		$$ = new NErrorExpression(unDeclar,line);
	}
}
| exp '+' exp       			{add_temp(3); $$ = new NBinaryOperator($1,'+',*$3,temp_num)}
| exp '-' exp       			{add_temp(3); $$ = new NBinaryOperator($1,'-',*$3,temp_num)}
| exp '*' exp       			{add_temp(3); $$ = new NBinaryOperator($1,'*',*$3,temp_num)}
| exp '/' exp       			{add_temp(3); $$ = new NBinaryOperator($1,'/',*$3,temp_num)}
| exp '%' exp       			{add_temp(3); $$ = new NBinaryOperator($1,'%',*$3,temp_num)}
| '-' exp  %prec NEG			{add_temp(3); $$ = new NBinaryOperator('-',*$2,temp_num)}
| '(' exp ')'        			{$$ = $2}

;
//compare
compare: exp CP_NOT_EQ exp 				{add_temp(3); $$ = new NCompareOperator(*$1,901,*$3,temp_num)}
|exp CP_EQ exp 				{add_temp(3); $$ = new NCompareOperator(*$1,902,*$3,temp_num)}
|exp CP_GE exp 				{add_temp(3); $$ = new NCompareOperator(*$1,903,*$3,temp_num)}
|exp CP_G exp 				{add_temp(3); $$ = new NCompareOperator(*$1,904,*$3,temp_num)}
|exp CP_LE exp 				{add_temp(3); $$ = new NCompareOperator(*$1,905,*$3,temp_num)}
|exp CP_L exp 				{add_temp(3); $$ = new NCompareOperator(*$1,906,*$3,temp_num)}
;

%%


/* Called by yyparse on error.  */
void yyerror (char const *s)
{
}


// hex from input char* to long long function
long htol(std::string str){
	std::string::const_iterator temp;
	long decimal=0;
	int i=str.size()-3, rem;

	while(i>-1){
		temp = str.begin()+2+str.size()-3-i;
		if(*temp=='f') rem=15;
		else if(*temp=='e') rem=14;
		else if(*temp=='d') rem=13;
		else if(*temp=='c') rem=12;
		else if(*temp=='b') rem=11;
		else if(*temp=='a') rem=10;
		else if(isdigit(*temp)) rem = *temp-'0';

		decimal += rem*pow(16,i);
		i--;
	}
	return decimal;
}
void test(std::string str)//$r in stack
{
	std::cout << str;
}
bool check(std::string str)
{
	int i;
   	for( i = 0 ; i < count ; i++)
   		if(!strcmp(str.c_str(),strList[i]))
   			return false;
	strList = (char**)realloc(strList,((count++)+1)*sizeof(*strList));
	if (strList==NULL)
       { puts ("Error (re)allocating memory"); exit (1); }
   	strList[count-1] = (char*)malloc(129 * sizeof(char*));
   	strcpy(strList[count-1], str.c_str());
   	strList[count-1][str.length()] = '\0';
   	
	return true;
}
int find(const std::string& name)//$r in stack
{
	for (int i=0;i<stack_symbol.size();i++) { 
		//printf("name : %s i : %d", name.c_str(),i);
		getiden = stack_symbol.begin()+i;
		//printf("(**getiden).name : %s",(**getiden).name.c_str());
		if((**getiden).name.compare(name)==0) return i;
	}
	//printf("\n");
	return -1;
}

bool findError(int line){
	ErrorList::const_iterator eit;
	for (eit = errorlist.begin(); eit != errorlist.end(); eit++) {
		if((**eit).line == line) return true;
	}
	return false;
}

void add_temp(int i){
	temp_num += i;
	if(max_temp<temp_num) max_temp = temp_num;
}

