#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <cstdlib>
using namespace std;

// Symbol Table pair<type(INT),ID>
map <string, string> idTable;
map <string, int> locID;
// The set of terminal symbols in the WLPP grammar.
const char *terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID",
  "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM",
  "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI",
  "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW",
  "DELETE", "NULL"
};
int isTerminal(const string &sym) {
  int idx;
  for(idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
    if(terminals[idx] == sym) return 1;
  return 0;
}

// Data structure for storing the parse tree.
class tree {
  public:
    string rule;
    vector<string> tokens;
    vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};
void printTree(tree *t){
  cout<<"rule = "<<t->rule<<endl;
  vector<string>::iterator it;
  cout<<"tokens = ";
  for(it = t->tokens.begin(); it != t->tokens.end(); it++){
    cout<<(*it)<<" ";
  }
  cout<<endl;
  if(t->children.size() == 0){
    cout<<"END"<<endl;
  }
  else{
    vector<tree*>::iterator lol;
    for(lol = t->children.begin() ; lol!=t->children.end(); lol++){
      printTree(*lol);
    }
  }
}
// Call this to display an error message and exit the program.
void bail(const string &msg) {
  // You can also simply throw a string instead of using this function.
  throw string(msg);
}

// Read and return wlppi parse tree.
tree *readParse(const string &lhs) {
  // Read a line from standard input.
  string line;
  getline(cin, line);
  if(cin.fail())
    bail("ERROR: Unexpected end of file.");
  tree *ret = new tree();
  // Tokenize the line.
  stringstream ss;
  ss << line;
  while(!ss.eof()) {
    string token;
    ss >> token;
    if(token == "") continue;
    ret->tokens.push_back(token);
  }
  // Ensure that the rule is separated by single spaces.
  for(int idx=0; idx<ret->tokens.size(); idx++) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }
  
  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
      ret->children.push_back(readParse(ret->tokens[idx]));

    }
  }
  return ret;
}

