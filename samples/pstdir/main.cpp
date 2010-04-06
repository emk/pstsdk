#include <iostream>        // wcout
#include <algorithm>       // for_each
#include <functional>      // bind

#include "pstsdk/pst.h"

using namespace pstsdk;
using namespace std;
using namespace std::placeholders;

void process_folder(int tab_depth, const folder& f)
{
    for(int i = 0; i < tab_depth; ++i) cout << '\t';

    wcout << f.get_name() << L" (" << f.get_message_count() << L")\n";
    
    for_each(f.sub_folder_begin(), f.sub_folder_end(), bind(process_folder, tab_depth+1, _1)); 
}

int main(int, char** argv)
{
    string path(argv[1]);
    wstring wpath(path.begin(), path.end());
    pst store(wpath);

    process_folder(0, store.open_root_folder());
}
