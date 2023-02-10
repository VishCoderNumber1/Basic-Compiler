
/* (
    When I use good grammar,
    it is because of Tscoding's language
    (https://gitlab.com/tsoding/porth)
)*/
#ifndef MAIN_COMPILER
#define MAIN_COMPILER

#include <iostream>
using namespace std;
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>



// UTILS





enum types
{
    COMPILER,
    HELP
};



void help()
{
    cout << "Help \n";
    cout << "  -o <output>        Specifies output of file (if not given it will be derived from file name)\n";
    cout << "   <file>      -  Specifies the file to use (Required Field)";
    exit(1);
}




void CMD(string command, string file)
{
    int a = system(command.c_str());
    if (a == 1){
        cout << "\n\n[CMD] Error while running '"+command+"' on file '"+file+"'\n";
        exit(1);
    }
    cout << "[CMD] Ran "+command.substr(0, command.find_first_of(" "))+" with output file " + file + "\n";
}




string changeFileExt(string file, string ext){
    size_t n = file.find_last_of('.');
    if (n==string::npos){
        return file;
    };
    return file.substr(0, n+1) + ext;
}





size_t count_substrings(std::string const &needle, std::string const &haystack)
{
    size_t count = 0;

    for (size_t pos =0; (pos=haystack.find(needle, pos)) != std::string::npos; ++pos, ++count)
        ;

   return count;
}



void runAsm(string output, string file)
{
    CMD("nasm -fwin32 "+output, file);
    CMD("gcc "+file.substr(0, file.find_last_of("."))+".obj"+" -o "+file.substr(0, file.find_last_of('.')+1)+"exe", file);
}



std::string remove_extension(const std::string& filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot); 
}


void CompileProj(string file, int type, string output);



vector<string> files;




// TOKENS




#include <cctype>

enum tokTypes{

    LP,

    RP,

    LB,

    RB,

    LNSB,

    RNSB,

    COLON,

    ID,

    STRING,

    NUMBER,

    EOFT,

    EQUALS,

    PLUS,

    MINUS,

    MUL,

    DIV,

    GT,

    LT,

    SEMICOLON,

    COMMA,

    LAST_NUM
    // TELLS US IF THERE IS AN ERROR WITH THE TOKEN TYPE IF IT IS TOO BIG

};




class Token{
    public:
        tokTypes type;
        string value;
        Token(tokTypes typeS, string valueS)
        {
            type = typeS;
            value = valueS;
            if (type >= LAST_NUM){
                cout << "Exhaustive handling of tok operations";
                exit(1);
            }
        }
        Token(){;}
};




Token new_tok(tokTypes type, string valueS)
{
    Token t;
    t.type = type;
    t.value = valueS;
    return t;
}



