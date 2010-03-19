#ifndef FAIRPORT_LTP_NAMEID_H
#define FAIRPORT_LTP_NAMEID_H

#include <string>
#include <algorithm>

#include "fairport/util/primatives.h"

#include "fairport/ndb/database_iface.h"

#include "fairport/ltp/propbag.h"

namespace fairport
{

class named_prop
{
public:
    named_prop(const guid& g, long id) 
        : m_guid(g), m_is_string(false), m_id(id) { }
    named_prop(const guid& g, const std::wstring& name)
        : m_guid(g), m_is_string(true), m_name(name) { }

    const guid& get_guid() const { return m_guid; }
    bool is_string() const { return m_is_string; }
    long get_id() const { return m_id; }
    const std::wstring& get_name() const { return m_name; }

private:
    guid m_guid;
    bool m_is_string;
    long m_id;
    std::wstring m_name;
};

class name_id_map : private boost::noncopyable
{
public:
    name_id_map(const shared_db_ptr& db) 
        : m_bag(db->lookup_node(nid_name_id_map)), m_buckets(m_bag.read_prop<long>(0x1)), m_entry_stream(m_bag.open_prop_stream(0x3)), m_guid_stream(m_bag.open_prop_stream(0x2)), m_string_stream(m_bag.open_prop_stream(0x4)) { }

    // query if a given name/id exists
    bool name_exists(const guid& g, const std::wstring& name) const
        { return named_prop_exists(named_prop(g, name)); }
    bool id_exists(const guid& g, long id) const
        { return named_prop_exists(named_prop(g, id)); }
    bool named_prop_exists(const named_prop& p) const;
    bool prop_id_exists(prop_id id) const;
    size_t get_prop_count() const;

    std::vector<prop_id> get_prop_list() const;

    // the following methods throw key_not_found<named_prop> if the requested name/id doesn't exist
    prop_id lookup(const guid& g, const std::wstring& name) const
        { return lookup(named_prop(g, name)); }
    prop_id lookup(const guid& g, long id) const
        { return lookup(named_prop(g, id)); }
    prop_id lookup(const named_prop& p) const;

    // throws key_not_found<prop_id> if the requested prop_id hasn't been allocated
    named_prop lookup(prop_id id) const;

private:
    // helper functions
    named_prop construct(const disk::nameid& entry) const;
    named_prop construct(ulong index) const;
    guid read_guid(ushort guid_index) const;
    ushort get_guid_index(const guid& g) const;
    std::wstring read_wstring(ulong string_offset) const;
    ulong compute_hash_base(const named_prop& n) const 
        { return n.is_string() ? disk::compute_crc(&(n.get_name())[0], n.get_name().length() * sizeof(wchar_t)) : n.get_id(); }
    ulong compute_hash_value(ushort guid_index, const named_prop& n) const
        { return (n.is_string() ? ((guid_index << 1) | 1) : (guid_index << 1)) ^ compute_hash_base(n); }
    prop_id get_bucket_prop(ulong hash_value) const
        { return static_cast<prop_id>((hash_value % m_buckets) + 0x1000); }

