#ifndef FAIRPORT_PST_FOLDER_H
#define FAIRPORT_PST_FOLDER_H

#include "fairport/ndb/database_iface.h"

#include "fairport/ltp/propbag.h"
#include "fairport/ltp/table.h"

#include "fairport/pst/folder.h"

namespace fairport
{

class folder
{
public:
    folder(const shared_db_ptr& db, node_id nid);
    folder(const node& n);
    folder(const const_table_row& row);

    // subobject discovery/enumeration
    some_iter_type sub_folder_begin();
    some_iter_type sub_folder_end();
    some_const_iter_type sub_folder_begin() const;
    some_const_iter_type sub_folder_end() const;

    folder open_sub_folder(const std::wstring& name);

    some_iter_type message_begin();
    some_iter_type message_end();
    some_const_iter_type message_begin() const;
    some_const_iter_type message_end() const;

    some_iter_type associated_message_begin();
    some_iter_type associated_message_end();
    some_const_iter_type associated_message_begin() const;
    some_const_iter_type associated_message_end() const;

    // property access
    std::wstring get_display_name() const
        { return get_property_bag().read_prop<std::wstring>(0x3001); }
    uint get_subfolder_count() const;
    uint get_message_count() const;
    uint get_associated_message_count() const;

    // lower layer access
    const property_bag& get_property_bag() const
        { ensure_property_bag(); return *m_bag; }
    const table& get_hierarchy_table() const;
    const table& get_contents_table() const;
    const table& get_associated_contents_table() const;
    shared_db_ptr get_db() const
        { return m_db; }

private:
};

}
#endif
