#include <cassert>
#include <iostream>
#include <string>
#include "test.h"
#include "pstsdk/ndb.h"
#include "pstsdk/ltp.h"

// this function works because the set of named props present in sample1.pst is known
void test_nameid_map_samp1(pstsdk::shared_db_ptr pdb)
{
    using namespace pstsdk;
    name_id_map nm(pdb);

    const guid g1 = { 0x20386, 0, 0, { 0xc0, 0, 0, 0, 0, 0, 0, 0x46 } };
    const guid g2 = { 0x62002, 0, 0, { 0xc0, 0, 0, 0, 0, 0, 0, 0x46 } };

    // is the count correct?
    assert(nm.get_prop_count() == 172);

    // test that the lookup succeeds and matches a few well known named props
    named_prop t1(ps_public_strings, L"urn:schemas-microsoft-com:office:outlook#storetypeprivate");
    assert(nm.lookup(t1) == 0x800f);
    assert(nm.lookup(0x800f).get_name() == L"urn:schemas-microsoft-com:office:outlook#storetypeprivate");

    named_prop t2(ps_public_strings, L"Keywords");
    assert(nm.lookup(t2) == 0x8012);
    assert(nm.lookup(0x8012).get_name() == L"Keywords");

    named_prop t3(g1, L"x-ms-exchange-organization-authdomain");
    assert(nm.lookup(t3) == 0x801c);
    assert(nm.lookup(0x801c).get_name() == L"x-ms-exchange-organization-authdomain");

    named_prop t4(g2, 0x8233);
    assert(nm.lookup(t4) == 0x8008);
    assert(nm.lookup(0x8008).get_id() == 0x8233);

    named_prop t5(g2, 0x8205);
    assert(nm.lookup(t5) == 0x8000);
    assert(nm.lookup(0x8000).get_id() == 0x8205);

    // test that the lookup fails for a few known-to-not-exist named props
    bool not_found = false;

    named_prop t6(ps_public_strings, L"fake-property");
    try
    {
        nm.lookup(t6);
    } 
    catch(key_not_found<named_prop>&)
    {
        not_found = true;
    }
    assert(not_found);
}

void test_prop_stream(pstsdk::const_property_object& obj, pstsdk::prop_id id)
{
    pstsdk::prop_stream stream(obj.open_prop_stream(id));
    std::vector<pstsdk::byte> contents = obj.read_prop<std::vector<pstsdk::byte> >(id);
    pstsdk::byte b;
    size_t pos = 0;

    assert(contents.size() == obj.size(id));

    stream.unsetf(std::ios::skipws);
    while(stream >> b)
        assert(b == contents[pos++]);
}

void test_table(const pstsdk::table& tc)
{
    using namespace std;
    using namespace pstsdk;

    wcout << "Properties on this table (" << tc.size() << "): " << endl;
    std::vector<prop_id> prop_list = tc.get_prop_list();
    for(pstsdk::uint i = 0; i < prop_list.size(); ++i)
        wcout << hex << prop_list[i] << " ";
    wcout << endl;

    for(pstsdk::uint i = 0; i < tc.size(); ++i)
    {
        wcout << "RowID: " << tc[i].get_row_id() << endl;
        wstring display_name;
        wstring subject;

         std::vector<pstsdk::ushort> proplist(tc[i].get_prop_list());
        for(pstsdk::uint j = 0; j < proplist.size(); ++j)
        {
            try {
                if(tc[i].get_prop_type(proplist[j]) == prop_type_wstring)
                {
                    wcout << "\t" << hex << proplist[j] << ": " << tc[i].read_prop<std::wstring>(proplist[j]) << endl;
                }
                else if(tc[i].get_prop_type(proplist[j]) == prop_type_long)
                {
                    wcout << "\t" << hex << proplist[j] << ": " << dec << tc[i].read_prop<slong>(proplist[j]) << endl;
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
                else if(tc[i].get_prop_type(proplist[j]) == prop_type_guid)
                {
                    guid g = tc[i].read_prop<guid>(proplist[j]);
                    (void) g;
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

void test_attachment_table(const pstsdk::node& message, const pstsdk::table& tc)
{
    using namespace std;
    using namespace pstsdk;
    for(pstsdk::uint i = 0; i < tc.size(); ++i)
    {
        node attach = message.lookup(tc[i].get_row_id());
        
        wcout << "Attachment " << i << endl;
        property_bag pc(attach);
            std::vector<pstsdk::ushort> proplist(pc.get_prop_list());
            for(pstsdk::uint i = 0; i < proplist.size(); ++i)
            {
                if(pc.get_prop_type(proplist[i]) == prop_type_wstring)
                {
                    wcout << "\t" << hex << proplist[i] << ": " << pc.read_prop<std::wstring>(proplist[i]) << endl;
                }
                else if(pc.get_prop_type(proplist[i]) == prop_type_long)
                {
                    wcout << "\t" << hex << proplist[i] << ": " << dec << pc.read_prop<slong>(proplist[i]) << endl;
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

void iterate(pstsdk::shared_db_ptr pdb)
{
    using namespace std;
    using namespace std::tr1;
    using namespace pstsdk;
    std::tr1::shared_ptr<const nbt_page> nbt_root = pdb->read_nbt_root();
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
            std::vector<pstsdk::ushort> proplist(bag.get_prop_list());

            // look for mv props
            for(pstsdk::uint i = 0; i < proplist.size(); ++i)
            {
                switch(bag.get_prop_type(proplist[i]))
                {
                case prop_type_mv_wstring:
                    {
                    vector<wstring> wstrings = bag.read_prop_array<wstring>(proplist[i]);
                    cout << "prop_type_mv_wstring" << endl;
                    for(size_t i = 0; i < wstrings.size(); ++i)
                        wcout << wstrings[i] << endl;
                    }
                    break;
                case prop_type_mv_string:
                    {
                    vector<string> strings = bag.read_prop_array<string>(proplist[i]);
                    cout << "prop_type_mv_string" << endl;
                    for(size_t i = 0; i < strings.size(); ++i)
                        cout << strings[i] << endl;
                    }
                    break;
                case prop_type_mv_binary:
                    {
                    vector<vector<byte> > bins = bag.read_prop_array<vector<byte> >(proplist[i]);
                    cout << "prop_type_mv_wstring" << endl;
                    }
                    break;
                default:
                    break;
                }
            }
        } 
        catch(exception&)
        {
        }


        if(get_nid_type(n.get_id()) == nid_type_message)
        {
            
            property_bag pc(n);
            std::vector<pstsdk::ushort> proplist(pc.get_prop_list());
            for(pstsdk::uint i = 0; i < proplist.size(); ++i)
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
            std::tr1::shared_ptr<bth_node<pstsdk::ushort, disk::prop_entry> > bth = h.open_bth<pstsdk::ushort, disk::prop_entry>(h.get_root_id());
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
    using namespace pstsdk;

    shared_db_ptr uni = open_database(L"test_unicode.pst");
    shared_db_ptr ansi = open_database(L"test_ansi.pst");
    shared_db_ptr samp1 = open_database(L"sample1.pst");
    shared_db_ptr samp2 = open_database(L"sample2.pst");
    shared_db_ptr submess = open_database(L"submessage.pst");

    iterate(uni);
    iterate(ansi);
    iterate(samp1);
    iterate(samp2);
    iterate(submess);

    // only valid to call on samp1
    test_nameid_map_samp1(samp1);
}