tree *parseTree;
string getType(tree *t){
  string type = "";
  
  if(t->rule.compare("type INT") == 0){
    type = "int";
  }
  else if(t->rule.compare("type INT STAR") == 0){
    type = "int*";
  }
  else if(t->rule.compare("factor ID") == 0){
    string IDt = t->children[0]->tokens[1];
    if(idTable.count(IDt) == 0) bail("ERROR: no ID");
    else{
      map<string,string>::iterator it = idTable.find(IDt);
      type = (*it).second;
    }
  }
  else if(t->rule.compare("factor NUM") == 0) type = "int";
  else if(t->rule.compare("factor NULL") == 0) type = "int*";
  else if(t->rule.compare("factor LPAREN expr RPAREN") == 0){
    string et = getType(t->children[1]);
    type = et;
  }
  else if(t->rule.compare("factor AMP lvalue") == 0){
    string lt = getType(t->children[1]);
    if(lt.compare("int") == 0) type = "int*";
    else bail("ERROR: type error ft AMP lv");
  }
  else if(t->rule.compare("factor STAR factor") == 0){
    string ft = getType(t->children[1]);
    if(ft.compare("int*") == 0) type = "int";
    else bail("ERROR: type error ft STAR ft");
  }
  else if(t->rule.compare("factor NEW INT LBRACK expr RBRACK") == 0){
    string et = getType(t->children[3]);
    if(et.compare("int") == 0) type = "int*";
    else bail("ERROR: type error ft NEW exp");
  }
  else if(t->rule.compare("lvalue ID") == 0){
    string key = t->children[0]->tokens[1];
    if(idTable.count(key) == 0) bail("ERROR: No such ID");
    else{
      map<string,string>::iterator it = idTable.find(key);
      type = (*it).second;
    }
  }
  else if(t->rule.compare("lvalue STAR factor") == 0){
    string ft = getType(t->children[1]);
    if(ft.compare("int*") == 0) type = "int";
    else bail("ERROR: type error lv STAR ft");
  }
  else if(t->rule.compare("lvalue LPAREN lvalue RPAREN") == 0){
    string lt = getType(t->children[1]);
    type = lt;
  }
  else if(t->rule.compare("expr term") == 0){
    string tt = getType(t->children[0]);
    type = tt;
  }
  else if(t->rule.compare("expr expr PLUS term") == 0){
    string et = getType(t->children[0]), tt = getType(t->children[2]);
    if(et == "int" && tt == "int"){
      type = "int";
    }
    else if(et == "int" && tt == "int*"){
      type = "int*";
    }
    else if(et == "int*" && tt == "int"){
      type = "int*";
    } 
    else bail("ERROR: type error exp exp + tm");
  }
  else if(t->rule.compare("expr expr MINUS term") == 0){
    string et = getType(t->children[0]), tt = getType(t->children[2]);
    if(et == "int" && tt == "int"){
      type = "int";
    }
    else if(et == "int*" && tt == "int"){
      type = "int*";
    }
    else if(et == "int*" && tt == "int*"){
      type = "int";
    }
    else bail("ERROR: type error exp exp - tm");

  }
  else if(t->rule.compare("term factor") == 0){
    type = getType(t->children[0]);
  }
  else if(t->rule.compare("term term STAR factor") == 0){
    string tt = getType(t->children[0]), ft = getType(t->children[2]);
    if(tt == "int" && ft == "int"){
      type = "int";
    }
    else bail("ERROR: type error tm tm STAR ft");
  }
  else if(t->rule.compare("term term SLASH factor") == 0){
    string tt = getType(t->children[0]), ft = getType(t->children[2]);
    if(tt == "int" && ft == "int") type = "int";
    else bail("ERROR: type error tm tm SLASH ft");
  }
  else if(t->rule.compare("term term PCT factor") == 0){
    string tt = getType(t->children[0]), ft = getType(t->children[2]);
    if(tt == "int" && ft == "int") type = "int";
    else bail("ERROR: type error tm tm PCT ft");
  }
  else if(t->rule.compare("dcls dcls dcl BECOMES NUM SEMI") == 0){
    string dt1 = getType(t->children[0]), dt2 = getType(t->children[1]);
    if(dt1 == "declare_true" && dt2 == "int") type = "declare_true";
    else bail("ERROR: type error dcls dcls dcl = num");
  }
  else if(t->rule.compare("dcls dcls dcl BECOMES NULL SEMI") == 0){
    string dt1 = getType(t->children[0]), dt2 = getType(t->children[1]);
    if(dt1 == "declare_true" && dt2 == "int*") type = "declare_true";
    else bail("ERROR: type error dcls dcls dcl = null pointer");
  }
  else if(t->rule.compare("dcl type ID") == 0){
    type = getType(t->children[0]);
  }
  else if(t->tokens[0] == "test"){
    string tt1 = getType(t->children[0]), tt2 = getType(t->children[2]);
    if(tt1 != tt2) bail("ERROR: type error test"); 
    else type = "test_true";
  }
  else if(t->rule.compare("statements statements statement") == 0){
    string st1 = getType(t->children[0]), st2 = getType(t->children[1]);
    if(st1 == "statements_true" && st2 == "statements_true") type = "statements_true";
    else bail("ERROR: type error sts sts st");
  }
  else if(t->rule.compare("statement lvalue BECOMES expr SEMI") == 0){
    string lt = getType(t->children[0]), et = getType(t->children[2]);
    if(lt != et) bail("ERROR: type error st lv = exp");
    else type = "statements_true";
  }
  else if(t->rule.compare("statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") == 0){
    string tet = getType(t->children[2]), st1 = getType(t->children[5]), st2 = getType(t->children[9]);
    if(tet == "test_true" && st1 == "statements_true" && st2 == "statements_true") type = "stataments_true";
    else bail("ERROR: type error");
  }
  else if(t->rule.compare("statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") == 0){
    string tet = getType(t->children[2]), st = getType(t->children[5]);
    if(tet == "test_true" && st == "statements_true") type = "statements_true";
    else bail("ERROR: type error");
  }
  else if(t->rule.compare("statement PRINTLN LPAREN expr RPAREN SEMI") == 0){
    string et = getType(t->children[2]);
    if(et == "int") type = "statements_true";
    else bail("ERROR: type error");
  }
  else if(t->rule.compare("statement DELETE LBRACK RBRACK expr SEMI") == 0){
    string et = getType(t->children[3]);
    if(et != "int*") bail("ERROR: type error");
    else type = "statements_true";
  }
  else if(t->rule.compare("statements") == 0) type = "statements_true";
  else if(t->rule.compare("dcls") == 0) type = "declare_true";
  else{
    bail("ERROR: unknown error");
  }

  return type;
}
string getLex(tree *t){
  if(t->tokens[0].compare("ID") == 0){
    return t->tokens[1];
  }
}
// Compute symbols defined in t.
int idCount = 0, isRETURN = 0, tempoffset = 0;
void genSymbols(tree *t) {
  tree* temp = t;
 
  
  if(temp->children.size() == 0 && temp->tokens[0].compare("RETURN") != 0){
    return;
  }
  else if(temp->tokens[0].compare("dcl") == 0){
    string type = getType(temp->children[0]), content = getLex(temp->children[1]);
    if(idTable.find(content) != idTable.end()) bail("ERROR: Redundant ID declaration");
    else{ 
      if(idCount != 2){
	idTable.insert(pair<string, string>(content, type));
	locID.insert(pair<string, int>(content, tempoffset));
	tempoffset+=4;
      }
      else if(idCount == 2){
	if(type != "int") bail("ERROR: incorrect procedure format");
	else{ 
	  idTable.insert( pair<string, string>(content, type) );
	  locID.insert(pair<string,int>(content, tempoffset));
	  tempoffset+=4;
	}
      }
    }
  }
  else if(temp->tokens[0].compare("dcls") == 0){
    getType(temp);
    vector<tree*>::iterator it;
    for(it = temp->children.begin(); it != temp->children.end(); it++){
      genSymbols(*it);
    }

  }
  else if(temp->tokens[0].compare("expr") == 0){
    if(isRETURN == 0){
      getType(temp);
      vector<tree*>::iterator it;
      for(it = temp->children.begin(); it != temp->children.end(); it++){
	genSymbols(*it);
      }
    }
    else{
      /*  if(getType(temp) != "int") bail("ERROR: incorrect procedure format");
	  else {*/
	vector<tree*>::iterator it;
	for(it = temp->children.begin(); it != temp->children.end(); it++){
	  genSymbols(*it);
	}

	/* }*/
    }
  }
  else if(temp->tokens[0].compare("lvalue") == 0){
    getType(temp);
    vector<tree*>::iterator it;
    for(it = temp->children.begin(); it != temp->children.end(); it++){
      genSymbols(*it);
    }

  }
  else if(temp->tokens[0].compare("statements") == 0){
    getType(temp);
    vector<tree*>::iterator it;
    for(it = temp->children.begin(); it != temp->children.end(); it++){
      genSymbols(*it);
    }

  }
  else if(temp->tokens[0].compare("test") == 0){
    getType(temp);
    vector<tree*>::iterator it;
    for(it = temp->children.begin(); it != temp->children.end(); it++){
      genSymbols(*it);
    }

  }
  else if(temp->tokens[0].compare("RETURN") == 0){
    isRETURN = 1;
    return;
  }
  else{
    vector<tree*>::iterator it;
    for(it = temp->children.begin(); it != temp->children.end(); it++){
      genSymbols(*it);
    }
  }
}

