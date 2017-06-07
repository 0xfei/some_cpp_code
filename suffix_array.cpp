#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

void normal_suffix_array(const string& s)
{
    vector<string> sa;
    string t;

    for (int i = s.size()-1; i >= 0; --i) {
        char v[2] = {s[i], 0};
        t = string(v) + t;
        sa.push_back(t);
    }

    sort(sa.begin(), sa.end());

    for (string::size_type i = 0; i < sa.size(); ++i) {
        cout << sa[i] << endl;
    }
}


void build_suffix_array(const string& s)
{
    static const int LEN = 256;
    
    int sa[LEN] = {0};
    int rank[LEN] = {0};
    int height[LEN] = {0};

    int num[LEN] = {0};

    for (string::size_type i = 0; i < s.size(); ++i) {
        num[s[i]] = 1;
    }
    for (int i = 1; i < LEN; ++i) {
        num[i] += num[i-1];
    }
    for (string::size_type i = 0; i < s.size(); ++i) {
        rank[i] = num[s[i]];
    }

    for (string::size_type k = 1; k < s.size(); k <<= 1) {
        memset(num, 0, sizeof(num));
        for (string::size_type i = 0; i < s.size(); ++i) {
            num[rank[i]*10 + rank[i+k]] = 1; // i + k may greater than s.size()
        }
        for (int i = 1; i < 100; ++i) {
            num[i] += num[i-1];
        }
        for (string::size_type i = 0; i < s.size(); ++i) {
            rank[i] = num[rank[i]*10 + rank[i+k]];
        }
    }

    for (string::size_type i = 0; i < s.size(); ++i) {
        rank[i] = rank[i] - 1;
        sa[rank[i]] = i;
        cout << rank[i] << endl;
    }
    for (string::size_type i = 0; i < s.size(); ++i) {
        cout << s.substr(sa[i], s.size()) << endl;
    }

    int k = 0;
    for (string::size_type i = 0; i < s.size(); ++i) {
        if (rank[i] == 0) {
            height[0] = 0;
            continue;
        } else {
            if (k) {
                --k;
            }
            string::size_type j = sa[rank[i]-1];
            while (i + k < s.size() && 
                j + k < s.size() && 
                s[i+k] == s[j+k]) {
                    ++k;
            }
            height[rank[i]] = k;
        }
    }
    for (string::size_type i = 0; i < s.size(); ++i) {
        cout << height[i] << endl;
    }
}

int main()
{
    string s;
    cin >> s;

    build_suffix_array(s);

    return 0;
}
