#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "pstsdk/util.h"

void test_wstring_conversion()
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

void test_util()
{
    test_wstring_conversion();
}
