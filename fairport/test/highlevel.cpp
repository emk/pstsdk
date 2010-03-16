#include <cassert>
#include <iostream>
#include <string>
#include "test.h"
#include "fairport/ndb.h"
#include "fairport/ltp.h"

void test_prop_stream(fairport::const_property_object& obj, fairport::prop_id id)
{
    fairport::prop_stream stream = obj.open_prop_stream(id);
    std::vector<fairport::byte> contents = obj.read_prop<std::vector<fairport::byte>>(id);
    fairport::byte b;
    size_t pos = 0;

    stream.unsetf(std::ios::skipws);
    while(stream >> b)
        assert(b == contents[pos++]);
}

void test_table(const fairport::table& tc)
{
    using namespace std;
    using namespace fairport;

    wcout << "Properties on this table (" << tc.size() << "): " << endl;
    std::vector<prop_id> prop_list = tc.get_prop_list();
    for(uint i = 0; i < prop_list.size(); ++i)
        wcout << hex << prop_list[i] << " ";
    wcout << endl;

    for(uint i = 0; i < tc.size(); ++i)
    {
        wcout << "RowID: " << tc[i].get_row_id() << endl;
        wstring display_name;
        wstring subject;

         std::vector<ushort> proplist(tc[i].get_prop_list());
        for(uint j = 0; j < proplist.size(); ++j)
        {
            try {
                if(tc[i].get_prop_type(proplist[j]) == prop_type_wstring)
                {
                    wcout << "\t" << hex << proplist[j] << ": " << tc[i].read_prop<std::wstring>(proplist[j]) << endl;
                }
                else if(tc[i].get_prop_type(proplist[j]) == prop_type_long)
                {
                    wcout << "\t" << hex << proplist[j] << ": " << dec << tc[i].read_prop<long>(proplist[j]) << endl;
                }
                else if(tc[i].get_prop_type(proplist[j]) == prop_type_boolean)
                {
                    wcout << "\t" << hex << proplist[j] << ": " << dec << (tc[i].read_prop<bool>(proplist[j]) ? L"true" : L"false") << endl;
                }
                else if(tc[i].get_prop_type(proplist[j]) == prop_type_binary)
                {
                    const_table_row row = tc[i];
                    test_prop_stream(row, proplist[j]);
                }
                else
                {
                    wcout << "\t" << hex << proplist[j] << "(" << dec << tc[i].get_prop_type(proplist[j]) << ")" << endl;
                }
            } catch(key_not_found<prop_id>&)
            {
            }
        }
    }
}

void test_attachment_table(const fairport::node& message, const fairport::table& tc)
{
    using namespace std;
    using namespace fairport;
    for(uint i = 0; i < tc.size(); ++i)
    {
        node attach = message.lookup(tc[i].get_row_id());
        
        wcout << "Attachment " << i << endl;
        property_bag pc(attach);
            std::vector<ushort> proplist(pc.get_prop_list());
            for(uint i = 0; i < proplist.size(); ++i)
            {
                if(pc.get_prop_type(proplist[i]) == prop_type_wstring)
                {
                    wcout << "\t" << hex << proplist[i] << ": " << pc.read_prop<std::wstring>(proplist[i]) << endl;
                }
                else if(pc.get_prop_type(proplist[i]) == prop_type_long)
                {
                    wcout << "\t" << hex << proplist[i] << ": " << dec << pc.read_prop<long>(proplist[i]) << endl;
                }
                else if(pc.get_prop_type(proplist[i]) == prop_type_boolean)
                {
                    wcout << "\t" << hex << proplist[i] << ": " << dec << (pc.read_prop<bool>(proplist[i]) ? L"true" : L"false") << endl;
                }
                else if(pc.get_prop_type(proplist[i]) == prop_type_binary)
                {
                    test_prop_stream(pc, proplist[i]);
                }
                else
                {
                    wcout << "\t" << hex << proplist[i] << "(" << dec << pc.get_prop_type(proplist[i]) << ")" << endl;
                }
            }

    }
}

void iterate(fairport::shared_db_ptr pdb)
{
    using namespace std;
    using namespace fairport;
    shared_ptr<const nbt_page> nbt_root = pdb->read_nbt_root();
    for(const_nodeinfo_iterator iter = nbt_root->begin();
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
                    /*wcout <<*/ pc.read_prop<std::wstring>(proplist[i])/* << endl*/;
                }
                else if(pc.get_prop_type(proplist[i]) == prop_type_binary)
                {
                    test_prop_stream(pc, proplist[i]);
                }
            }

            // attachment table
            for(const_subnodeinfo_iterator si = n.subnode_info_begin();
                    si != n.subnode_info_end();
                    ++si)
            {
                if(get_nid_type(si->id) == nid_type_attachment_table)
                {
                    table atc(node(n, *si));
                    wcout << "Found Attachment Table: " << atc.size() << endl;
                    test_table(atc);
                    test_attachment_table(n, atc);
                }

                if(get_nid_type(si->id) == nid_type_recipient_table)
                {
                    table atc(node(n, *si));
                    wcout << "Found Recipient Table: " << atc.size() << endl;
                    test_table(atc);
                    //test_attachment_table(n, atc);
                }
            }

        }
        
        try{
            heap h(n);
            std::unique_ptr<bth_node<ushort, disk::prop_entry>> bth = h.open_bth<ushort, disk::prop_entry>(h.get_root_id());
         }
        catch(exception&)
        {
        }

        if(get_nid_type(n.get_id()) == nid_type_contents_table)
        {
            table tc(n);
            //wcout << "Found TC: " << tc.size() << endl;
            test_table(tc);
        }
        else if(get_nid_type(n.get_id()) == nid_type_associated_contents_table)
        {
            table tc(n);
            //wcout << "Found Associated TC: " << tc.size() << endl;
            test_table(tc);
        }
        else if(get_nid_type(n.get_id()) == nid_type_hierarchy_table)
        {
            table tc(n);
            //wcout << "Found Hierarchy TC: " << tc.size() << endl;
            test_table(tc);
        }
    }
}

void test_highlevel()
{
    using namespace fairport;

    shared_db_ptr uni = open_database(L"test_unicode.pst");
    shared_db_ptr ansi = open_database(L"test_ansi.pst");
    shared_db_ptr samp1 = open_database(L"sample1.pst");
    shared_db_ptr samp2 = open_database(L"sample2.pst");

    iterate(uni);
    iterate(ansi);
    iterate(samp1);
    iterate(samp2);
}
