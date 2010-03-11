#include <cassert>
#include <iostream>
#include <string>
#include "test.h"

#include <algorithm>
#include <fstream>

#include "fairport/ndb/database.h"
#include "fairport/ndb/database_iface.h"
#include "fairport/ndb/page.h"

#include "fairport/pst/message.h"
#include "fairport/pst/folder.h"
#include "fairport/pst/pst.h"

void process_recipient(const fairport::recipient& r)
{
    using namespace std;
    using namespace fairport;
    
    wcout << "\t\t" << r.get_name() << "(" << r.get_email_address() << ")\n";
}

void process_attachment(const fairport::attachment& a)
{
    using namespace std;
    using namespace fairport;
    
    wcout << "\t\t" << a.get_filename() << endl;

    ofstream newfile(a.get_filename(), ios::out | ios::binary);
    newfile << a;
}

void process_message(const fairport::message& m)
{
    using namespace std;
    using namespace fairport;

    wcout << "Message Subject: " << m.get_subject() << endl;
    wcout << "\tAttachment Count: " << m.get_attachment_count() << endl;

    if(m.get_attachment_count() > 0)
    {
        for_each(m.attachment_begin(), m.attachment_end(), [](const attachment& a) { 
            process_attachment(a);
        });
    }

    wcout << "\tRecipient Count: " << m.get_recipient_count() << endl;

    if(m.get_recipient_count() > 0)
    {
        for_each(m.recipient_begin(), m.recipient_end(), [](const recipient& r) { 
            process_recipient(r);
        });
    }
}


void process_folder(const fairport::folder& f)
{
    using namespace std;
    using namespace fairport;

    wcout << "Folder (M" << f.get_message_count() << ", F" << f.get_subfolder_count() << ") : " << f.get_name() << endl;

    for_each(f.message_begin(), f.message_end(), [](const message& m) {
        process_message(m);
    });

    for_each(f.sub_folder_begin(), f.sub_folder_end(), [](const folder& f) {
        process_folder(f);
    });

}

void process_pst(const fairport::pst& p)
{
    using namespace std;
    using namespace fairport;

    wcout << "PST Name: " << p.get_name() << endl;
    folder root = p.open_root_folder();
    process_folder(root);
}

void test_pstlevel()
{
    using namespace fairport;

    pst uni(L"test_unicode.pst");
    pst ansi(L"test_ansi.pst");
    pst uni2(L"test2.pst");

    process_pst(uni);
    process_pst(ansi);
    process_pst(uni2);

    // make sure searching by name works
    process_folder(uni.open_folder(L"Folder"));
}