class Lexer{
    public:
    string contents = "";
    string file = "";
    int count = 0;
    int line = 0;
    int col = 0;
    char curChar = '\0';
    Lexer(){};
    Lexer(string contentsS, string fileS)
    {
        contents = contentsS;
        file = fileS;
    }
    void lexer_advance()
    {
        count++;
        if (curChar == '\n'){line++;col = 0;}
        else{col++;};
        curChar = contents[count];
    }
    void lexer_error(string title, string desc)
    {
        cout << "File " << file << ",\n " << title << ": " << desc;
        char* pch = strtok((char*)contents.c_str(), (char*)"\n");
        if (line == 0){cout << "\n" << pch;};
        int i = 0;
        while (pch != NULL){
            pch = strtok(NULL, (char*)"\n");
            i++;
            if (i == line){
                cout << "\n" << pch << "\n";
                for (int i =0; col!=i; i++){cout << " ";};
                cout << "^";
            };
        };
    }
    Token lex_tok()
    {
        curChar = contents[count];
        switch (curChar){
            case '=': lexer_advance(); return new_tok(EQUALS, "=");
            case '(': lexer_advance(); return new_tok(LP, "(");
            case ')': lexer_advance(); return new_tok(RP, ")");
            case '{': lexer_advance(); return new_tok(LB, "{");
            case '}': lexer_advance(); return new_tok(RB, "}");
            case '[': lexer_advance(); return new_tok(LNSB, "[");
            case ']': lexer_advance(); return new_tok(RNSB, "[");
            case ':': lexer_advance(); return new_tok(COLON, ":");
            case '\n': lexer_advance(); return lex_tok();
            case '+': lexer_advance(); return new_tok(PLUS, "+");
            case '-': lexer_advance(); return new_tok(MINUS, "-");
            case '*': lexer_advance(); return new_tok(MUL, "*");
            case ',': lexer_advance(); return new_tok(COMMA, ",");
            case ';': lexer_advance(); return new_tok(SEMICOLON, ";");
            case '/': {
                lexer_advance();
                if(curChar=='/'){
                    lexer_advance();
                    while (curChar!='\n'){
                        lexer_advance();
                    };
                    lexer_advance();
                    return lex_tok();
                }else if(curChar=='*'){
                    lexer_advance();
                    while (curChar!='*' && contents[count+1] != '/'){
                        lexer_advance();
                    };
                    lexer_advance();
                    lexer_advance();
                    return lex_tok();
                }
                return new_tok(DIV, "/");
            }
            case '<': lexer_advance(); return new_tok(LT, "<");
            case '>': lexer_advance(); return new_tok(GT, ">");
            case '\0': return new_tok(EOFT, "\0");
            case '\r': lexer_advance(); return lex_tok();
            case '\'': {
                lexer_advance();
                string str;
                while (curChar != '\''){
                    str += curChar;
                    lexer_advance();
                };
                lexer_advance();
                return new_tok(STRING, str);
            }
            case '"': {
                lexer_advance();
                string str;
                int quotes = 200;
                while (curChar != '"' && quotes != 0){
                    if (curChar == '\\'){
                        lexer_advance();
                        if (curChar == 'n' && quotes){
                            str += "', 0xA, '";
                            lexer_advance();
                            continue;
                        }else if(curChar == 't' && quotes){
                            str += "\t";
                            lexer_advance();
                            continue;
                        }else if(curChar == 'r' && quotes){
                            str += "', 13, 10, '";
                            lexer_advance();
                            continue;
                        }else if(curChar == 'b' && quotes){
                            str += "\b";
                            lexer_advance();
                            continue;
                        }else if(curChar == '0' && quotes){
                            str += "\0";
                            lexer_advance();
                            continue;
                        }
                        else{
                            str += curChar;
                            lexer_advance();
                            continue;
                        }
                    }
                    str += curChar;
                    lexer_advance();
                };
                lexer_advance();
                return new_tok(STRING, str);
            }
            case ' ': lexer_advance(); return lex_tok();
            case '_': {
                lexer_advance();
                string id = "_";
                while (isalnum(curChar)||curChar=='@'){
                    id += curChar;
                    lexer_advance();
                };
                return new_tok(ID, id);
            }
            default:
                if (isalpha(curChar)){
                    string id;
                    while ((isalpha(curChar)||curChar=='@')&&curChar!='\n'){
                        id += curChar;
                        lexer_advance();
                    };
                    return new_tok(ID, id);
                }else if(isdigit(curChar)){
                    string num;
                    while (isdigit(curChar)){
                        num += curChar;
                        lexer_advance();
                    };
                    if (num == "0" && curChar == 'x'){
                        while (isalnum(curChar)){
                            num += curChar;
                            lexer_advance();
                        };
                    }
                    return new_tok(NUMBER, num);
                }else{
                    string str = "Unexpected token " + string(1,this->curChar);
                    lexer_error("SyntaxError", str);
                    exit(1);
                }
        };
    };
};



// COMPILER




