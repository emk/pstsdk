#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "test.h"
#include <iostream>

int main()
{
    using namespace std;

    cout << "test_btree();" << endl;
    test_btree();
    cout << "test_disk();" << endl;
    test_disk();
    cout << "test_db();" << endl;
    test_db();
    cout << "test_highlevel();" << endl;
    test_highlevel();

#ifdef _MSC_VER
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
    _CrtDumpMemoryLeaks();
#endif
}
