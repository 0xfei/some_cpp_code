#include <iostream>
#include <vector>
#include <deque>
#include <stack>
#include <map>

using namespace std;

const string CS_ASN(":=");
const string CS_AND("and");
const string CS_OR("or");
const string CS_TRUE("true");
const string CS_FALSE("false");
const string CS_IF("if");
const string CS_THEN("then");
const string CS_ELSE("else");
const string CS_WHILE("while");
const string CS_DO("do");

typedef enum {
    IT_OPERATOR,
    IT_VARIABLE,
    IT_BOOL,
    IT_NUMBER,
    IT_END,
} ITYPE;

struct Term {
    virtual void output() {
        cout << ";" << endl;
    };
    ITYPE type;
};

struct Operator : Term {
    virtual void output() {
        cout << v << " ";
    }
    string v;
};

struct Variable : Term {
    virtual void output() {
        cout << var << " ";
    }
    string var;
};

struct Number : Term {
    virtual void output() {
        cout << number << " ";
    }
    int number;
};

struct Bool : Term {
    virtual void output() {
        cout << t;
    }
    bool t;
};

typedef vector<Term*> exp_vector;
typedef vector<Term*>::size_type exp_index;

/*
 * ast
 */
typedef enum {
    OPT_START,
    OPT_ASSIGN,
    OPT_IF,
    OPT_WHILE,
} OPTYPE;

struct AST {
    OPTYPE Type;
    string lvalue;
    pair<int, int> rvalue;
    pair<int, int> pred;
    AST* block1;
    AST* block2;
    AST* next;
};


class Interpreter {
public:
    Interpreter(const string& s): _exp(s), _ast_root(NULL) {}
    ~Interpreter() {
        free_ast(_ast_root);
        for (int i = 0; i < _ast_queue.size(); ++i) {
            if (_ast_queue[i]->type != IT_END) {
                delete _ast_queue[i];
            }
        }
    }
    bool eval();

    void debug() {
        for (int i = 0; i < _ast_queue.size(); ++i) {
            _ast_queue[i]->output();
        }
        cout << endl;
    }

    void output() {
        for (map<string,long long>::iterator it = _environment.begin();
             it != _environment.end();
             ++it) {
            cout << it->first << " " << it->second << endl;
        }
    }

private:
    /* helper */
    bool is_space(char c) { return c == ' '; }

    bool is_linend(char c) { return (c == ';') || (c == '\n'); }

    bool is_variable(string exp) { return (!exp.empty() && isalpha(exp[0])); }

    bool is_number(string exp) { return (!exp.empty() && !is_variable(exp)); }

    int to_number(string exp) { return atoi(exp.c_str()); }

    void skip_space(int &i, const int len, const string &exp) {
        while (i < len && is_space(exp[i])) { ++i; }
    }

    bool is_block_end(Term* term) {
        return (term->type == IT_OPERATOR && ((Operator*)term)->v[0] == '}');
    }

    void set_variable_value(const string &v, long long value) {
        _environment[v] = value;
    }

    long long get_variable_value(const string &v) {
        return _environment[v];
    }

    inline int weight(const Operator *exp);

    exp_index find_block(exp_index start, exp_index end);
    exp_index find_then(exp_index start, exp_index end);
    exp_index find_else(exp_index start, exp_index end);
    exp_index find_do(exp_index start, exp_index end);
    exp_index find_end(exp_index start, exp_index end);

    /* internal */
    bool get_item(string &item, string &left);

    Term* parse_item(bool &still);

    void free_ast(AST* root);

    /* parser */
    bool parser();

    /* make ast */
    AST* make_ast(exp_index& start, const exp_index end);

    /* execute */
    long long simple_execute(const exp_index start, const exp_index end);

    /* do_eval */
    long long do_eval(AST* _ast_root);

    map<string, long long> _environment; /* _environment */
    exp_vector _ast_queue; /* terms */
    AST* _ast_root; /* ast root tree */
    string _exp; /* source code */
};

/*
 * free memory
 */
void Interpreter::free_ast(AST *root)
{
    while (root != NULL) {
        if (root->block1) {
            free_ast(root->block1);
        }
        if (root->block2) {
            free_ast(root->block2);
        }
        AST* t = root->next;
        delete root;
        root = t;
    }
}


/*
 * find helper
 */
exp_index Interpreter::find_else(exp_index start, exp_index end) {
    while (start < end) {
        Operator* opt = (Operator*)_ast_queue[start];
        if (opt->v == CS_ELSE) {
            return start;
        }
        ++start;
    }
    return end;
}

exp_index Interpreter::find_then(exp_index start, exp_index end) {
    while (start < end) {
        Operator* opt = (Operator*)_ast_queue[start];
        if (opt->v == CS_THEN) {
            return start;
        }
        ++start;
    }
    return end;
}

exp_index Interpreter::find_do(exp_index start, exp_index end) {
    while (start < end) {
        Operator* opt = (Operator*)_ast_queue[start];
        if (opt->v == CS_DO) {
            return start;
        }
        ++start;
    }
    return end;
}

