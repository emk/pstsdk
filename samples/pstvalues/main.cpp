#include "values.h"
#include <string>
#include <iostream>
#include <regex>
#include <stdexcept>

#ifdef USE_BOOST
namespace REGEX = boost;
#else
namespace REGEX = std::tr1;
#endif

void print_value(int i) 
{
    using namespace std;

    wcout << values[i].name;
    wcout << L": " << dec << values[i].v;
    wcout << ", 0x" << hex << values[i].v << endl;
}

int main(int argc, const char* argv[])
{
    try {
        std::vector<REGEX::wregex> expressions;
        
        for(int i = 1; i < argc; ++i)
        {
            std::string s(argv[i]);
            std::wstring w(s.begin(), s.end());
            expressions.push_back(REGEX::wregex(w));
        }

        for(unsigned int i = 0; i < sizeof(values)/sizeof(values[0]); ++i)
        {
            for(unsigned int j = 0; j < expressions.size(); ++j)
            {
                if(regex_search(std::wstring(values[i].name), expressions[j]))
                {
                    print_value(i);
                    break;
                }
            }
        }
    } catch(std::exception& e) {
        std::wcout << e.what() << std::endl;
    }
} 
