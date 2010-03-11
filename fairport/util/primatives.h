#ifndef FAIRPORT_UTIL_PRIMATIVES_H
#define FAIRPORT_UTIL_PRIMATIVES_H

// Global Validation Settings
//
// #define FAIRPORT_VALIDATION_LEVEL_NONE before including any fairport headers for no validation
//      well, slightly more than no validation - type checks are still performed
// #define FAIRPORT_VALIDATION_LEVEL_WEAK before including any fairport headers for weak validation
//      weak validation generally involves fast checks, such as signature matching, param validation, etc
// #define FAIRPORT_VALIDATION_LEVEL_FULL before including any fairport headers for full validation
//      full validation includes all weak checks plus crc validation and any other "expensive" check
//
// Weak validation is the default.
//
#ifndef FAIRPORT_VALIDATION_LEVEL_NONE
#define FAIRPORT_VALIDATION_LEVEL_WEAK
#endif

#ifdef FAIRPORT_VALIDATION_LEVEL_FULL
// full validation also implies weak validation
#define FAIRPORT_VALIDATION_LEVEL_WEAK
#endif

namespace fairport
{

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;
typedef long long longlong;
typedef unsigned char byte;
typedef unsigned short ushort;

static_assert(sizeof(byte) == 1, "fairport::byte unexpected size");
static_assert(sizeof(ushort) == 2, "fairport::ushort unexpected size");
static_assert(sizeof(uint) == 4, "fairport::uint unexpected size");
static_assert(sizeof(ulonglong) == 8, "fairport::ulonglong unexpected size");

typedef ulong node_id;
typedef ulonglong block_id;
typedef block_id page_id;

typedef ulong heap_id;
typedef ulong heapnode_id;

typedef ushort prop_id;

typedef ulong row_id;

struct alias_tag { };

//
// node id
//
enum nid_type
{
    nid_type_none = 0x00,
    nid_type_internal = 0x01,
    nid_type_folder = 0x02,
    nid_type_search_folder = 0x03,
    nid_type_message = 0x04,
    nid_type_attachment = 0x05,
    nid_type_search_update_queue = 0x06,
    nid_type_search_criteria_object = 0x07,
    nid_type_associated_message = 0x08,
    nid_type_storage = 0x09,
    nid_type_contents_table_index = 0x0A,
    nid_type_receive_folder_table = 0x0B,
    nid_type_outgoing_queue_table = 0x0C,
    nid_type_hierarchy_table = 0x0D,
    nid_type_contents_table = 0x0E,
    nid_type_associated_contents_table = 0x0F,
    nid_type_search_contents_table = 0x10,
    nid_type_attachment_table = 0x11,
    nid_type_recipient_table = 0x12,
    nid_type_search_table_index = 0x13,
    nid_type_contents_smp = 0x14,
    nid_type_associated_contents_smp = 0x15,
    nid_type_change_history_table = 0x16,
    nid_type_tombstone_table = 0x17,
    nid_type_tombstone_date_table = 0x18,
    nid_type_lrep_dups_table = 0x19,
    nid_type_folder_path_tombstone_table = 0x1A,
    nid_type_ltp = 0x1F,
    nid_type_max = 0x20
};

const ulong nid_type_mask = 0x1FL;

#define make_nid(nid_type,nid_index) (((nid_type)&nid_type_mask)|((nid_index) << 5))
#define make_prv_pub_nid(nid_index) (make_nid(nid_type_folder, nid_index_prv_pub_base + (nid_index)))

enum predefined_nid
{
    nid_message_store = make_nid(nid_type_internal, 0x1),
    nid_name_id_map = make_nid(nid_type_internal, 0x3),
    nid_normal_folder_template = make_nid(nid_type_folder, 0x6),
    nid_search_folder_template = make_nid(nid_type_search_folder, 0x7),
    nid_root_folder = make_nid(nid_type_folder, 0x9),
    nid_search_management_queue = make_nid(nid_type_internal, 0xF),
    nid_search_activity_list = make_nid(nid_type_internal, 0x10),
    nid_search_domain_alternative = make_nid(nid_type_internal, 0x12),
    nid_search_domain_object = make_nid(nid_type_internal, 0x13),
    nid_search_gatherer_queue = make_nid(nid_type_internal, 0x14),
    nid_search_gatherer_descriptor = make_nid(nid_type_internal, 0x15),
    nid_table_rebuild_queue = make_nid(nid_type_internal, 0x17),
    nid_junk_mail_pihsl = make_nid(nid_type_internal, 0x18),
    nid_search_gatherer_folder_queue = make_nid(nid_type_internal, 0x19),
    nid_tc_sub_props = make_nid(nid_type_internal, 0x27),
    nid_index_template = 0x30,
    nid_hierarchy_table_template = make_nid(nid_type_hierarchy_table, nid_index_template),
    nid_contents_table_template = make_nid(nid_type_contents_table, nid_index_template),
    nid_associated_contents_table_template = make_nid(nid_type_associated_contents_table, nid_index_template),
    nid_search_contents_table_template = make_nid(nid_type_search_contents_table, nid_index_template),
    nid_smp_template = make_nid(nid_type_contents_smp, nid_index_template),
    nid_tombstone_table_template = make_nid(nid_type_tombstone_table, nid_index_template),
    nid_lrep_dups_table_template = make_nid(nid_type_lrep_dups_table, nid_index_template),
    nid_receive_folders = make_nid(nid_type_receive_folder_table, 0x31),
    nid_outgoing_queue = make_nid(nid_type_outgoing_queue_table, 0x32),
    nid_attachment_table = make_nid(nid_type_attachment_table, 0x33),
    nid_recipient_table = make_nid(nid_type_recipient_table, 0x34),
    nid_change_history_table = make_nid(nid_type_change_history_table, 0x35),
    nid_tombstone_table = make_nid(nid_type_tombstone_table, 0x36),
    nid_tombstone_date_table = make_nid(nid_type_tombstone_date_table, 0x37),
    nid_all_message_search_folder = make_nid(nid_type_search_folder, 0x39),
    nid_all_message_search_contents = make_nid(nid_type_search_contents_table, 0x39),
    nid_lrep_gmp = make_nid(nid_type_internal, 0x40),
    nid_lrep_folders_smp = make_nid(nid_type_internal, 0x41),
    nid_lrep_folders_table = make_nid(nid_type_internal, 0x42),
    nid_folder_path_tombstone_table = make_nid(nid_type_internal, 0x43),
    nid_hst_hmp = make_nid(nid_type_internal, 0x60),
    nid_index_prv_pub_base = 0x100,
    nid_pub_root_folder = make_prv_pub_nid(0),
    nid_prv_root_folder = make_prv_pub_nid(5),
    nid_criterr_notification = make_nid(nid_type_internal, 0x3FD),
    nid_object_notification = make_nid(nid_type_internal, 0x3FE),
    nid_newemail_notification = make_nid(nid_type_internal, 0x3FF),
    nid_extended_notification = make_nid(nid_type_internal, 0x400),
    nid_indexing_notification = make_nid(nid_type_internal, 0x401)
};

inline nid_type get_nid_type(node_id id)
    { return (nid_type)(id & nid_type_mask); }

inline ulong get_nid_index(node_id id)
    { return id >> 5; }

//
// heap id
//
enum heap_page_type
{
    heap_page_type_first = 0,
    heap_page_type_normal = 1,
    heap_page_type_fill_bitmap = 2
};

inline ulong get_heap_page(heap_id id)
    { return (id >> 16); } 

inline ulong get_heap_index(heap_id id)
    { return (((id >> 5) - 1) & 0x7FF); }


//
// heapnode id
//

inline bool is_heap_id(heapnode_id id)
    { return (get_nid_type(id) == nid_type_none); }

inline bool is_subnode_id(heapnode_id id)
    { return (get_nid_type(id) != nid_type_none); }

//
// properties
//

enum prop_type
{
    prop_type_unspecified = 0,
    prop_type_null = 1,
    prop_type_short = 2,
    prop_type_mv_short = 4098,
    prop_type_long = 3,
    prop_type_mv_long = 4099,
    prop_type_float = 4,
    prop_type_mv_float = 4100,
    prop_type_double = 5,
    prop_type_mv_double = 4101,
    prop_type_currency = 6,
    prop_type_mv_currency = 4102,
    prop_type_apptime = 7, // VT_DATE
    prop_type_mv_apptime = 4103,
    prop_type_error = 10,
    prop_type_boolean = 11,
    prop_type_object = 13,
    prop_type_longlong = 20,
    prop_type_mv_longlong = 4116,
    prop_type_string = 30,
    prop_type_mv_string = 4126,
    prop_type_wstring = 31,
    prop_type_mv_wstring = 4127,
    prop_type_systime = 64, // Win32 FILETIME
    prop_type_mv_systime = 4160,
    prop_type_guid = 72,
    prop_type_mv_guid = 4168,
    prop_type_binary = 258,
    prop_type_mv_binary = 4354,
};

//
// mapi recipient type
//

enum recipient_type
{
    mapi_to = 1,
    mapi_cc = 2,
    mapi_bcc = 3
};

// 
// message specific values
//

const byte message_subject_prefix_lead_byte = 0x01;

//
// Win32 GUID
//
struct win_guid
{
    ulong data1;
    short data2;
    short data3;
    byte data4[8];
};

} // end fairport namespace
#endif
