#ifndef FAIRPORT_PST_FOLDER_H
#define FAIRPORT_PST_FOLDER_H

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "fairport/util/primatives.h"
#include "fairport/util/errors.h"

#include "fairport/ndb/database_iface.h"

#include "fairport/ltp/propbag.h"
#include "fairport/ltp/table.h"

#include "fairport/pst/message.h"

namespace fairport
{

template<node_id Type>
struct is_nid_type
{
    bool operator()(const node_info& info)
        { return get_nid_type(info.id) == Type; }
    bool operator()(const_table_row row)
        { return get_nid_type(row.row_id()) == Type; }
};

class search_folder
{
public:
    typedef boost::transform_iterator<message_transform_row, const_table_row_iter> message_iter;
    
    search_folder(const shared_db_ptr& db, const node& n)
        : m_db(db), m_bag(n) { }
    search_folder(const search_folder& other);

    // subobject discovery/enumeration
    message_iter message_begin() const
        { return boost::make_transform_iterator(get_contents_table().begin(), message_transform_row(m_db)); }
    message_iter message_end() const
        { return boost::make_transform_iterator(get_contents_table().begin(), message_transform_row(m_db)); }

    // property access
    std::wstring get_name() const
        { return m_bag.read_prop<std::wstring>(0x3001); }
    size_t get_message_count() const
        { return m_bag.read_prop<long>(0x3602); }

    // lower layer access
    property_bag& get_property_bag()
        { return m_bag; }
    const property_bag& get_property_bag() const
        { return m_bag; }
    shared_db_ptr get_db() const 
        { return m_db; }
    const table& get_contents_table() const;
    table& get_contents_table();

private:
    shared_db_ptr m_db;
    property_bag m_bag;
    mutable std::unique_ptr<table> m_contents_table;
};

class search_folder_transform : public std::unary_function<const_table_row, search_folder>
{
public:
    search_folder_transform(const shared_db_ptr& db) 
        : m_db(db) { }
    search_folder operator()(const_table_row row) const
        { return search_folder(m_db, m_db->lookup_node(row.row_id())); }

private:
    shared_db_ptr m_db;
};

class folder;
class folder_transform_row : public std::unary_function<const_table_row, folder>
{
public:
    folder_transform_row(const shared_db_ptr& db) 
        : m_db(db) { }
    folder operator()(const_table_row row) const;

private:
    shared_db_ptr m_db;
};

class folder
{
    typedef boost::filter_iterator<is_nid_type<nid_type_search_folder>, const_table_row_iter> search_folder_filter_iter;
    typedef boost::filter_iterator<is_nid_type<nid_type_folder>, const_table_row_iter> folder_filter_iter;

public:
    typedef boost::transform_iterator<message_transform_row, const_table_row_iter> message_iter;
    typedef boost::transform_iterator<folder_transform_row, folder_filter_iter> folder_iter;
    typedef boost::transform_iterator<search_folder_transform, search_folder_filter_iter> search_folder_iter;

    folder(const shared_db_ptr& db, const node& n)
        : m_db(db), m_bag(n) { }
    folder(const folder& other);

    // subobject discovery/enumeration
    folder_iter sub_folder_begin() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_folder>>(get_hierarchy_table().begin(), get_hierarchy_table().end()), folder_transform_row(m_db)); }
    folder_iter sub_folder_end() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_folder>>(get_hierarchy_table().end(), get_hierarchy_table().end()), folder_transform_row(m_db)); }

    search_folder_iter sub_search_folder_begin() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_search_folder>>(get_hierarchy_table().begin(), get_hierarchy_table().end()), search_folder_transform(m_db)); }
    search_folder_iter sub_search_folder_end() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_search_folder>>(get_hierarchy_table().begin(), get_hierarchy_table().end()), search_folder_transform(m_db)); }

    folder open_sub_folder(const std::wstring& name);

    message_iter message_begin() const
        { return boost::make_transform_iterator(get_contents_table().begin(), message_transform_row(m_db)); }
    message_iter message_end() const
        { return boost::make_transform_iterator(get_contents_table().end(), message_transform_row(m_db)); }

    message_iter associated_message_begin() const
        { return boost::make_transform_iterator(get_associated_contents_table().begin(), message_transform_row(m_db)); }
    message_iter associated_message_end() const
        { return boost::make_transform_iterator(get_associated_contents_table().end(), message_transform_row(m_db)); }

    // property access
    std::wstring get_name() const
        { return m_bag.read_prop<std::wstring>(0x3001); }
    size_t get_subfolder_count() const
        { return m_bag.read_prop<long>(0x3603); }
    size_t get_message_count() const
        { return m_bag.read_prop<long>(0x3602); }
    size_t get_associated_message_count() const
        { return m_bag.read_prop<long>(0x3617); }

    // lower layer access
    property_bag& get_property_bag()
        { return m_bag; }
    const property_bag& get_property_bag() const
        { return m_bag; }
    const table& get_hierarchy_table() const;
    const table& get_contents_table() const;
    const table& get_associated_contents_table() const;
    table& get_hierarchy_table();
    table& get_contents_table();
    table& get_associated_contents_table();

