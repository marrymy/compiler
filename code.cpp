#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

class code;
class codeError;

typedef std::vector<code*> codeList;
typedef std::vector<codeError*> ErrorList;

extern bool is_digits(const std::string &str);

extern int countFcall;

enum{unDeclar=1, notAssign ,Unknow, declareError, whileError, forError, ifError, elseError, assignError, sameDeclar};

class code{
public:
	virtual void print() {std::cout<<"unknow\n";}
	virtual void writeNASM(FILE* fp) {std::cout<<"unknow NASM\n";}
};

class codeOperand : public code {
public:
	std::string lhs;
	std::string rhs;
	codeOperand(std::string lhs, std::string rhs): lhs(lhs), rhs(rhs) {}
	void print() {std::cout<<"\t"<<lhs<<" = "<<rhs<<"\n";}
	void writeNASM(FILE* fp) {
		fprintf(fp,"\tmov\t eax,");
		if(is_digits(rhs))fprintf(fp, "%s\n",rhs.c_str());
		else fprintf(fp, "[%s]\n",rhs.c_str());

		fprintf(fp, "\tmov\t[%s],eax\n",lhs.c_str());

	}
};

class codeOperation : public code {
public:
	std::string lhs;
	std::string rhs_lop;
	int rhs_op;
	std::string rhs_rop;
	codeOperation(std::string lhs, std::string rhs_lop, int rhs_op, std::string rhs_rop): lhs(lhs), rhs_lop(rhs_lop), rhs_op(rhs_op), rhs_rop(rhs_rop) {}
	void print() {
		std::cout<<"\t"<<lhs<<" = "<<rhs_lop<<" ";

		if(rhs_op==901)std::cout<<"!=";
		else if(rhs_op==902)std::cout<<"==";
		else if(rhs_op==903)std::cout<<">=";
		else if(rhs_op==904)std::cout<<">";
		else if(rhs_op==905)std::cout<<"<=";
		else if(rhs_op==906)std::cout<<"<";
		else std::cout<<(char)rhs_op;

		std::cout<<" "<<rhs_rop<<"\n";
	}
	void writeNASM(FILE* fp) {
		fprintf(fp, "\tmov\teax,");
		fprintf(fp, "[%s]\n",rhs_lop.c_str());

		if(rhs_op=='*'){
			fprintf(fp, "\tmov\tebx,[%s]\n",rhs_rop.c_str());
			fprintf(fp, "\tmul\tebx\n");
			fprintf(fp, "\tmov\t[%s],eax\n",lhs.c_str());
		}
		else if(rhs_op=='/'){
			fprintf(fp, "\tmov\tedx,0\n");
			fprintf(fp, "\tidiv\tdword [%s]\n",rhs_rop.c_str());
			fprintf(fp, "\tmov\t[%s],eax\n",lhs.c_str());
		}
		else if(rhs_op=='%'){
			fprintf(fp, "\tmov\tedx,0\n");
			fprintf(fp, "\tidiv\tdword [%s]\n",rhs_rop.c_str());
			fprintf(fp, "\timul\tdword [%s]\n",rhs_rop.c_str());

			fprintf(fp, "\tmov\tebx,[%s]\n",rhs_lop.c_str());
			fprintf(fp, "\tsub\tebx,eax\n");
			fprintf(fp, "\tmov\t[%s],ebx\n",lhs.c_str());
		}
		else if(rhs_op=='+'){
			fprintf(fp, "\tadd\teax,[%s]\n",rhs_rop.c_str());
			fprintf(fp, "\tmov\t[%s],eax\n",lhs.c_str());
		}
		else if(rhs_op=='-'){
			fprintf(fp, "\tsub\teax,[%s]\n",rhs_rop.c_str());
			fprintf(fp, "\tmov\t[%s],eax\n",lhs.c_str());
		}
		else if(rhs_op>900){
			fprintf(fp, "\tmov\tebx,[%s]\n",rhs_rop.c_str());

			fprintf(fp, "\tcmp\teax,ebx\n");

			if(rhs_op==901) fprintf(fp, "\tje\t");
			else if(rhs_op==902) fprintf(fp, "\tjne\t");
			else if(rhs_op==903) fprintf(fp, "\tjl\t");
			else if(rhs_op==904) fprintf(fp, "\tjle\t");
			else if(rhs_op==905) fprintf(fp, "\tjg\t");
			else if(rhs_op==906) fprintf(fp, "\tjge\t");
			else fprintf(fp, "\tunknow comparator GO TO");


		}
		else fprintf(fp, "\tunknow operator\n");
	}
};

class codeIfZ : public code {
public:
	std::string var;
	std::string label;
	codeIfZ(std::string var, std::string label): var(var), label(label) {}
	void print() {std::cout<<"\tIfZ "<<var<<" GO TO "<<label<<"\n";}
	void writeNASM(FILE* fp) {
		fprintf(fp, "%s\n",label.c_str());
	}
};

class codeGoto : public code {
public:
	std::string label;
	codeGoto(std::string label): label(label) {}
	void print() {std::cout<<"\tGO TO "<<label<<"\n";}
	void writeNASM(FILE* fp) {
		fprintf(fp, "\tjmp\t%s\n",label.c_str());
	}
};

class codeLabel : public code {
public:
	std::string label;
	codeLabel(std::string label): label(label) {}
	void print() {std::cout<<label<<":\n";}
	void writeNASM(FILE* fp) {
		fprintf(fp, "%s:\n",label.c_str());
	}
};

class codeFCall : public code {
public:
	int type;
	std::string id;
    codeFCall(int type, std::string id): type(type), id(id) {};
	void print() {
		std::cout<<"\tcall: "<<id<<"  type: ";
		if(type == 1111) std::cout<<"SHOW DEC";
		else if(type == 8888) std::cout<<"SHOW HEX";
		std::cout<<"\n";
	}
	void writeNASM(FILE* fp) {
		fprintf(fp, "\tpush\tdword [%s]\n",id.c_str());
		fprintf(fp, "\tpushIden\t\"%s\"\n",id.c_str());
		if(type == 1111) fprintf(fp, "\tpush\tshD\n");
		else if(type == 8888) fprintf(fp, "\tpush\tshH\n");
		fprintf(fp, "\tcall\t_printf\n");
		fprintf(fp, "\tadd\tesp,12\n");
		fprintf(fp, "_FCall%d:\n",countFcall++);
	}
};

class codeError : public code {
public:
	int code;
	int line;
	codeError(int code, int line): code(code), line(line) {}
	void print() {

		std::cout<<"!ERROR!("<<code<<"):";

		switch(code){

            case unDeclar:      std::cout<<"variable is not declare";   	break;
            case notAssign:     std::cout<<"variable is not assign";    	break;
            case Unknow:        std::cout<<"unknow syntax";             	break;
			case declareError:  std::cout<<"declartion syntax error";  	break;
            case assignError:   std::cout<<"assignment syntax error";   	break;
            case whileError:    std::cout<<"while syntax error";        	break;
            case forError:      std::cout<<"for syntax error";          	break;
            case ifError:       std::cout<<"if syntax error";           	break;
            case elseError:     std::cout<<"else syntax error";      	break;
			case sameDeclar:	std::cout<<"already declare this variable";      	break;
            default:            std::cout<<"unknow syntax";
		}

		std::cout<<"---at line "<<line<<":\n";
	}
};


