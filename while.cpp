#include <iostream>
#include <cctype>
#include <cstdlib>
#include <string>
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
const string CS_END(";");

typedef enum {
    IT_OPERATOR,
    IT_VARIABLE,
    IT_BOOL,
    IT_NUMBER,
    IT_END,
} ITYPE;

struct Term {
    ITYPE type;
};

struct Operator : Term {
    string v;
};

struct Variable : Term {
    string var;
};

struct Number : Term {
    int number;
};

struct Bool : Term {
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


/*
 *  global environment
 */
map<string, int> environment;


/*
 *  inline helper function 
 */
inline bool is_variable(string exp)
{
    return (!exp.empty() && isalpha(exp[0]));
}

inline bool is_number(string exp)
{
    return (!exp.empty() && !is_variable(exp));
}

int to_number(string exp)
{
    return atoi(exp.c_str());
}

inline bool is_space(char c)
{
    return c == ' ';
}

inline bool is_linend(char c)
{
    return (c == ';') || (c == '\n');
}

inline void skip_space(
        int &i, 
        const int len,
        const string &exp)
{
    while (i < len && is_space(exp[i])) {
        ++i;
    }
}


/*
 *  get item , return false if exp end
 */
bool get_item(string exp, string &item, string &left)
{
    int i = 0, len = exp.size();

    item.clear();
    left.clear();
    skip_space(i, len, exp);

    if (i == len) {
        return false;
    }

    while (i < len && !is_space(exp[i]) && !is_linend(exp[i])) {
        item = item + exp[i];
        ++i;
    }

    skip_space(i, len, exp);

    if (i == len) {
        return false;
    }

    if (is_linend(exp[i])) {
        left = exp.substr(i+1);
        return false;
    } else {
        left = exp.substr(i);
        return true;
    }
}


/*
 *  environment
 */
inline void set_variable_value(const string &v, int value)
{
    environment[v] = value;
}

inline int get_variable_value(const string &v)
{
    return environment[v];
}


/*
 *  parse_item
 */
Term* parse_item(string &exp, bool &still)
{
    Term* ast = NULL;

    string left_exp = exp, item, label;
    still = get_item(exp, item, left_exp);

    // debug
    cout << item << ' ';

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

    exp = left_exp;
    return ast;
}


/*
 *  parser, split source code to terms
 */
bool parser(exp_vector& ast_queue, string& exp)
{
    static Term s_end;
    s_end.type = IT_END;

    while (!exp.empty()) {
        bool still;
        Term* item = parse_item(exp, still);
        if (item) {
            ast_queue.push_back(item);
        }
        if (!still) {
            ast_queue.push_back(&s_end);

            // debug
            cout << ";" << endl;
        }
    }

    return true;
}


/*
 *  make_ast
 */
inline exp_index find_else(const exp_vector& ast_queue, exp_index start, exp_index end)
{
    while (start < end) {
        Operator* opt = (Operator*)ast_queue[start];
        if (opt->v == CS_ELSE) {
            return start;
        }
        ++start;
    }
    return end;
}
inline exp_index find_then(const exp_vector& ast_queue, exp_index start, exp_index end)
{
    while (start < end) {
        Operator* opt = (Operator*)ast_queue[start];
        if (opt->v == CS_THEN) {
            return start;
        }
        ++start;
    }
    return end;
}

inline exp_index find_do(const exp_vector& ast_queue, exp_index start, exp_index end)
{
    while (start < end) {
        Operator* opt = (Operator*)ast_queue[start];
        if (opt->v == CS_DO) {
            return start;
        }
        ++start;
    }
    return end;
}

inline exp_index find_end(const exp_vector& ast_queue, exp_index start, exp_index end)
{
    while (start < end) {
        Operator* opt = (Operator*)ast_queue[start];
        if (opt->v == CS_END) {
            return start;
        }
        ++start;
    }
    return end;
}

AST* make_ast(exp_vector& ast_queue, exp_index& start, const exp_index end)
{
    AST* ast_root = new AST;
    ast_root->Type = OPT_START;

    AST* last = ast_root;

    string lvalue;

    while (start < end) {
        Term* item = ast_queue[start];
        if (item->type == IT_VARIABLE) {
            Variable* var = (Variable*)item;
            lvalue = var->var;
            ++start;
        } else if (item->type == IT_OPERATOR) {
            Operator* opt = (Operator*)item;
            if (opt->v == CS_IF) { // if
                exp_index next = find_then(ast_queue, start, end);
                AST* item = new AST;
                item->Type = OPT_IF;
                item->next = NULL;
                item->pred.first = start + 1;
                item->pred.second = next;

                last->next = item;
                last = item;
                start = next + 1;
            } else if (opt->v == CS_WHILE) { // while
                exp_index next = find_do(ast_queue, start, end);
                AST* item = new AST;
                item->Type = OPT_WHILE;
                item->next = NULL;
                item->pred.first = start + 1;
                item->pred.second = next;

                last->next = item;
                last = item;
                start = next + 1;
            } else if (opt->v == CS_ASN) { // assign
                exp_index next = find_end(ast_queue, start, end);
                AST* item = new AST;
                item->Type = OPT_ASSIGN;
                item->lvalue = lvalue;
                item->rvalue.first = start + 1;
                item->rvalue.second = next;
                item->next = NULL;
                last->next = item;
                last = item;
                start = next + 1;
            } else {
                // should be error!
                ++start;
            }
        } else /*IT_BOOL IT_NUMBER IT_END*/ {
            lvalue.clear();
            ++start;
        }
    }

    return ast_root;
}


/*
 *  weight, calculate expressions
 */
int weight(const Operator *exp)
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

int simple_execute(exp_vector &ast_queue, const exp_index start, const exp_index end)
{
	deque<Term*> temp;
	stack<Term*> lque;
	stack<int> ans;

	for (exp_index i = start; i < end; ++i) {
        Term* exp = ast_queue[i];

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

			int a, b = ans.top();
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
 *  eval
 */
bool eval(string exp)
{
    exp_vector ast_queue;
    parser(ast_queue, exp);

    exp_index start = 0;
    AST* ast_root = NULL;
    ast_root = make_ast(ast_queue, start, ast_queue.size());

    environment.clear();

    return true;
}

void do_test()
{
    const char* s = "fact := 1 ;\
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

    const char* rvalue = "3 - ( 451 + 2 ) - ( 2 * 99 ) ; ";
    eval(s);
}

int main()
{
    do_test();

    return 0;
}
