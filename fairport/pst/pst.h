#ifndef FAIRPORT_PST_PST_H
#define FAIRPORT_PST_PST_H

#include "fairport/ndb/database.h"
#include "fairport/ndb/database_iface.h"

#include "fairport/ltp/propbag.h"

#include "fairport/pst/folder.h"

namespace fairport
{

class pst
{
public:
    pst(const std::wstring& filename) m_db(open_database(filename)) { }
    pst(pst&& other) : m_db(other.m_db), m_bag(std::move(other.m_bag)) { }

    // subobject discovery/enumeration
    folder open_root_folder() const;
    folder open_folder(const std::wstring& name) const;

    // property access
    std::wstring get_display_name() const
        { return get_property_bag().read_prop<std::wstring>(0x3001); }

    // lower layer access
    const property_bag& get_property_bag() const
        { ensure_property_bag(); return *m_bag; }
    shared_db_ptr get_db() const 
        { return m_db; }

private:
    void ensure_property_bag() const;
    pst(const pst& other); // = delete
    pst& operator=(const pst& other); // = delete

    shared_db_ptr m_db;
    mutable std::unique_ptr<property_bag> m_bag;
};

} // end fairport namespace

inline void fairport::pst::ensure_property_bag() const
{
    if(!m_bag)
        m_bag.reset(new property_bag(m_db->lookup_node(nid_message_store)));
}

#endif
