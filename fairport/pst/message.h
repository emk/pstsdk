#ifndef FAIRPORT_PST_MESSAGE_H
#define FAIRPORT_PST_MESSAGE_H

#include <functional>
#include <ostream>
#include <boost/iterator/transform_iterator.hpp>

#include "fairport/ndb/database_iface.h"
#include "fairport/ndb/node.h"

#include "fairport/ltp/propbag.h"
#include "fairport/ltp/table.h"

namespace fairport
{

class message;
class attachment
{
public:
    // property access
    std::wstring get_filename() const;
    std::vector<byte> get_bytes() const
        { return m_bag.read_prop<std::vector<byte>>(0x3701); }
    hnid_stream_device open_byte_stream()
        { return m_bag.open_prop_stream(0x3701); }
    size_t size() const
        { return m_bag.read_prop<uint>(0xe20); }
    bool is_message() const
        { return m_bag.read_prop<uint>(0x3705) == 5; }
    message open_as_message() const;

    // lower layer access
    const property_bag& get_property_bag() const
        { return m_bag; }
    property_bag& get_property_bag()
        { return m_bag; }

private:
    attachment& operator=(const attachment&); // = delete
    friend class message;
    friend class attachment_transform;
    attachment(property_bag attachment)
        : m_bag(std::move(attachment)) { }

    property_bag m_bag;
};

std::ostream& operator<<(std::ostream& out, const attachment& attach)
{
    std::vector<byte> data = attach.get_bytes();
    out.write(reinterpret_cast<char*>(&data[0]), data.size());
    return out;
}

class attachment_transform : public std::unary_function<const_table_row, attachment>
{
public:
    attachment_transform(const node& n) 
        : m_node(n) { }
    attachment_transform(attachment_transform&& other)
        : m_node(std::move(other.m_node)) { }
    attachment operator()(const const_table_row& row) const
        { return attachment(property_bag(m_node.lookup(row.get_row_id()))); }

private:
    node m_node;
};

class recipient
{
public:
    // property access
    std::wstring get_name() const
        { return m_row.read_prop<std::wstring>(0x3001); }
    recipient_type get_type() const
        { return static_cast<recipient_type>(m_row.read_prop<ulong>(0xc15)); }
    std::wstring get_address_type() const
        { return m_row.read_prop<std::wstring>(0x3002); }
    std::wstring get_email_address() const
        { return m_row.read_prop<std::wstring>(0x39fe); }
    std::wstring get_account_name() const
        { return m_row.read_prop<std::wstring>(0x3a00); }

    // lower layer access
    const const_table_row& get_property_row() const
        { return m_row; }

private:
    recipient& operator=(const recipient&); // = delete
    friend struct recipient_transform;

    recipient(const const_table_row& row)
        : m_row(row) { }
    const_table_row m_row;
};

struct recipient_transform : public std::unary_function<const_table_row, recipient>
{
    recipient operator()(const_table_row row) const
        { return recipient(row); }
};

class message
{
public:
    typedef boost::transform_iterator<attachment_transform, const_table_row_iter> attachment_iterator;
    typedef boost::transform_iterator<recipient_transform, const_table_row_iter> recipient_iterator;

    message(const node& n)
        : m_bag(n) { }
    message(const message& other);
    message(message&& other)
        : m_bag(std::move(other.m_bag)), m_attachment_table(std::move(other.m_attachment_table)), m_recipient_table(std::move(other.m_recipient_table)) { }
     
    // subobject discovery/enumeration
    attachment_iterator attachment_begin() const
        { return boost::make_transform_iterator(get_attachment_table().begin(), attachment_transform(m_bag.get_node())); }
    attachment_iterator attachment_end() const
        { return boost::make_transform_iterator(get_attachment_table().end(), attachment_transform(m_bag.get_node())); }
    recipient_iterator recipient_begin() const
        { return boost::make_transform_iterator(get_recipient_table().begin(), recipient_transform()); }
    recipient_iterator recipient_end() const
        { return boost::make_transform_iterator(get_recipient_table().end(), recipient_transform()); }