private:
    shared_db_ptr m_db;
    property_bag m_bag;
    mutable std::unique_ptr<table> m_contents_table;
    mutable std::unique_ptr<table> m_associated_contents_table;
    mutable std::unique_ptr<table> m_hierarchy_table;
};

class folder_transform_info : public std::unary_function<node_info, folder>
{
public:
    folder_transform_info(const shared_db_ptr& db) 
        : m_db(db) { }
    folder operator()(const node_info& info) const
        { return folder(m_db, node(m_db, info)); }

private:
    shared_db_ptr m_db;
};

}

inline fairport::search_folder::search_folder(const fairport::search_folder& other)
: m_db(other.m_db), m_bag(other.m_bag) 
{ 
    if(other.m_contents_table)
        m_contents_table.reset(new table(*other.m_contents_table));
}

inline fairport::folder::folder(const fairport::folder& other)
: m_db(other.m_db), m_bag(other.m_bag) 
{ 
    if(other.m_contents_table)
        m_contents_table.reset(new table(*other.m_contents_table));
    if(other.m_associated_contents_table)
        m_contents_table.reset(new table(*other.m_associated_contents_table));
    if(other.m_hierarchy_table)
        m_contents_table.reset(new table(*other.m_hierarchy_table));
}

inline fairport::folder fairport::folder_transform_row::operator()(fairport::const_table_row row) const
{ 
    return folder(m_db, m_db->lookup_node(row.row_id())); 
}

inline const fairport::table& fairport::search_folder::get_contents_table() const
{
    if(!m_contents_table)
        m_contents_table.reset(new table(m_db->lookup_node(make_nid(nid_type_search_contents_table, get_nid_index(m_bag.get_node().get_id())))));

    return *m_contents_table;
}

inline fairport::table& fairport::search_folder::get_contents_table()
{
    return const_cast<table&>(const_cast<const search_folder*>(this)->get_contents_table());
}

inline fairport::folder fairport::folder::open_sub_folder(const std::wstring& name)
{
    folder_iter iter = std::find_if(sub_folder_begin(), sub_folder_end(), [&name](folder f) {
        return f.get_name() == name;
    });
    
    if(iter != sub_folder_end())
        return *iter;

    throw key_not_found<std::wstring>(name);
}

inline const fairport::table& fairport::folder::get_contents_table() const
{
    if(!m_contents_table)
        m_contents_table.reset(new table(m_db->lookup_node(make_nid(nid_type_contents_table, get_nid_index(m_bag.get_node().get_id())))));

    return *m_contents_table;
}

inline fairport::table& fairport::folder::get_contents_table()
{
    return const_cast<table&>(const_cast<const folder*>(this)->get_contents_table());
}

inline const fairport::table& fairport::folder::get_hierarchy_table() const
{
    if(!m_hierarchy_table)
        m_hierarchy_table.reset(new table(m_db->lookup_node(make_nid(nid_type_hierarchy_table, get_nid_index(m_bag.get_node().get_id())))));

    return *m_hierarchy_table;
}

inline fairport::table& fairport::folder::get_hierarchy_table()
{
    return const_cast<table&>(const_cast<const folder*>(this)->get_hierarchy_table());
}

inline const fairport::table& fairport::folder::get_associated_contents_table() const
{
    if(!m_associated_contents_table)
        m_associated_contents_table.reset(new table(m_db->lookup_node(make_nid(nid_type_associated_contents_table, get_nid_index(m_bag.get_node().get_id())))));

    return *m_associated_contents_table;
}

inline fairport::table& fairport::folder::get_associated_contents_table()
{
    return const_cast<table&>(const_cast<const folder*>(this)->get_associated_contents_table());
}

#endif
