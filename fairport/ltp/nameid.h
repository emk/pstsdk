#ifndef FAIRPORT_LTP_NAMEID_H
#define FAIRPORT_LTP_NAMEID_H

#include <string>

#include "fairport/util/primatives.h"

#include "fairport/ndb/database_iface.h"

#include "fairport/ltp/propbag.h"

namespace fairport
{

class name_id_map : private boost::noncopyable
{
public:
    name_id_map(const shared_db_ptr& db, const guid& namespace) : m_bag(db->lookup_node(nid_name_id_map)), m_guid(guid) { }
    
    guid get_namespace() const { return m_guid; }

    // query if a given name/id exists
    bool name_exists(const std::stringw& name) const;
    bool id_exists(long id) const;

    // the following two methods throw key_not_found if the requested name/id doesn't exist
    prop_id operator[](const std::stringw& name) const;
    prop_id operator[](long id) const;

private:
    guid m_guid;
    property_bag m_bag;
};

} // end namespace fairport

#endif
