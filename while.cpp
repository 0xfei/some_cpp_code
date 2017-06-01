#include <iostream>
#include <cctype>
#include <cstdlib>
#include <string>
#include <deque>
#include <stack>
#include <map>

using namespace std;

#define LIMIT	0x80000000

map<string, int> variable;

struct Exp {
    int type;
};

struct Var : Exp {
    string value;
};

struct Num : Exp {
    int number;
}

struct Assign : Exp {
    string var;
    Exp* value;
};

struct Opa : Exp {
    char opt;
    Exp* exp1;
    Exp* exp2;
}

struct Opb : Exp {
    bool and_or;
    Exp* exp1;
    Exp* exp2;
}

struct Opr : Exp{
    bool bg_ls;
    Exp* exp1;
    Exp* exp2;
}

struct If : Exp{
    Exp* pred;
    Exp* cons;
    Exp* altr;
}

struct While : Exp{
    Exp* pred;
    Exp* loop;
}


/*
 * is variable?
 */
inline bool is_variable?(string exp)
{
    return (!exp.empty() && isalpha(exp[0]));
}


/*
 * is number?
 */
inline bool is_number?(string exp)
{
    return (!exp.empty() && !is_variable?(exp));
}


/*
 * to number
 */
int to_number(string exp)
{
    return atoi(exp.c_str());
}


/*
 * skip space
 */
inline void skip_space(
        int &i, 
        const int len,
        const string &exp)
{
    while (i < len && isspace(exp[i++]));
}


/*
 * get item
 */
void get_item(string exp, string &item, string &left)
{
    string::size i = 0, len = exp.size();

    item.clear();
    left.clear();
    skip_space(i, len, exp);

    /* end of exp */
    if (i == len) {
        return;
    }

    while (i < len && !isspace(exp[i])) {
        item = item + exp[i];
        ++i;
    }

    skip_space(i, len, exp);

    /* end of exp */
    if (i == len) {
        return;
    }

    /* ; */
    if (exp[i] = ';') {
        left = exp.substr(i+1);
    } else {
        left = exp.substr(i);
    }
}


/*
 * variable define v := S
 */
bool is_defination?(
        string exp,
        Exp *exp,
        string &left)
{
    string left_exp, label;
    
    get_item(exp, variable, left_exp);
    
    if (!is_variable(variable)) {
        return false;
    }

    get_item(left_exp, label, left_exp);

    if (label != ":=") {
        return false;
    }

    get_item(left_exp, value, left);

    // execute value
    // exp = variable + value

    return true;
}


/*
 *
 */


/*
 * basic execution use stack
 */
int f(char fl)
{
    switch (fl) {
        case '+':
        case '-':
            return 0;
        case '*':
        case '/':
            return 1;
        case '(':
            return -1;
        default:
            return 2;
    }
}

int simple_execute(string s)
{
	deque<int> t;
	stack<int> q;
	stack<int> v;
	for (size_t i=0; i<s.size();) {
        if (isspace(s[i])) {
            ++i;
            continue;
        }
        if (isdigit(s[i])) {
            int k = 0;
            while (i < s.size() && isdigit(s[i])) {
                k = k*10 + s[i] - '0';
                ++i;
            }
            t.push_back(k);
            continue;
        }
		if (q.empty()) {
			q.push(s[i++]);
			continue;
		}

		if (s[i] == '(') {
			q.push(s[i++]);
			continue;
		}

		char tp;
		if (s[i] == ')') {
			while ((tp = q.top()) != '(') {
				t.push_back(tp | LIMIT);
				q.pop();
			}
			q.pop();
			++i;
			continue;
		}

		while (!q.empty() && f(q.top()) >= f(s[i])) {
			t.push_back(q.top() | LIMIT);
			q.pop();
		}
		q.push(s[i++]);
	}
	while (!q.empty()) {
		t.push_back(q.top() | LIMIT);
		q.pop();
	}

	for (size_t i=0; i<t.size(); ++i) {
		if (t[i] & LIMIT) {
			char c = static_cast<char>(t[i] & ~LIMIT);
			int a, b;
			b = v.top();
			v.pop();
			if (v.empty()) {
				a = 0;
			} else {
				a = v.top();
				v.pop();
			}
			switch (c) {
				case '+':
					v.push(a+b);
					break;
				case '-':
					v.push(a-b);
					break;
				case '*':
					v.push(a*b);
					break;
				case '/':
					v.push(a/b);
					break;
				default:
					break;
			}
		} else {
			v.push(t[i]);
		}
	}
	return v.top();
}

int main()
{
    string exp, s;
    
    while (cin >> s) {
        exp += s;
    }

    cout << simple_execute(exp) << endl;
    return 0;
}

