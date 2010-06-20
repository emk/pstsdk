#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "pstsdk/util/primitives.h"

#include "pstsdk/ltp/propbag.h"

#include "pstsdk/pst/pst.h"

using namespace std;
using namespace pstsdk;

void pretty_print(const guid& g, bool special)
{
    cout << "{" << hex << setw(8) << setfill('0') << g.data1 << "-" << setw(4) << g.data2 << "-" << setfill('0') << setw(4) << g.data3 << "-";
    
    // next two bytes
    cout << hex << setw(2) << setfill('0') << (short)g.data4[0] << setw(2) << setfill('0') << (short)g.data4[1] << "-";

    // next six bytes
    for(int i = 2; i < 8; ++i)
        cout << hex << setw(2) << setfill('0')<< (short)g.data4[i];

    cout << (special ? "} *" : "}");
}

void pretty_print(const disk::nameid& id)
{
    cout << "id/str offset: " << setw(6) << id.id << ", guid idx: " << setw(2) << disk::nameid_get_guid_index(id) 
         << ", prop idx: " << setw(4) << disk::nameid_get_prop_index(id) 
         << ", is str: " << (disk::nameid_is_string(id) ? "y" : "n");
}

void pretty_print(const disk::nameid_hash_entry& id)
{
    cout << "hash value: " << setw(8) << hex << id.hash_base << ", guid idx: " << setw(2) << dec << disk::nameid_get_guid_index(id) 
         << ", prop idx: " << setw(4) << disk::nameid_get_prop_index(id) 
         << ", is str: " << (disk::nameid_is_string(id) ? "y" : "n");
}

