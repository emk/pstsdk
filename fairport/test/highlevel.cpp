#include <cassert>
#include <iostream>
#include <string>
#include "test.h"
#include "fairport/ndb.h"
#include "fairport/ltp.h"

void test_table(const fairport::table& tc)
{
    using namespace std;
    using namespace fairport;

    wcout << "Properties on this table: ";
    std::vector<prop_id> prop_list = tc.get_prop_list();
    for(uint i = 0; i < prop_list.size(); ++i)
        wcout << hex << prop_list[i] << " ";
    wcout << endl;

    for(uint i = 0; i < tc.size(); ++i)
    {
        wcout << "RowID: " << tc[i].row_id() << endl;
        wstring display_name;
        wstring subject;

        try
        {
            display_name = tc[i].read_prop<wstring>(0x3001);
        }
        catch(...)
        {
            wcout << "display name not found..." << endl;
        }

        try
        {
            subject = tc[i].read_prop<wstring>(0x37);
        }
        catch(...)
        {
            wcout << "subject not found..." << endl;
        }

        wcout << "\tSubject: " << subject << endl;
        wcout << "\tDisplay Name: " << display_name << endl;
    }
}

void iterate(fairport::shared_db_ptr pdb)
{
    using namespace std;
    using namespace fairport;
	shared_ptr<const nbt_page> nbt_root = pdb->read_nbt_root();
    for(const_node_iterator iter = nbt_root->begin();
            iter != nbt_root->end();
            ++iter)
    {
        node n(pdb, *iter);
        std::vector<byte> buffer(n.size());
        n.read(buffer, 0);

        try
        {
            property_bag bag(n);
            std::vector<ushort> proplist(bag.get_prop_list());
        } 
        catch(exception&)
        {
        }


        if(get_nid_type(n.get_id()) == nid_type_message)
        {
            property_bag pc(n);
            std::vector<ushort> proplist(pc.get_prop_list());
            for(uint i = 0; i < proplist.size(); ++i)
            {
                if(pc.get_prop_type(proplist[i]) == prop_type_wstring)
                {
                    wcout << pc.read_prop<std::wstring>(proplist[i]) << endl;
                }
            }
        }
    
        try{
            heap h(n);
            bth_node<ushort, disk::prop_entry>* bth = h.open_bth<ushort, disk::prop_entry>(h.get_root_id());

            delete bth;
         }
        catch(exception&)
        {
        }

        if(get_nid_type(n.get_id()) == nid_type_contents_table)
        {
            table tc(n);
            wcout << "Found TC: " << tc.size() << endl;
            test_table(tc);
        }
        else if(get_nid_type(n.get_id()) == nid_type_associated_contents_table)
        {
            table tc(n);
            wcout << "Found Associated TC: " << tc.size() << endl;
            test_table(tc);
        }
        else if(get_nid_type(n.get_id()) == nid_type_hierarchy_table)
        {
            table tc(n);
            wcout << "Found Hierarchy TC: " << tc.size() << endl;
            test_table(tc);
        }
    }
}

void test_highlevel()
{
    using namespace fairport;

    shared_db_ptr uni = open_database(L"test_unicode.pst");
    shared_db_ptr ansi = open_database(L"test_ansi.pst");

    iterate(uni);
    iterate(ansi);
}
