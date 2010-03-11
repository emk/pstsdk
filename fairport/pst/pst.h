#ifndef FAIRPORT_PST_PST_H
#define FAIRPORT_PST_PST_H

#include <boost/noncopyable.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "fairport/ndb/database.h"
#include "fairport/ndb/database_iface.h"
#include "fairport/ndb/node.h"

#include "fairport/ltp/propbag.h"

#include "fairport/pst/folder.h"
#include "fairport/pst/message.h"

namespace fairport
{

class pst : boost::noncopyable
{
    typedef boost::filter_iterator<is_nid_type<nid_type_folder>, const_nodeinfo_iterator> folder_filter_iterator;
    typedef boost::filter_iterator<is_nid_type<nid_type_message>, const_nodeinfo_iterator> message_filter_iterator;

public:
    typedef boost::transform_iterator<message_transform_info, message_filter_iterator> message_iterator;
    typedef boost::transform_iterator<folder_transform_info, folder_filter_iterator> folder_iterator;

    pst(const std::wstring& filename) 
        : m_db(open_database(filename)) { }
    pst(pst&& other) 
        : m_db(std::move(other.m_db)), m_bag(std::move(other.m_bag)) { }

    // subobject discovery/enumeration
    folder_iterator folder_begin() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_folder>>(m_db->read_nbt_root()->begin(), m_db->read_nbt_root()->end()), folder_transform_info(m_db) ); }
    folder_iterator folder_end() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_folder>>(m_db->read_nbt_root()->end(), m_db->read_nbt_root()->end()), folder_transform_info(m_db) ); }

    message_iterator message_begin() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_message>>(m_db->read_nbt_root()->begin(), m_db->read_nbt_root()->end()), message_transform_info(m_db) ); }
    message_iterator message_end() const
        { return boost::make_transform_iterator(boost::make_filter_iterator<is_nid_type<nid_type_message>>(m_db->read_nbt_root()->end(), m_db->read_nbt_root()->end()), message_transform_info(m_db) ); }

    folder open_root_folder() const
        { return folder(m_db, m_db->lookup_node(nid_root_folder)); }
    folder open_folder(const std::wstring& name) const;

    // property access
    std::wstring get_name() const
        { return get_property_bag().read_prop<std::wstring>(0x3001); }

    // lower layer access
    property_bag& get_property_bag();
    const property_bag& get_property_bag() const;
    shared_db_ptr get_db() const 
        { return m_db; }

private:
    shared_db_ptr m_db;
    mutable std::unique_ptr<property_bag> m_bag;
};

} // end fairport namespace

inline const fairport::property_bag& fairport::pst::get_property_bag() const
{
    if(!m_bag)
        m_bag.reset(new property_bag(m_db->lookup_node(nid_message_store)));

    return *m_bag;
}

inline fairport::property_bag& fairport::pst::get_property_bag()
{
    return const_cast<property_bag&>(const_cast<const pst*>(this)->get_property_bag());
}

inline fairport::folder fairport::pst::open_folder(const std::wstring& name) const
{
    folder_iterator iter = std::find_if(folder_begin(), folder_end(), [&name](const folder& f) {
        return f.get_name() == name;
    });
    
    if(iter != folder_end())
        return *iter;

    throw key_not_found<std::wstring>(name);
}

#endif
