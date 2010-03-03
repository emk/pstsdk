#ifndef FAIRPORT_PST_MESSAGE_H
#define FAIRPORT_PST_MESSAGE_H

#include "fairport/ndb/database_iface.h"

#include "fairport/ltp/propbag.h"
#include "fairport/ltp/table.h"

namespace fairport
{

class message
{
public:
	message();
	message(const node& n);
	message(const attachment& attach);
	 
	// subobject discovery/enumeration
    some_iter_type attachment_begin();
	some_iter_type attachment_end();
    some_const_iter_type attachment_begin() const;
	some_const_iter_type attachment_end() const;

    folder open_sub_folder(const std::wstring& name);

	some_iter_type recipient_begin();
	some_iter_type recipient_end();
	some_const_iter_type recipient_begin() const;
	some_const_iter_type recipient_end() const;


    // property access
    std::wstring get_subject() const;
	uint size() const;
    uint get_attachment_count() const;
	uint get_recipient_count() const;
	uint get_associated_message_count() const;

    // lower layer access
    const property_bag& get_property_bag() const
        { ensure_property_bag(); return *m_bag; }
	const table& get_attachment_table() const;
	const table& get_recpient_table() const;
	const table& get_associated_contents_table() const;
    shared_db_ptr get_db() const 
        { return m_db; }

private:
};

}
#endif