class Compile{
    public:
        Lexer lexer;
        vector<Token> toks;
        int i = 0;
        string res;
        int count = 0;
        int tabs = 0;
        int plusS = 0;
        Compile(Lexer lexerS, vector<Token> toksS)
        {
            lexer = lexerS;
            toks = toksS;
        }
        void expect(Token expect)
        {
            if (toks[i].type == expect.type){
                i++;
            }else{
                lexer.lexer_error("SyntaxError", "Expected token \""+expect.value+"\", found \""+toks[i].value+"\"");
                exit(1);
            };
        }
        string addTabs()
        {
            string rr;
            for (int i=0;i<tabs; i++){
                rr += "\t";
            };
            return rr;
        }
        string curFuncName = "";
        bool sysOp = false;
        string str = "eax";
        vector<string> strs = {"eax", "eax", "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"};
        int cn = 3;
        int ifStatement = 0;
        int doneCount = 0;
        int adds = 0;
        string op = "jne";
        string comp_tok()
        {
            switch (toks[i].type){
                case STRING: {
                    string toput = "LC"+to_string(count)+": db '"+toks[i].value+"'"+", 0"+"\n";
                    res = toput + res;
                    if (sysOp){
                        cn++;
                        return "mov "+strs[cn-1]+", LC"+to_string(count++)+"\n"+addTabs();
                    }
                    return "push LC"+to_string(count++)+"\n"+addTabs();
                }
                case RB: {
                    string r = "";
                    if(curFuncName == "main"){
                        res = "extern _exit\n" + res;
                        r = "push 0\n"+addTabs()+"call _exit\n";
                    }
                    curFuncName = "";
                    tabs--;
                    return r;
                }
                case GT: {
                    op = "jg";
                    if (toks[i].value == ">="){
                        op = "jge";
                    };
                    return "cmp "+strs[cn-2]+", "+strs[cn-1]+"\n"+addTabs();
                }
                case LT: {
                    op = "jl";
                    if (toks[i].value == "<="){
                        op = "jle";
                    };
                    return "cmp "+strs[cn-2]+", "+strs[cn-1]+"\n"+addTabs();
                }
                case NUMBER: {
                    if (sysOp){
                        return "mov "+strs[cn++]+", "+toks[i].value+"\n"+addTabs();
                    }
                    return "push "+toks[i].value+"\n"+addTabs();
                }
                case ID: {
                    string val = toks[i].value;
                    if (val == "is"){
                        op = "je";
                        return "cmp "+strs[cn-2]+", "+strs[cn-1]+"\n"+addTabs();
                        // je command
                    }else if (val == "isnt"){
                        op = "jne";
                        return "cmp "+strs[cn-2]+", "+strs[cn-1]+"\n"+addTabs();
                        // je command
                    }
                    i++;
                    string r = "";
                    if (toks[i].value == "return"){
                        return "ret\n"+addTabs();
                    }else if(toks[i-1].value == "if"){
                        i++; // for now: do the conditional
                        string str = "";
                        sysOp = true;
                        while (toks[i].type != RP){
                            str += comp_tok();
                            i++;
                        };
                        i++;
                        if (str.substr(0, string("mov eax, ").length()) == "mov eax, "){
                            op = "jne";
                            str += "mov ebx, 0\n\t";
                            str += "cmp eax, ebx\n";
                        }
                        sysOp = false;
                        string code = "";
                        i++;
                        while (toks[i].type != RB){
                            code += comp_tok();
                            i++;
                        }
                        tabs--;
                        string rs = res;
                        res = "ifSt"+to_string(ifStatement++) + ":\n\t";
                        res += code + "\n\t";
                        res += "jmp done"+to_string(doneCount+1)+"\n\t";
                        res += rs;
                        if (op != ""){
                            str += op + " "+"ifSt"+to_string(ifStatement-1)+"\n\t";
                        };
                        string r = str+"\n"+addTabs();
                        r += "jmp done"+to_string(doneCount+1)+"\n"+addTabs();
                        r += "done"+to_string(doneCount+1)+":\n";
                        doneCount++;
                        tabs++;
                        if (toks[i+1].value == "else"){
                            i++;
                            i+=2;
                            string code = "";
                            while (toks[i].type != RB){
                                code += comp_tok();
                                i++;
                            };
                            rs = res;
                            res = "elseSt"+to_string(ifStatement++) + ":\n\t";
                            res += code+"\n\t";
                            res += "jmp done"+to_string(ifStatement)+"\n\t";
                            res += rs;
                            string d = "\tjmp ifSt"+to_string(ifStatement-1);
                            i++;
                            r = r.substr(0, r.length()-17) + "\tjmp elseSt"+to_string(ifStatement-1) + "\n\t"+ r.substr(r.length()-17, r.length());
                        };
                        r += addTabs();
                        i--;
                        return r;
                    }else if(toks[i-1].value == "while"){
                        string str = "";
                        i++;
                        sysOp = true;
                        while (toks[i].type != RP){
                            str += comp_tok();
                            i++;
                        };
                        sysOp = false;
                        i++;
                        if (toks[i].type != LB){
                            lexer.lexer_error("SyntaxError", "Left bracket for if statement not found");
                        }
                        string code = "";
                        i++;
                        while (toks[i].type != RB){
                            code += comp_tok();
                            i++;
                        }
                        res = "codeL"+to_string(ifStatement++) + ":\n"+addTabs()+ code + str+"\n\t"+op+" codeL"+to_string(doneCount)+"\n\tjmp done"+to_string(ifStatement)+"\n\t"+ res;
                        if (op != ""){
                            str += op + " "+"codeL"+to_string(ifStatement-1)+"\n\t";
                        }else{
                            str += "";
                        };
                        string r = "jmp done"+to_string(doneCount-1)+"\n";
                        doneCount++;
                        i--;
                        tabs++;
                        return "jmp codeL"+to_string(ifStatement-1)+"\ndone"+to_string(doneCount-1)+":\n\t";
                    }
                    if (toks[i].type == LP){
                        int nPl = plusS;
                        int nCn = cn;
                        if (val == "add"){
                            plusS++;
                            sysOp = true;
                            adds++;
                        }
                        i++;
                        if (val == "asm"){
                            if (toks[i].value == "BEGIN"){
                                i+=2;
                                res = toks[i].value + "\n"+res;
                                i++;
                                return "";
                            };
                            i++;
                            return toks[i-1].value+"\n"+addTabs();
                        }
                        while (toks[i].type != RP){\
                            if (toks[i].type != COMMA){
                                string str = comp_tok();
                                r += str;
                            };
                            i++;
                        }
                        i++;
                        if (toks[i].type == LB){
                            curFuncName = val;
                            tabs++;
                            if (val == "main"){val = "_main";}
                            r += "\n"+val + ":\n"+addTabs();
                        }else{
                            if (val == "print"){
                                if (r.length()==0){
                                    res = "LC"+to_string(count++)+": db '%c', 0\n" + res;
                                    r = "push 10\n\tpush LC"+to_string(count-1)+"\n\t" + r;
                                }
                                res = "extern _printf\n" + res;
                                val = "_printf";
                            }else if(val == "exit"){
                                res = "extern _exit\n" + res;
                                val = "_exit";
                            }else if(val == "MessageBox"){
                                res = "extern _MessageBoxA@16\n" + res;
                                val = "_MessageBoxA@16";
                            }else if(val[0] == '_'){
                                res = "extern "+val+"\n"+res;
                            }
                            if(val == "add"){
                                if (adds == 1){
                                    sysOp = false;
                                }
                                if (nPl == 0){
                                    r += "pop "+strs[nCn-1]+"\n"+addTabs();
                                    r += "pop "+strs[nCn-2]+"\n"+addTabs();
                                }
                                r += (string)"add "+strs[cn-2]+", "+strs[cn-1]+"\n"+addTabs() + "push "+strs[cn-2]+"\n"+addTabs();
                                adds--;
                                cn--;
                            }else{
                            r += "call "+val+"\n"+addTabs();
                            }
                            i--;
                        };
                    }else if(toks[i-1].value == "use"){
                        i++;
                        string lib = toks[i-1].value;
                        string dir = "";
                        size_t n = lexer.file.find_last_of('/');
                        if (n != string::npos){dir = lexer.file.substr(0, n+1);};
                        lib = dir + lib;
                        if (lib == lexer.file){
                            lexer.lexer_error("IncludeError", "Recursive includes of file \""+lexer.file+"\"");
                        };
                        if (lib[lib.size()-1] == 'm' && lib[lib.size()-2] == 'c' && lib[lib.size()-3]=='.'){
                            CompileProj(lib, COMPILER, lib.substr(0, lib.find_last_of("."))+".asm");
                            string r =  "%include \""+lib.substr(0, lib.find_last_of("."))+".asm\"";
                            return r;
                        }else{;
                            return "%include \""+lib+"\"";
                        }
                    }else if(toks[i].value == "+" && toks[i+1].value == "+"){
                        string vn = toks[i-1].value;
                        i++;
                        return "inc "+vn+"\n"+addTabs();
                    }else if(toks[i].value == "-" && toks[i+1].value == "-"){
                        string vn = toks[i-1].value;
                        i++;
                        return "dec "+vn+"\n"+addTabs();
                    }

                    if (toks[i].value == "["){
                        sysOp = true;
                    }
                    
                    
                    if (toks[i-1].value == "argc"){
                        r = "mov "+strs[cn++]+", [esp+4]\n"+addTabs();
                        cn--;
                        if (sysOp == false){
                            r += "push "+strs[cn]+"\n"+addTabs();
                            r += "mov "+strs[cn]+", 0\n"+addTabs();
                        }
                        cn++;
                        i--;
                        if (toks[i+1].value == "["){
                            i++;
                        }
                    }else if(toks[i-1].value == "argv"){
                        r = "mov "+strs[cn++]+", [esp+8]\n"+addTabs();
                        if (sysOp == false){
                            r += "push "+strs[cn]+"\n"+addTabs();
                            r += "mov "+strs[cn]+", 0\n"+addTabs();
                        }
                        i--;
                        cn++;
                        if (toks[i+1].value == "["){
                            i++;
                        }
                    }
                    if (toks[i].value == "["){
                        string str;
                        int cnt = stoi(toks[i+1].value);
                        i--;
                        if (r == ""){
                            str = "mov "+strs[cn++]+", "+toks[i].value + "\n" + addTabs();
                            str += "mov "+strs[cn++]+", ["+strs[cn-2]+"+"+to_string(cnt*4)+"]\n" + addTabs();
                            cn -= 2;
                            str += "push "+strs[cn+1]+"\n"+addTabs();
                            str += "mov "+strs[cn]+", 0\n"+addTabs();
                            str += "mov "+strs[cn+1]+", 0\n"+addTabs();
                            // mov ebx, list \n mov ecx, [ebx+4]\npush ecx\nmov ebx, 0\nmov ecx,0
                        }else{
                            str =  r;
                            sysOp = false;
                            str += "mov "+strs[cn-1]+", ["+strs[cn-2]+"+"+to_string(cnt*4)+"]\n"+addTabs();
                            str += "push "+strs[cn-1]+"\n"+addTabs();
                            str += "mov "+strs[cn-1]+", 0\n"+addTabs();
                        }
                        i+=3;
                        return str;
                    }

                    if (r==""){
                        i--;
                        if (sysOp){
                            return "mov "+strs[cn++]+", "+toks[i].value+"\n"+addTabs();
                        }
                        return "push "+toks[i].value+"\n"+addTabs();
                    }
                    return r;
                }
                case SEMICOLON: {return "";}
            }
            return "push"+toks[i].value+"\n"+addTabs();
        };
        void all(){
            for (; i<toks.size(); i++){
                res += comp_tok();
            };
            res = "global _main\nsection .text\nNULL equ 0\nTRUE equ 1\nFALSE equ 0\n" + res;
        }
};



