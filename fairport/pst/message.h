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

class attachment
{
public:
    // property access
    std::wstring get_filename() const
		{ return m_bag.read_prop<std::wstring>(0x3707); }
	std::vector<byte> get_bytes() const
		{ return m_bag.read_prop<std::vector<byte>>(0x3701); }
	size_t size() const
		{ return m_bag.read_prop<uint>(0xe20); }

	// lower layer access
	const property_bag& get_property_bag() const
		{ return *m_bag; }
	property_bag& get_property_bag()
		{ return *m_bag; }

private:
	friend class message;
	attachment(const property_bag& attachment)
		: m_bag(attachment, alias_tag) { }

	property_bag m_bag;
};

}
#endif
