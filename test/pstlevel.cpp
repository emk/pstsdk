#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>

#include "test.h"

#include "pstsdk/ndb/database.h"
#include "pstsdk/ndb/database_iface.h"
#include "pstsdk/ndb/page.h"

#include "pstsdk/pst/message.h"
#include "pstsdk/pst/folder.h"
#include "pstsdk/pst/pst.h"

void process_recipient(const pstsdk::recipient& r)
{
    using namespace std;
    using namespace pstsdk;

    wcout << "\t\t" << r.get_name() << "(" << r.get_email_address() << ")\n";
}

void process_message(const pstsdk::message& m);
void process_attachment(const pstsdk::attachment& a)
{
    using namespace std;
    using namespace pstsdk;

    wcout << "\t\t" << a.get_filename() << endl;

    if(a.is_message())
    {
        process_message(a.open_as_message());
    }
    else
    {
        std::wstring wfilename = a.get_filename();
        std::string filename(wfilename.begin(), wfilename.end());
        ofstream newfile(filename.c_str(), ios::out | ios::binary);
        newfile << a;

        std::vector<byte> contents = a.get_bytes();
        assert(contents.size() == a.content_size());
    }
}

void process_message(const pstsdk::message& m)
{
    using namespace std;
    using namespace pstsdk;

    wcout << "Message Subject: " << m.get_subject() << endl;
    wcout << "\tAttachment Count: " << m.get_attachment_count() << endl;

    if(m.get_attachment_count() > 0)
    {
        for_each(m.attachment_begin(), m.attachment_end(), process_attachment);
    }

    wcout << "\tRecipient Count: " << m.get_recipient_count() << endl;

    if(m.get_recipient_count() > 0)
    {
        for_each(m.recipient_begin(), m.recipient_end(), process_recipient);
    }
}


void process_folder(const pstsdk::folder& f)
{
    using namespace std;
    using namespace pstsdk;

    wcout << "Folder (M" << f.get_message_count() << ", F" << f.get_subfolder_count() << ") : " << f.get_name() << endl;

    for_each(f.message_begin(), f.message_end(), process_message);

    for_each(f.sub_folder_begin(), f.sub_folder_end(), process_folder);
}

void process_pst(const pstsdk::pst& p)
{
    using namespace std;
    using namespace pstsdk;

    wcout << "PST Name: " << p.get_name() << endl;
    folder root = p.open_root_folder();
    process_folder(root);
}

void test_entry_id(pstsdk::pst& sample1, pstsdk::pst& submessage) {
    using namespace std;
    using namespace pstsdk;

    // Folder "Sample1" (000000006a552b813c43f94384f18b7da2393e9582800000)
    folder f(*(--sample1.folder_end()));
    static const byte expected1_bytes[24] = {
        0x00, 0x00, 0x00, 0x00, 0x6a, 0x55, 0x2b, 0x81, 0x3c, 0x43, 0xf9, 0x43,
        0x84, 0xf1, 0x8b, 0x7d, 0xa2, 0x39, 0x3e, 0x95, 0x82, 0x80, 0x00, 0x00, 
    };
    vector<byte> expected1(expected1_bytes, expected1_bytes + 24);
    assert(expected1 == f.get_entry_id());

    // Message (000000006a552b813c43f94384f18b7da2393e9524002000)
    message m(*sample1.message_begin());
    static const byte expected2_bytes[24] = {
        0x00, 0x00, 0x00, 0x00, 0x6a, 0x55, 0x2b, 0x81, 0x3c, 0x43, 0xf9, 0x43,
        0x84, 0xf1, 0x8b, 0x7d, 0xa2, 0x39, 0x3e, 0x95, 0x24, 0x00, 0x20, 0x00, 
    };
    vector<byte> expected2(expected2_bytes, expected2_bytes + 24);
    assert(expected2 == m.get_entry_id());

    // Submessage should not have an entry ID.
    attachment a(*submessage.message_begin()->attachment_begin());
    message sm(a.open_as_message());
    bool raised_exception = false;
    try
    {
        sm.get_entry_id();
    }
    catch (key_not_found<prop_id> &)
    {
        raised_exception = true;
    }
    assert(raised_exception);
}

void test_pstlevel()
{
    using namespace pstsdk;

    pst uni(L"test_unicode.pst");
    pst ansi(L"test_ansi.pst");
    pst s1(L"sample1.pst");
    pst s2(L"sample2.pst");
    pst submess(L"submessage.pst");

    process_pst(uni);
    process_pst(ansi);
    process_pst(s1);
    process_pst(s2);
    process_pst(submess);

    // make sure searching by name works
    process_folder(uni.open_folder(L"Folder"));

    // make sure we can calculate entry_id values
    test_entry_id(s1, submess);
}