exp_index Interpreter::find_end(exp_index start, exp_index end) {
    while (start < end) {
        if (_ast_queue[start]->type == IT_END) {
            return start;
        }
        ++start;
    }
    return end;
}

exp_index Interpreter::find_block(exp_index start, exp_index end) {
    while (start < end) {
        Operator* opt = (Operator*)_ast_queue[start];
        if (opt->v[0] == '{') {
            return start;
        }
        ++start;
    }
    return end;
}


/*
 *  get item , return false if exp end
 */
bool Interpreter::get_item(string &item, string &left)
{
    int i = 0, len = static_cast<int >(_exp.size());

    item.clear();
    left.clear();
    skip_space(i, len, _exp);

    if (i == len) {
        return false;
    }

    while (i < len && !is_space(_exp[i]) && !is_linend(_exp[i])) {
        item = item + _exp[i];
        ++i;
    }

    skip_space(i, len, _exp);

    if (i == len) {
        return false;
    }

    if (is_linend(_exp[i])) {
        left = _exp.substr(i+1);
        return false;
    } else {
        left = _exp.substr(i);
        return true;
    }
}


/*
 *  parse_item
 */
Term* Interpreter::parse_item(bool &still)
{
    Term* ast = NULL;

    string left_exp = _exp, item, label;
    still = get_item(item, left_exp);

    if (item == CS_ASN ||
        item == CS_AND ||
        item == CS_OR  ||
        item == CS_IF  ||
        item == CS_THEN ||
        item == CS_ELSE ||
        item == CS_WHILE||
        item == CS_DO  ||
        item[0] == '+' ||
        item[0] == '-' ||
        item[0] == '*' ||
        item[0] == '/' ||
        item[0] == '<' ||
        item[0] == '>' ||
        item[0] == '(' ||
        item[0] == ')' ||
        item[0] == '{' ||
        item[0] == '}')
    {
        Operator* itm = new Operator;
        itm->type = IT_OPERATOR;
        itm->v = item;
        ast = itm;
    } else if (item == CS_TRUE || item == CS_FALSE) {
        Bool* bol = new Bool;
        bol->type = IT_BOOL;
        bol->t = item == CS_TRUE;
        ast = bol;
    } else if (is_variable(item)) {
        Variable* var = new Variable;
        var->type = IT_VARIABLE;
        var->var = item;
        ast = var;
    } else if (is_number(item)) {
        Number* num = new Number;
        num->type = IT_NUMBER;
        num->number = to_number(item);
        ast = num;
    }

    _exp = left_exp;
    return ast;
}


/*
 *  parser, split source code to terms
 */
bool Interpreter::parser()
{
    static Term s_end;
    s_end.type = IT_END;

    while (!_exp.empty()) {
        bool still;
        Term* item = parse_item(still);
        if (item) {
            if (is_block_end(item)) {
                _ast_queue.push_back(&s_end);
                still = true;
            }
            _ast_queue.push_back(item);
        }
        if (!still) {
            _ast_queue.push_back(&s_end);
        }
    }

    return true;
}


/*
 * build ast
 */
AST* Interpreter::make_ast(exp_index& start, const exp_index end)
{
    AST* root = new AST;
    root->Type = OPT_START;
    root->next = NULL;
    root->block1 = NULL;
    root->block2 = NULL;

    AST* last = root;

    string lvalue;

    while (start < end) {
        Term* item = _ast_queue[start];
        if (item->type == IT_VARIABLE) {
            Variable* var = (Variable*)item;
            lvalue = var->var;
            ++start;
        } else if (item->type == IT_OPERATOR) {
            Operator* opt = (Operator*)item;
            if (opt->v == CS_IF) { // if
                exp_index next = find_then(start, end);
                AST* item = new AST;
                item->Type = OPT_IF;
                item->next = NULL;
                item->pred.first = start + 1;
                item->pred.second = next;

                start = find_block(next + 1, end) + 1;
                item->block1 = make_ast(start, end);

                start = find_else(start, end) + 1;
                item->block2 = make_ast(start, end);

                last->next = item;
                last = item;
            } else if (opt->v == CS_WHILE) { // while
                exp_index next = find_do(start, end);
                AST* item = new AST;
                item->Type = OPT_WHILE;
                item->next = NULL;
                item->block2 = NULL;
                item->pred.first = start + 1;
                item->pred.second = next;

                start = find_block(next + 1, end) + 1;
                item->block1 = make_ast(start, end);

                last->next = item;
                last = item;
            } else if (opt->v == CS_ASN) { // assign
                exp_index next = find_end(start, end);
                AST* item = new AST;
                item->Type = OPT_ASSIGN;
                item->lvalue = lvalue;
                item->rvalue.first = start + 1;
                item->rvalue.second = next;
                item->next = NULL;
                item->block1 = NULL;
                item->block2 = NULL;

                last->next = item;
                last = item;

                start = next + 1;
            } else if (opt->v[0] == '}') { // end of block
                ++start;
                return root;
            } else {
                // should be error!
                ++start;
            }
        } else /*IT_BOOL IT_NUMBER IT_END*/ {
            lvalue.clear();
            ++start;
        }
    }

    return root;
}