string para1, para2; 
int i,para = 0, numID = 0;

// Generate the code for the parse tree t.
void genCode(tree *t) {
  srand(time(NULL));
  if(t->rule == "procedure INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"){
    cout<<".import print"<<endl;
    cout<<".import init"<<endl;
    cout<<".import new"<<endl;
    cout<<".import delete"<<endl;
    cout<<"lis $4"<<endl;
    cout<<".word 4"<<endl;
    cout<<"lis $11"<<endl;
    cout<<".word 1"<<endl;
    cout<<"sw $31,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    numID = idTable.size();
    for(i=0;i<numID;i++){
      cout<<"sub $30,$30,$4"<<endl;
    }
    cout<<"add $29,$0,$30"<<endl;
    cout<<"sw $1,0($29)"<<endl;
    cout<<"sw $2,4($29)"<<endl;
    // procedure -> INT WAIN dcls ...

    genCode(t->children[3]);
    string dt = getType(t->children[3]);
    if(dt == "int"){
      cout<<"add $2,$0,$0"<<endl;
    }
    cout<<"lis $3"<<endl;
    cout<<".word init"<<endl;
    cout<<"jalr $3"<<endl;
    genCode(t->children[5]);
    genCode(t->children[8]);
    genCode(t->children[9]);
    genCode(t->children[11]);

    // junk allocation
    for(i=0;i<numID;i++){
      cout<<"add $30,$30,$4"<<endl;
    }
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $31,-4($30)"<<endl;
    cout<<"jr $31"<<endl;

  }
  else if(t->rule == "dcls"){
    // leave blank
  }
  else if(t->rule == "statements"){
    // leave blank
  }
  else if(t->rule == "type INT"){
    // leave blank
  }
  else if(t->rule == "dcl type ID"){
    int offset = locID.find(t->children[1]->tokens[1])->second;
    cout<<"lis $5"<<endl;
    cout<<".word "<<offset<<endl;
    cout<<"add $3,$29,$5"<<endl;
  }
  else if(t->rule == "expr term"){
    genCode(t->children[0]);
  }
  else if(t->rule == "term factor"){
    genCode(t->children[0]);
  }
  else if(t->rule == "factor ID"){
    int off = locID.find(t->children[0]->tokens[1])->second;
    cout<<"lw $3,"<<off<<"($29)"<<endl;
  
  }
  else if(t->rule == "factor NUM"){
    cout<<"lis $3"<<endl;
    cout<<".word "<<t->children[0]->tokens[1]<<endl;
  }
  else if(t->rule == "expr expr PLUS term"){
    string et2 = getType(t->children[0]), tt1 = getType(t->children[2]);
    if(et2 == "int" && tt1 == "int"){
      genCode(t->children[0]);
      cout<<"sw $3,-4($30)"<<endl;
      cout<<"sub $30,$30,$4"<<endl;
      genCode(t->children[2]);
      cout<<"add $30,$30,$4"<<endl;
      cout<<"lw $5,-4($30)"<<endl;
      cout<<"add $3,$5,$3"<<endl;
    }
    else if(et2 == "int*" && tt1 == "int"){
      genCode(t->children[0]);
      cout<<"sw $3,-4($30)"<<endl;
      cout<<"sub $30,$30,$4"<<endl;
      genCode(t->children[2]);
      cout<<"add $3,$3,$3"<<endl;
      cout<<"add $3,$3,$3"<<endl;
      cout<<"add $30,$30,$4"<<endl;
      cout<<"lw $5,-4($30)"<<endl;
      cout<<"add $3,$5,$3"<<endl;
    }
    else if(et2 == "int" && tt1 == "int*"){
      genCode(t->children[0]);
      cout<<"sw $3,-4($30)"<<endl;
      cout<<"sub $30,$30,$4"<<endl;
      genCode(t->children[2]);
      cout<<"add $3,$3,$3"<<endl;
      cout<<"add $3,$3,$3"<<endl;
      cout<<"add $30,$30,$4"<<endl;
      cout<<"lw $5,-4($30)"<<endl;
      cout<<"add $3,$5,$3"<<endl;
    }
  }
  else if(t->rule == "expr expr MINUS term"){
    string et2 = getType(t->children[0]), tt1 = getType(t->children[2]);
    if(et2 == "int" && tt1 == "int"){
      genCode(t->children[0]);
      cout<<"sw $3,-4($30)"<<endl;
      cout<<"sub $30,$30,$4"<<endl;
      genCode(t->children[2]);
      cout<<"add $30,$30,$4"<<endl;
      cout<<"lw $5,-4($30)"<<endl;
      cout<<"sub $3,$5,$3"<<endl;
    }
    else if(et2 == "int*" && tt1 == "int"){
      genCode(t->children[0]);
      cout<<"sw $3,-4($30)"<<endl;
      cout<<"sub $30,$30,$4"<<endl;
      genCode(t->children[2]);
      cout<<"add $3,$3,$3"<<endl;
      cout<<"add $3,$3,$3"<<endl;
      cout<<"add $30,$30,$4"<<endl;
      cout<<"lw $5,-4($30)"<<endl;
      cout<<"sub $3,$5,$3"<<endl;
    }
    else if(et2 == "int*" && tt1 == "int*"){
      genCode(t->children[0]);
      cout<<"sw $3,-4($30)"<<endl;
      cout<<"sub $30,$30,$4"<<endl;
      genCode(t->children[2]);
      cout<<"add $30,$30,$4"<<endl;
      cout<<"lw $5,-4($30)"<<endl;
      cout<<"sub $3,$5,$3"<<endl;
      cout<<"div $3,$4"<<endl;
      cout<<"mflo $3"<<endl;
    }
  }
  else if(t->rule == "term term STAR factor" || t->rule == "term term SLASH factor" || t->rule == "term term PCT factor"){
    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;

    if(t->tokens[2] == "STAR"){
      cout<<"mult $5,$3"<<endl;
      cout<<"mflo $3"<<endl;
    }
    else if(t->tokens[2] == "SLASH"){
      cout<<"div $5,$3"<<endl;
      cout<<"mflo $3"<<endl;
    }
    else if(t->tokens[2] == "PCT"){
      cout<<"div $5,$3"<<endl;
      cout<<"mfhi $3"<<endl;
    }
  }
  else if(t->rule == "factor LPAREN expr RPAREN"){
    genCode(t->children[1]);
  }
  else if(t->rule == "statements statements statement"){
    genCode(t->children[0]);
    genCode(t->children[1]);
  }
  else if(t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI"){
    genCode(t->children[2]);
    cout<<"add $1,$3,$0"<<endl;
    cout<<"lis $3"<<endl;
    cout<<".word print"<<endl;
    cout<<"jalr $3"<<endl;
  }
  else if(t->rule == "dcls dcls dcl BECOMES NUM SEMI"){
    genCode(t->children[1]);
    cout<<"lis $5"<<endl;
    cout<<".word "<<t->children[3]->tokens[1]<<endl;
    cout<<"sw $5,0($3)"<<endl;
    genCode(t->children[0]);
  }
  else if(t->rule == "statement lvalue BECOMES expr SEMI"){
    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    cout<<"sw $3,0($5)"<<endl;
  }
  else if(t->rule == "lvalue ID"){
    int off = locID.find(t->children[0]->tokens[1])->second;
    cout<<"lis $5"<<endl;
    cout<<".word "<<off<<endl;
    cout<<"add $3,$29,$5"<<endl;
  }
  else if(t->rule == "lvalue LPAREN lvalue RPAREN"){
    genCode(t->children[1]);
  }
  else if(t->rule == "test expr LT expr"){
    string et1 = getType(t->children[0]), et2 = getType(t->children[0]);
    if(et1.compare(et2) != 0) bail("ERROR: wrong type when generating test expr code");

    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    if(et1 == "int" && et2 == "int")    cout<<"slt $3,$5,$3"<<endl;
    else if(et1 == "int*" && et2 == "int*") cout<<"sltu $3,$5,$3"<<endl;
  }
  else if(t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE"){
    srand(time(NULL));
    string label = "loop", label2 = "done";
    stringstream ss;;
    ss<<rand()%1000;
    string templabel;
    ss>>templabel;
    label += templabel;
    label2 += templabel;
    cout<<label<<":"<<endl;

    genCode(t->children[2]);
    cout<<"beq $3,$0,"<<label2<<endl;
    genCode(t->children[5]);
    cout<<"beq $0,$0,"<<label<<endl;
    cout<<label2<<":"<<endl;
  }
  else if(t->rule == "test expr EQ expr"){
    string et1 = getType(t->children[0]), et2 = getType(t->children[0]);
    if(et1.compare(et2) != 0) bail("ERROR: wrong type when generating test expr code");

    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    cout<<"add $1,$11,$0"<<endl;
    cout<<"beq $3,$5,1"<<endl;
    cout<<"add $1,$0,$0"<<endl;
    cout<<"add $3,$1,$0"<<endl;
  }
  else if(t->rule == "test expr NE expr"){
    string et1 = getType(t->children[0]), et2 = getType(t->children[0]);
    if(et1.compare(et2) != 0) bail("ERROR: wrong type when generating test expr code");


    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    cout<<"add $1,$0,$0"<<endl;
    cout<<"beq $3,$5,1"<<endl;
    cout<<"add $1,$11,$0"<<endl;
    cout<<"add $3,$1,$0"<<endl;
  }
  else if(t->rule == "test expr LE expr"){
    string et1 = getType(t->children[0]), et2 = getType(t->children[0]);
    if(et1.compare(et2) != 0) bail("ERROR: wrong type when generating test expr code");

    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    cout<<"slt $3,$3,$5"<<endl;
    cout<<"sub $3,$11,$3"<<endl;
  }
  else if(t->rule == "test expr GE expr"){
    string et1 = getType(t->children[0]), et2 = getType(t->children[0]);
    if(et1.compare(et2) != 0) bail("ERROR: wrong type when generating test expr code");

    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    cout<<"slt $3,$5,$3"<<endl;
    cout<<"sub $3,$11,$3"<<endl;
  }
  else if(t->rule == "test expr GT expr"){
    string et1 = getType(t->children[0]), et2 = getType(t->children[0]);
    if(et1.compare(et2) != 0) bail("ERROR: wrong type when generating test expr code");

    genCode(t->children[0]);
    cout<<"sw $3,-4($30)"<<endl;
    cout<<"sub $30,$30,$4"<<endl;
    genCode(t->children[2]);
    cout<<"add $30,$30,$4"<<endl;
    cout<<"lw $5,-4($30)"<<endl;
    if(et1 == "int" && et2 == "int")  cout<<"slt $3,$3,$5"<<endl;
    else if(et1 == "int*" && et2 == "int*") cout<<"sltu $3,$5,$3"<<endl;
  }
  else if(t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
    srand(time(NULL));
    string label1 = "truepart",label2 = "done";
    stringstream ss;
    ss<<rand()%1000;
    string templabel;
    ss>>templabel;
    label1 += templabel;
    label2 += templabel;

    genCode(t->children[2]); 
    cout<<"beq $3,$0,"<<label1<<endl;
    genCode(t->children[5]);
    cout<<"beq $0,$0,"<<label2<<endl;
    cout<<label1<<":"<<endl;
    genCode(t->children[9]);
    cout<<label2<<":"<<endl;
  }
  else if(t->rule == "dcls dcls dcl BECOMES NULL SEMI"){
    string dt = getType(t->children[1]);
    if(dt != "int*") bail("ERROR: type error ");

    genCode(t->children[1]);
    cout<<"sub $3,$3,$3"<<endl;
    genCode(t->children[0]);
  }
  else if(t->rule == "factor NULL"){
    cout<<"sub $3,$3,$3"<<endl;
  }
  else if(t->rule == "factor AMP lvalue"){
    string lt = getType(t->children[1]);
    if(lt != "int") bail("ERROR: type error");

    genCode(t->children[1]);
  }
  else if(t->rule == "factor STAR factor"){
    string ft = getType(t->children[1]);
    if(ft != "int*") bail("ERROR: type error");
    genCode(t->children[1]);
    cout<<"lw $3,0($3)"<<endl;

  }
  else if(t->rule == "lvalue STAR factor"){
    string ft = getType(t->children[1]);
    if(ft != "int*") bail("ERROR: type error");

    genCode(t->children[1]);
  }
  else if(t->rule == "factor NEW INT LBRACK expr RBRACK"){
    string et = getType(t->children[3]);
    if(et != "int") bail("ERROR: type error");
    
    genCode(t->children[3]);
    cout<<"add $1,$3,$0"<<endl;
    cout<<"lis $3"<<endl;
    cout<<".word new"<<endl;
    cout<<"jalr $3"<<endl;
  }
  else if(t->rule == "statement DELETE LBRACK RBRACK expr SEMI"){
    string et = getType(t->children[3]);
    if(et != "int*") bail("ERROR: type error");

    genCode(t->children[3]);
    cout<<"add $1,$3,$0"<<endl;
    cout<<"lis $3"<<endl;
    cout<<".word delete"<<endl;
    cout<<"jalr $3"<<endl;
  }
  else{
    vector<tree*>::iterator it;
    for(it = t->children.begin(); it!= t->children.end();it++){
      genCode(*it);

    }
  }
 
}

void printIDTable(){
  cout<<"Printing ID Table now."<<endl;
  map<string, string>::iterator it;
  for(it = idTable.begin(); it != idTable.end(); it++){
    cerr<<(*it).first<<" "<<(*it).second<<endl;
  }
  cout<<"ID Table ends."<<endl;
  cout<<"locID table:"<<endl;

  map<string,int>::iterator itt;
  for(itt = locID.begin();itt!=locID.end();itt++){
    cerr<<(*itt).first<< " " <<(*itt).second<<endl;
  }
  cout<<"locID ends"<<endl;
}
int main() {
  // Main program.
  try {
    parseTree = readParse("S");
    //printTree(parseTree);
    genSymbols(parseTree);
    //printIDTable();
    genCode(parseTree);
    
  } catch(string msg) {
    cerr << msg << endl;
  }
  if (parseTree) delete parseTree;
  return 0;
}
