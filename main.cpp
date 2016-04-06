#include <iostream>
#include <stdio.h>
#include <string.h>
#include "node.cpp"

extern NBlock* programBlock;
extern IdentifierList stack_symbol;
extern IdentifierList all_stack_symbol;

extern ErrorList errorlist;

codeList IRcode;

extern FILE* yyin;
extern int yyparse();

extern int max_temp;

extern int line;

int countFcall = 0;

void printIR();
void printD();
void writeStartNASM(FILE*);
void writeEndNASM(FILE*);

int main(int argc, char **argv)
{
	++argv, --argc;  /* skip over program name */
	if ( argc > 0 ) {
		yyin = fopen( argv[0], "r" );
		if(yyin){
			std::cout<<"open:"<<argv[0]<<"\n";
			do {
				yyparse();
			} while (!feof(yyin));

			if(errorlist.size()==0){
				programBlock->cgen();

				if(argc==2){
					if(strcmp(argv[1],"-IR")==0) printIR();
					if(strcmp(argv[1],"-D")==0) printD();
				}

				if(argc==3){
					if(strcmp(argv[1],"-IR")==0||strcmp(argv[2],"-IR")==0) printIR();
					if(strcmp(argv[1],"-D")==0||strcmp(argv[2],"-D")==0) printD();
				}

				FILE *foutput;
				foutput = fopen("output.asm","w");

				writeStartNASM(foutput);

				codeList::const_iterator cit;
				for (cit = IRcode.begin(); cit != IRcode.end(); cit++) {
					(**cit).writeNASM(foutput);
				}

				writeEndNASM(foutput);

				fclose(foutput);

			}else {
				ErrorList::const_iterator eit;
				for (eit = errorlist.begin(); eit != errorlist.end(); eit++) {
					(**eit).print();
				}
			}
		}
		else std::cout<<"!ERROR! can't open:"<<argv[0]<<"\n";
	}
	else std::cout<<"!ERROR! please insert input\n\n";
	return 0;
}

void printIR(){
	std::cout<<"\n------------------------\nIR CODE\n------------------------\n";
	codeList::const_iterator cit;
	for (cit = IRcode.begin(); cit != IRcode.end(); cit++) {
		(**cit).print();
	}
	std::cout<<"----------------------------------------------------------\n";
}

void printD(){
	std::cout<<"\n\n------------------------\nidenlist\n------------------------\n";
	IdentifierList::const_iterator it;
	for (it = stack_symbol.begin(); it != stack_symbol.end(); it++) {
		std::cout<<"declare : "<<(**it).cgen()<<"\n";
	}

	std::cout<<"\n\n------------------------\nALL-idenlist\n------------------------\n";
	for (it = all_stack_symbol.begin(); it != all_stack_symbol.end(); it++) {
		std::cout<<"allocate : "<<(**it).cgen()<<"\n";
	}

	std::cout<<"\n\n------------------------\nAttribute\n------------------------\n";
	std::cout<<"max _t = "<<max_temp<<"\n";
	std::cout<<"line count = "<<line<<"\n";
}

void writeStartNASM(FILE* fp){
	fprintf(fp, "\tglobal  main\n\textern  _printf\n");
	
	
	fprintf(fp, "\n%cmacro pushIden 1\n",(char)37);
	fprintf(fp, "section .data\n");
	fprintf(fp, "\t.str\tdb	%c1,0\n",(char)37);
	fprintf(fp, "section .text\n");
	fprintf(fp, "\tpush\tdword .str\n");
	fprintf(fp, "%cendmacro\n\n",(char)37);

	fprintf(fp, "\nsegment .data\n");
	
	for(int i=1;i<=max_temp;i++){
		fprintf(fp, "\t_t%d:\tdd 0\n",i);
	}

	IdentifierList::const_iterator it;
	for (it = all_stack_symbol.begin(); it != all_stack_symbol.end(); it++) {
		fprintf(fp, "\t%s:\tdd 0\n",(**it).name.c_str());
	}

	fprintf(fp, "\tshD:\tdb \"show %cs: %cld\",10,0\n",(char)37,(char)37);
	fprintf(fp, "\tshH:\tdb \"show %cs: %clx\",10,0\n",(char)37,(char)37);

	fprintf(fp, "\nsection .text\n");
	fprintf(fp, "\tglobal _WinMain@16\n");
	fprintf(fp, "\n_WinMain@16:\n\n");
}

void writeEndNASM(FILE* fp){
	fprintf(fp, "\n\tmov\teax,0\n");
	fprintf(fp, "\tret\n");
}

bool is_digits(const std::string &str)
{
	return str.find_first_not_of("0123456789") == std::string::npos;
}