    property_bag m_bag;
    ulong m_buckets;
    mutable prop_stream m_entry_stream;
    mutable prop_stream m_guid_stream;
    mutable prop_stream m_string_stream;
};

inline fairport::named_prop fairport::name_id_map::construct(const disk::nameid& entry) const
{
    if(nameid_is_string(entry))
        return named_prop(read_guid(disk::nameid_get_guid_index(entry)), read_wstring(entry.string_offset));
    else
        return named_prop(read_guid(disk::nameid_get_guid_index(entry)), entry.id);
}

inline fairport::named_prop fairport::name_id_map::construct(ulong index) const
{
    disk::nameid entry;
    m_entry_stream.seekg(index * sizeof(disk::nameid), std::ios_base::beg);
    m_entry_stream.read((char*)&entry, sizeof(entry));
    return construct(entry);
}

inline fairport::guid fairport::name_id_map::read_guid(ushort guid_index) const
{
    if(guid_index == 0)
        return ps_none;
    if(guid_index == 1)
        return ps_mapi;
    if(guid_index == 2)
        return ps_public_strings;

    guid g;
    m_guid_stream.seekg((guid_index-3) * sizeof(guid), std::ios_base::beg);
    m_guid_stream.read((char*)&g, sizeof(g));
    return g;
}

inline fairport::ushort fairport::name_id_map::get_guid_index(const guid& g) const
{
    if(memcmp(&g, &ps_none, sizeof(g)) == 0)
        return 0;
    if(memcmp(&g, &ps_mapi, sizeof(g)) == 0)
        return 1;
    if(memcmp(&g, &ps_public_strings, sizeof(g)) == 0)
        return 2;

    guid g_disk;
    ushort num = 0;
    m_guid_stream.seekg(0, std::ios_base::beg);
    while(m_guid_stream.read((char*)&g_disk, sizeof(g_disk)))
    {
        if(memcmp(&g, &g_disk, sizeof(g)) == 0)
            return num + 3;
        ++num;
    }

    // didn't find it
    m_guid_stream.clear();
    throw key_not_found<guid>(g);
}

inline std::wstring fairport::name_id_map::read_wstring(ulong string_offset) const
{
    m_string_stream.seekg(string_offset, std::ios_base::beg);

    ulong size;
    m_string_stream.read((char*)&size, sizeof(size));

    std::vector<char> buffer(size);
    m_string_stream.read(&buffer[0], size);

    return std::wstring(reinterpret_cast<wchar_t*>(&buffer[0]), size/sizeof(wchar_t));
}

inline bool fairport::name_id_map::named_prop_exists(const named_prop& p) const
{
    try 
    {
        lookup(p);
        return true;
    }
    catch(std::exception&)
    {
        return false;
    }
}

inline bool fairport::name_id_map::prop_id_exists(prop_id id) const
{
    if(id >= 0x8000)
        return static_cast<size_t>((id - 0x8000)) < get_prop_count();

    // id < 0x8000 is a ps_mapi prop
    return true;
}

inline std::vector<prop_id> fairport::name_id_map::get_prop_list() const
{
    disk::nameid entry;
    std::vector<prop_id> props;

    m_entry_stream.seekg(0, std::ios_base::beg);
    while(m_entry_stream.read((char*)&entry, sizeof(entry)) != 0)
        props.push_back(nameid_get_prop_index(entry) + 0x8000);

    m_entry_stream.clear();

    return props;
}

inline size_t fairport::name_id_map::get_prop_count() const
{
    m_entry_stream.seekg(0, std::ios_base::end);

    return static_cast<size_t>(m_entry_stream.tellg()) / sizeof(disk::nameid);
}

inline fairport::prop_id fairport::name_id_map::lookup(const named_prop& p) const
{
    ushort guid_index;
    
    try
    {
        guid_index = get_guid_index(p.get_guid());
    }
    catch(key_not_found<guid>&)
    {
        throw key_not_found<named_prop>(p);
    }

    // special handling of ps_mapi
    if(guid_index == 1)
    {
        if(p.is_string()) throw key_not_found<named_prop>(p);
        if(p.get_id() >= 0x8000) throw key_not_found<named_prop>(p);
        return static_cast<prop_id>(p.get_id());
    }

    ulong hash_value = compute_hash_value(guid_index, p);
    ulong hash_base = compute_hash_base(p);

    if(!m_bag.prop_exists(get_bucket_prop(hash_value)))
        throw key_not_found<named_prop>(p);

    prop_stream bucket(const_cast<name_id_map*>(this)->m_bag.open_prop_stream(get_bucket_prop(hash_value)));
    disk::nameid_hash_entry entry;
    while(bucket.read((char*)&entry, sizeof(entry)) != 0)
    {
        if( (entry.hash_base == hash_base) &&
            (disk::nameid_is_string(entry) == p.is_string()) &&
            (disk::nameid_get_guid_index(entry) == guid_index)
        )
        {
            // just double check the string..
            if(p.is_string())
            {
                if(construct(disk::nameid_get_prop_index(entry)).get_name() != p.get_name())
                    continue;
            }

            // found it!
            return disk::nameid_get_prop_index(entry) + 0x8000;
        }
    }

    throw key_not_found<named_prop>(p);
}

inline fairport::named_prop fairport::name_id_map::lookup(prop_id id) const
{
    if(id < 0x8000)
        return named_prop(ps_mapi, id);

    ulong index = id - 0x8000;

    if(index > get_prop_count())
        throw key_not_found<prop_id>(id);

    return construct(index);
}

} // end namespace fairport

#endif
