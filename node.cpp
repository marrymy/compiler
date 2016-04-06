#include <iostream>
#include <vector>
#include <stdio.h>
#include "code.cpp"

class NStatement;
class NIdentifier;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NIdentifier*> IdentifierList;


extern codeList IRcode;

class NExpression {
public:
	virtual std::string cgen() {return "unknow";}
};

class NStatement {
public:
	virtual void cgen() { std::cout<<"error"<<"\n"; };
};


class NLong : public NExpression {
public:
    long value;
    NLong(long value) : value(value) { }
	std::string  cgen() {
	    char buffer[21];
        sprintf(buffer,"%lu", value);
        return buffer;
	}
};

class NIdentifier : public NExpression {
public:
    std::string name;
	bool valid;
    NIdentifier(const std::string& name,bool valid) : name(name), valid(valid) { }
	std::string cgen(){
	    return name; }
};

class NBinaryOperator : public NExpression {
public:
    NExpression* lhs;
    int op;
    NExpression& rhs;
	int temp_num;
    NBinaryOperator(NExpression* lhs, int op, NExpression& rhs, int temp_num) :
        lhs(lhs), op(op), rhs(rhs), temp_num(temp_num) { }
	NBinaryOperator(int op, NExpression& rhs, int temp_num) :
        lhs(NULL), op(op), rhs(rhs), temp_num(temp_num) { }
	std::string  cgen() {
		char buffer1[6];
		char buffer2[6];
		char buffer3[6];
        sprintf(buffer1,"_t%d", temp_num-2);
		sprintf(buffer2,"_t%d", temp_num-1);
		sprintf(buffer3,"_t%d", temp_num);
		if(lhs==NULL) IRcode.push_back(new codeOperand(buffer1,"0"));
		else IRcode.push_back(new codeOperand(buffer1,lhs->cgen()));
		IRcode.push_back(new codeOperand(buffer2,rhs.cgen()));
		IRcode.push_back(new codeOperation(buffer3,buffer1,op,buffer2));
        return buffer3;
	}
};

class NCompareOperator : public NExpression {
public:
    NExpression& lhs;
    int op;
    NExpression& rhs;
    int temp_num;
    NCompareOperator(NExpression& lhs, int op, NExpression& rhs, int temp_num) :
        lhs(lhs), op(op), rhs(rhs),temp_num(temp_num){ }
	std::string  cgen() {
		char buffer1[6];
		char buffer2[6];
		char buffer3[6];
        sprintf(buffer1,"_t%d", temp_num-2);
		sprintf(buffer2,"_t%d", temp_num-1);
		sprintf(buffer3,"_t%d", temp_num);
		IRcode.push_back(new codeOperand(buffer1,lhs.cgen()));
		IRcode.push_back(new codeOperand(buffer2,rhs.cgen()));
		IRcode.push_back(new codeOperation(buffer3,buffer1,op,buffer2));
        return buffer3;
	}
};






class NBlock : public NStatement {
public:
    StatementList statements;
    NBlock() { }
	void cgen(){
		StatementList::const_iterator it;
		for (it = statements.begin(); it != statements.end(); it++) {
			(**it).cgen();
		}
	}
};

class NAssignment : public NStatement {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) :
        lhs(lhs), rhs(rhs) { }
	void cgen() {
		IRcode.push_back(new codeOperand(lhs.cgen(),rhs.cgen()));
	}
};

class NLongDeclaration : public NStatement {
public:
    NIdentifier& id;
    NExpression* assignmentExpr;
    NLongDeclaration(NIdentifier& id) :
        id(id),	assignmentExpr(NULL) { }
    NLongDeclaration(NIdentifier& id, NExpression* assignmentExpr) :
        id(id), assignmentExpr(assignmentExpr) { }
	void cgen(){
		if(assignmentExpr!=NULL) IRcode.push_back(new codeOperand(id.cgen(),assignmentExpr->cgen()));
	};
};

class NIfBlock : public NStatement {
public:
	NCompareOperator compare;
    NBlock block;
	StatementList elseStatement;
	bool elseif = false;
	int label_num;
	int last_label_num;
    NIfBlock(NCompareOperator& cp, NBlock& bl, int ln, int lln) :
        compare(cp),block(bl) {label_num = ln; last_label_num= lln;}
	void cgen(){
		bool haveelse = elseStatement.size()>0;
		
		char label_c[6];
        sprintf(label_c,"_IF%d", label_num);
		char label_lc[10];
        sprintf(label_lc,"_LastIF%d", last_label_num);
		
		if(haveelse) IRcode.push_back(new codeIfZ(compare.cgen(),label_c));
		else IRcode.push_back(new codeIfZ(compare.cgen(),label_lc));

		block.cgen();
		if(haveelse) {
			IRcode.push_back(new codeGoto(label_lc));
			IRcode.push_back(new codeLabel(label_c));
			(**(elseStatement.begin())).cgen();
		}
		if(!elseif) IRcode.push_back(new codeLabel(label_lc));
	}
};


class NWhileStatement : public NStatement {
public:
	NCompareOperator& compare;
    NBlock& block;
	int label_num;
    NWhileStatement(NCompareOperator& compare, NBlock& block, int label_num):
		compare(compare), block(block), label_num(label_num) {};
	void cgen(){
	
		char label1[5];
        sprintf(label1,"_L%d", label_num-1);
		char label2[5];
        sprintf(label2,"_LastIF%d", label_num);
	
		IRcode.push_back(new codeLabel(label1));
		IRcode.push_back(new codeIfZ(compare.cgen(),label2));
		block.cgen();
		IRcode.push_back(new codeGoto(label1));
		IRcode.push_back(new codeLabel(label2));
	};
};

class NForStatement : public NStatement {
public:
	NStatement& bf_stmt;
	NCompareOperator& compare;
	NAssignment& af_stmt;
    NBlock& block;
	int label_num;
    NForStatement(NStatement& bf_stmt, NCompareOperator& compare, NAssignment& af_stmt, NBlock& block, int label_num):
		bf_stmt(bf_stmt), compare(compare), af_stmt(af_stmt), block(block), label_num(label_num) {};
	void cgen(){
	
		char label1[5];
        sprintf(label1,"_L%d", label_num-1);
		char label2[5];
        sprintf(label2,"_LastIF%d", label_num);
	
		bf_stmt.cgen();
		IRcode.push_back(new codeLabel(label1));
		IRcode.push_back(new codeIfZ(compare.cgen(),label2));
		block.cgen();
		af_stmt.cgen();
		IRcode.push_back(new codeGoto(label1));
		IRcode.push_back(new codeLabel(label2));
	};
};

class NFCallStatement : public NStatement {
public:
	int type;
	NIdentifier& id;
    NFCallStatement(int type, NIdentifier& id):
		type(type), id(id) {};
	void cgen(){
		IRcode.push_back(new codeFCall(type,id.name));
	};
};

class NErrorExpression : public NExpression {
public:
    int code;
	int line;
    NErrorExpression(int code, int line)
    :
		code(code), line(line) {};
	std::string cgen(){
		return "error";
	};
};

class NErrorStatement : public NStatement {
public:
    int code;
	int line;
    NErrorStatement(int code, int line):
		code(code), line(line) {};
	void cgen(){};
};