/*
 *  weight, calculate expressions
 */
int Interpreter::weight(const Operator *exp)
{
    const string &s = exp->v;

    if (s == CS_AND) {
        return 0;
    } else if (s == CS_OR) {
        return 1;
    } else if (s == ">" || s == "<") {
        return 2;
    } else if (s == "+" || s == "-") {
        return 3;
    } else if (s == "*" || s == "/") {
        return 4;
    } else if (s == "(") {
        return -1;
    } else {
        return -2;
    }
}

long long Interpreter::simple_execute(const exp_index start, const exp_index end)
{
    deque<Term*> temp;
    stack<Term*> lque;
    stack<long long> ans;

    for (exp_index i = start; i < end; ++i) {
        Term* exp = _ast_queue[i];

        if (exp->type == IT_NUMBER ||
            exp->type == IT_VARIABLE ||
            exp->type == IT_BOOL) {
            /* push back */
            temp.push_back(exp);
            continue;
        } else if (exp->type == IT_END) {
            continue;
        }

        if (lque.empty()) {
            lque.push(exp);
            continue;
        }

        Operator* label = (Operator*)exp;

        if (label->v == "(") {
            lque.push(exp);
            continue;
        } else if (label->v == ")") {
            while ((exp = lque.top()) &&
                   (label = (Operator*)exp) &&
                   (label->v != "(")) {
                temp.push_back(exp);
                lque.pop();
            }
            lque.pop();
            continue;
        }

        while (!lque.empty() &&
               weight((Operator*)lque.top()) >= weight(label)) {
            temp.push_back(lque.top());
            lque.pop();
        }
        lque.push(exp);
    }

    while (!lque.empty()) {
        temp.push_back(lque.top());
        lque.pop();
    }

    for (deque<Term*>::size_type i = 0; i < temp.size(); ++i) {
        Term* exp = temp[i];

        if (exp->type == IT_OPERATOR) {
            Operator* opt = (Operator*)exp;
            const string& s = opt->v;

            long long a, b = ans.top();
            ans.pop();
            if (ans.empty()) {
                a = 0;
            } else {
                a = ans.top();
                ans.pop();
            }

            if (s == CS_AND) {
                ans.push(a && b);
            } else if (s == CS_OR) {
                ans.push(a || b);
            } else if (s == ">") {
                ans.push(a > b);
            } else if (s == "<") {
                ans.push(a < b);
            } else if (s == "+") {
                ans.push(a + b);
            } else if (s == "-") {
                ans.push(a - b);
            } else if (s == "*") {
                ans.push(a * b);
            } else if (s == "/") {
                ans.push(a / b);
            } else {
                break;
            }
        } else {
            if (exp->type == IT_NUMBER) {
                Number* num = (Number*)exp;
                ans.push(num->number);
            } else if (exp->type == IT_VARIABLE) {
                Variable* var = (Variable*)exp;
                ans.push(get_variable_value((var)->var));
            } else if (exp->type == IT_BOOL) {
                Bool* bol = (Bool*)exp;
                ans.push(bol->t);
            } else {
                break;
            }
        }
    }
    return ans.top();
}


/*
 * real eval
 */
long long Interpreter::do_eval(AST* root)
{
    long long ans = 0;

    while (root != NULL) {
        switch (root->Type) {
            case OPT_ASSIGN: {
                set_variable_value(
                        root->lvalue,
                        simple_execute(
                                root->rvalue.first,
                                root->rvalue.second
                        )
                );
                break;
            }
            case OPT_IF: {
                if (simple_execute(
                        root->pred.first,
                        root->pred.second
                )) {
                    do_eval(root->block1);
                } else {
                    do_eval(root->block2);
                }
                break;
            }
            case OPT_WHILE: {
                while (simple_execute(
                        root->pred.first,
                        root->pred.second
                )) {
                    do_eval(root->block1);
                }
                break;
            }
            default: {
                break;
            }
        }
        root = root->next;
    }

    return ans;
}


/*
 *  eval
 */
bool Interpreter::eval()
{
    exp_index start = 0;
    parser();
    _ast_root = make_ast(start, _ast_queue.size());
    do_eval(_ast_root);
    return true;
}

void do_test()
{
    const char* s1 = "fact := 1 ;\
                    val := 10000 ;\
                    cur := val ;\
                    mod := 1000000007 ;\
                     \
                    while ( cur > 1 ) \
                    do \
                    { \
                    fact := fact * cur ;\
                    fact := fact - fact / mod * mod ;\
                    cur := cur - 1\
                    } ;\
                    cur := 0";
    const char* s2 = "a := 10 ;\
            b := 100 ; \
            if ( a < b ) then\
            {\
                    min := a ; \
                    max := b ; \
                    if ( max < b ) then \
                    { b := 99 ; } else { b := 77 } \
            }\
            else {\
                min := b ;\
                max := a\
                }";

    Interpreter d1(s1);
    d1.eval();
    d1.output();

    Interpreter d2(s2);
    d2.eval();
    d2.output();
}

int main()
{
    do_test();
    return 0;
}