int main() 
{
    pst p(L"../../test/sample2.pst");
    property_bag names(p.get_db()->lookup_node(nid_name_id_map));

    //
    // dump out the bucket count
    //
    // The bucket count is stored in property 0x0001
    cout << "Bucket Count: " << names.read_prop<slong>(0x1) << endl;

    //
    // dump out the guid stream
    //
    // The GUID Stream is a flat array of guid structures, with three
    // predefined values. Because of the predefined values, structures
    // referencing the GUID stream refer to the first entry as entry "3"
    prop_stream guid_stream(names.open_prop_stream(0x2));
    guid g;
    int i = 3;
    
    cout << endl << "GUID Stream: " << endl;

    cout << "[000] ";
    pretty_print(ps_none, true);
    cout << "\n[001] ";
    pretty_print(ps_mapi, true);
    cout << "\n[002] ";
    pretty_print(ps_public_strings, true);
    cout << endl;

    while(guid_stream.read((char*)&g, sizeof(g)) != 0)
    {
        cout << dec << setfill('0') << "[" << setw(3) << i++ << "] ";
        pretty_print(g, false);
        cout << endl;
    }

    cout << "* predefined guid, not actually present in guid stream" << endl;

    //
    // dump out the entry stream
    //
    // The entry stream is a flat array of nameid structures
    prop_stream entry_stream(names.open_prop_stream(0x3));
    disk::nameid id;
    i = 0;

    cout << endl << "Entry Stream: " << endl;
    while(entry_stream.read((char*)&id, sizeof(id)) != 0)
    {
        cout << dec << setfill('0') << "[" << setw(5) << i++ << "] ";
        pretty_print(id);
        cout << endl;
    }

    //
    // dump out the string stream
    //
    // The string stream is a series of long values (4 bytes), which give
    // the length of the immediately following string, followed by padding
    // to an 8 byte boundry, then another long for the next string, etc. 
    // When structures address into the string stream, they are giving the 
    // offset of the long value for the length of the string they want to access.
    prop_stream sstream(names.open_prop_stream(0x4));
    ulong size;
    std::vector<char> buffer;

    cout << endl << "String Stream (offset, size: string) : " << endl;
    while(sstream.read((char*)&size, sizeof(size)) != 0)
    {
        if(buffer.size() < size)
            buffer.resize(size);

        sstream.read((char*)&buffer[0], size);

        std::wstring val(reinterpret_cast<wchar_t*>(&buffer[0]), size/sizeof(wchar_t));
        wcout << setw(6) << ((size_t)sstream.tellg() - size - 4) << ", " << setw(4) << size << ": " << val << endl;

        size_t pos = (size_t)sstream.tellg();
        pos = (pos + 3L & ~3L);
        sstream.seekg(pos);
    }

    //
    // dump out the hash buckets
    //
    // Each hash bucket is an array of what basically are the same nameid structures
    // in the entry array, except the first field contains the crc of the string instead
    // of the string offset. For named props based on identifiers the value is identical.
    cout << endl << "Buckets:" << endl;
    prop_id max_bucket = (prop_id)(0x1000 + names.read_prop<slong>(0x1));
    for(prop_id p = 0x1000; p < max_bucket; ++p)
    {
        cout << "[" << setw(4) << hex << p << "] ";

        try
        {
            prop_stream hash_values(names.open_prop_stream(p));

            // calculate the number of values
            hash_values.seekg(0, ios_base::end);
            size_t size = (size_t)hash_values.tellg();
            size_t num = size/sizeof(disk::nameid_hash_entry);
            cout << num << (num == 1 ? " entry" : " entries") << endl;

            int i = 0;
            disk::nameid_hash_entry entry;
            hash_values.seekg(0, ios_base::beg);
            while(hash_values.read((char*)&entry, sizeof(entry)) != 0)
            {
                cout << "\t" << dec << setfill('0') << "[" << setw(4) << i++ << "] ";
                pretty_print(entry);
                cout << endl;
            }
        } 
        catch(key_not_found<prop_id>&)
        {
            // doesn't exist
            cout << "Empty" << endl;
        }
    }

    //
    // dump out all the named props
    //
    // Bring it all together. Iterate over the entry stream again, this time
    // fetching the proper values from the guid and the string streams.
    guid_stream.clear();
    guid_stream.seekg(0, ios_base::beg);
    entry_stream.clear();
    entry_stream.seekg(0, ios_base::beg);
    sstream.clear();
    sstream.seekg(0, ios_base::beg);
    i = 0;

    cout << endl << "Named Props: " << endl;
    while(entry_stream.read((char*)&id, sizeof(id)) != 0)
    {
        // read the guid
        if(disk::nameid_get_guid_index(id) == 0)
        {
            g = ps_none;
        }
        else if(disk::nameid_get_guid_index(id) == 1)
        {
            g = ps_mapi;
        }
        else if(disk::nameid_get_guid_index(id) == 2)
        {
            g = ps_public_strings;
        }
        else
        {
            // note how you need to subtract 3 from the guid index to account for the
            // three predefined guids not stored in the guid stream
            guid_stream.seekg((disk::nameid_get_guid_index(id) - 3) * sizeof(g), ios_base::beg);
            guid_stream.read((char*)&g, sizeof(g));
        }

        cout << "Name Prop 0x" << hex << setw(4) << (0x8000 + disk::nameid_get_prop_index(id)) << endl;
        cout << "\t" << dec << disk::nameid_get_guid_index(id) << ": ";
        pretty_print(g, false);
        cout << endl;

        if(disk::nameid_is_string(id))
        {
            cout << "\tkind: string" << endl;

            // read the string
            sstream.seekg(id.string_offset, ios_base::beg);
            sstream.read((char*)&size, sizeof(size));

            if(buffer.size() < size)
            buffer.resize(size);

            sstream.read((char*)&buffer[0], size);

            std::wstring val((wchar_t*)&buffer[0], (wchar_t*)&buffer[size]);
            wcout << L"\tname: " << val << endl;
        }
        else
        {
            cout << "\tkind: id" << endl;
            cout << "\tid: " << id.id << endl;
        }

        cout << endl;
    }
}