    // property access
    std::wstring get_subject() const;
    std::wstring get_body() const
        { return m_bag.read_prop<std::wstring>(0x1000); }
    hnid_stream_device open_body_stream()
        { return m_bag.open_prop_stream(0x1000); }
    std::wstring get_html_body() const
        { return m_bag.read_prop<std::wstring>(0x1013); }
    hnid_stream_device open_html_body_stream()
        { return m_bag.open_prop_stream(0x1013); }
    size_t size() const
        { return m_bag.read_prop<long>(0xe08); }
    size_t get_attachment_count() const;
    size_t get_recipient_count() const;

    // lower layer access
    property_bag& get_property_bag()
        { return m_bag; }
    const property_bag& get_property_bag() const
        { return m_bag; }
    const table& get_attachment_table() const;
    const table& get_recipient_table() const;
    table& get_attachment_table();
    table& get_recipient_table();

private:
    message& operator=(const message&); // = delete

    property_bag m_bag;
    mutable std::unique_ptr<table> m_attachment_table;
    mutable std::unique_ptr<table> m_recipient_table;
};

class message_transform_row : public std::unary_function<const_table_row, message>
{
public:
    message_transform_row(const shared_db_ptr& db) 
        : m_db(db) { }
    message operator()(const const_table_row& row) const
        { return message(m_db->lookup_node(row.get_row_id())); }

private:
    shared_db_ptr m_db;
};

class message_transform_info : public std::unary_function<node_info, message>
{
public:
    message_transform_info(const shared_db_ptr& db) 
        : m_db(db) { }
    message operator()(const node_info& info) const
        { return message(node(m_db, info)); }

private:
    shared_db_ptr m_db;
};

} // end namespace fairport

inline std::wstring fairport::attachment::get_filename() const
{
    try
    {
        return m_bag.read_prop<std::wstring>(0x3707);
    } 
    catch(key_not_found<prop_id>&)
    {
        return m_bag.read_prop<std::wstring>(0x3704);
    }
}

inline fairport::message fairport::attachment::open_as_message() const
{
    if(!is_message()) 
        throw std::bad_cast();

    std::vector<byte> buffer = get_bytes();
    disk::sub_object* psubo = (disk::sub_object*)&buffer[0];

    return message(m_bag.get_node().lookup(psubo->nid));
}

inline fairport::message::message(const fairport::message& other)
: m_bag(other.m_bag)
{
    if(other.m_attachment_table)
        m_attachment_table.reset(new table(*other.m_attachment_table));
    if(other.m_recipient_table)
        m_recipient_table.reset(new table(*other.m_recipient_table));
}

inline const fairport::table& fairport::message::get_attachment_table() const
{
    if(!m_attachment_table)
        m_attachment_table.reset(new table(m_bag.get_node().lookup(nid_attachment_table)));

    return *m_attachment_table;
}

inline const fairport::table& fairport::message::get_recipient_table() const
{
    if(!m_recipient_table)
        m_recipient_table.reset(new table(m_bag.get_node().lookup(nid_recipient_table)));

    return *m_recipient_table;
}

inline fairport::table& fairport::message::get_attachment_table()
{
    return const_cast<table&>(const_cast<const message*>(this)->get_attachment_table());
}

inline fairport::table& fairport::message::get_recipient_table()
{
    return const_cast<table&>(const_cast<const message*>(this)->get_recipient_table());
}

inline size_t fairport::message::get_attachment_count() const
{
    size_t count = 0;
    try 
    {
        count = get_attachment_table().size();
    } 
    catch (const key_not_found<node_id>&) { }

    return count;
}

inline size_t fairport::message::get_recipient_count() const
{
    size_t count = 0;
    try
    {
        count = get_recipient_table().size();
    }
    catch (const key_not_found<node_id>&) { }

    return count;
}

inline std::wstring fairport::message::get_subject() const
{
    std::wstring buffer = m_bag.read_prop<std::wstring>(0x37);

    if(buffer[0] == message_subject_prefix_lead_byte)
    {
        // Skip the second chracter as well
        return buffer.substr(2);
    }
    else
    {
        return buffer;
    }
}

#endif