void CompileProj(string file, int type, string output)
{
    files.push_back(file);
    ifstream stream(file);
    if(!stream){
        printf("Error: File %s not found", file.c_str());
        exit(1);
    }
    string text;
    string ln;
    while (getline (stream, ln)) {
        text += ln + "\n";
    }
    Lexer lexer = Lexer(text, file);
    vector<Token> toks;
    Token tok;
    int tokslength = 0;
    int plusPuts = 0;
    int pos = 0;
    while (tok.type != EOFT){
        tok = lexer.lex_tok();
        if(tok.type==EOFT){break;};
        if(tok.value == "is"){
            Token tk = lexer.lex_tok();
            toks.push_back(tk);
            toks.push_back(tok);
            pos = 1;
        }else if(tok.value == "isnt"){
            Token tk = lexer.lex_tok();
            toks.push_back(tk);
            toks.push_back(tok);
            pos = 1;
        }else if(tok.value == "<"){
            Token tk = lexer.lex_tok();
            if (tk.value == "="){
                tok.value = "<=";
                tk = lexer.lex_tok();
            }
            toks.push_back(tk);
            toks.push_back(tok);
        }else if(tok.value == ">"){
            Token tk = lexer.lex_tok();
            if (tk.value == "="){
                tok.value = ">=";
                tk = lexer.lex_tok();
            }
            toks.push_back(tk);
            toks.push_back(tok);
        }
        else{
            if (pos != 0){
                pos++;
            }
            if(plusPuts!=0&&(pos == 3 | pos == 3 * plusPuts)){
                pos -= 3;
                toks.push_back(new_tok(PLUS, "()+"));
            }else{
                toks.push_back(tok);
            }
        }
    };
    Compile cmp(lexer, toks);
    cmp.all();
    // cout << cmp.res;
    fstream f(output, ios::out);
    f << cmp.res;
    f.close();
    cout << "\n";
}




// MAIN



int main(int argc,char** argv)
{
    string output = "";  // FILE TO USE AS OUTPUT
    string file = "";    // FILE THAT WILL BE READ

    int type = COMPILER; // TELLS WHETER IT IS GOING TO COMPILE CODE OR SHOW THE HELP

    for (int i=1;i<argc;i++){
        if (string(argv[i]) == "-o"){
            i++;
            output = string(argv[i]);
        }else if(string(argv[i]) == "-h"){
            help();
        }else{
            file = string(argv[i]);
        };
    };
    if (file == ""){
        cout << "ERROR: NO FILE PASSED IN\n";
        exit(1);
    }
    if (output == ""){
        output = changeFileExt(file, "asm");
    };
    CompileProj(file, type, output);
    runAsm(output, file);
    for (int i=0;i<files.size();i++){
        remove((char*)string(remove_extension(files[i])+".obj").c_str());
        if (files[i] != file){
            remove((char*)string(remove_extension(files[i])+".asm").c_str());
        }
    }
    return 0;
}

#endif
