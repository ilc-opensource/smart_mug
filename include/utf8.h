#include <string>
#include <vector>
using namespace std;

typedef vector<unsigned int> unicode_uint_t;

bool utf8_to_unicode_uint(const string &src, vector<unsigned int> &dst);
char* utf8_to_unicode_char(char* src);
wchar_t* utf8_to_unicode_wchar(char* src);

