#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "pstsdk/util.h"

void test_bytes_to_wstring()
{
    using namespace pstsdk;

    // Convert to and from Windows Unicode bytestring.
    const byte abc_data[] = { 'a', 0, 'b', 0, 'c', 0 };
    std::vector<byte> abc_bytes(abc_data, abc_data + 6);
    std::wstring abc_wstring(L"abc");
    assert(wstring_to_bytes(abc_wstring) == abc_bytes);
    assert(abc_wstring == bytes_to_wstring(abc_bytes));

    // Handle zero-length strings.
    assert(wstring_to_bytes(std::wstring()).size() == 0);
    assert(bytes_to_wstring(std::vector<byte>()).size() == 0);
}

void test_codepage_1252_to_wstring()
{
    using namespace pstsdk;
    
    assert(codepage_1252_to_wstring("") == L"");
    assert(codepage_1252_to_wstring("ab\xa7") == L"ab\u00a7");
    // Handle the character block with non-trivial Unicode mappings.
    assert(codepage_1252_to_wstring("\x80\x81\x82\x83\x84\x85\x86\x87") ==
           L"\u20ac\x0081\u201a\u0192\u201e\u2026\u2020\u2021");
    assert(codepage_1252_to_wstring("\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f") ==
           L"\u02c6\u2030\u0160\u2039\u0152\x008d\u017d\x008f");
    assert(codepage_1252_to_wstring("\x90\x91\x92\x93\x94\x95\x96\x97") ==
           L"\x0090\u2018\u2019\u201c\u201d\u2022\u2013\u2014");
    assert(codepage_1252_to_wstring("\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f") ==
           L"\u02dc\u2122\u0161\u203a\u0153\x009d\u017e\u0178");
}

void test_util()
{
    test_bytes_to_wstring();
    test_codepage_1252_to_wstring();
}
