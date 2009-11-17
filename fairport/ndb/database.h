#ifndef FAIRPORT_NDB_DATABASE_H
#define FAIRPORT_NDB_DATABASE_H

#include <fstream>
#include <memory>

#include "fairport/util/btree.h"
#include "fairport/util/errors.h"
#include "fairport/util/primatives.h"
#include "fairport/util/util.h"

#include "fairport/disk/disk.h"

#include "fairport/ndb/node.h"
#include "fairport/ndb/page.h"
#include "fairport/ndb/database_iface.h"

namespace fairport 
{ 

class node;

template<typename T> 
class database_impl;
typedef database_impl<ulonglong> large_pst;
typedef database_impl<ulong> small_pst;

shared_db_ptr open_database(const std::wstring& filename);
std::shared_ptr<small_pst> open_small_pst(const std::wstring& filename);
std::shared_ptr<large_pst> open_large_pst(const std::wstring& filename);

template<typename T>
class database_impl :
    public std::enable_shared_from_this<database_impl<T>>,
    public database
{
public:

    node lookup_node(node_id nid)
        { return node(this->shared_from_this(), lookup_node_info(nid)); }
    node_info lookup_node_info(node_id nid);
    block_info lookup_block_info(block_id bid); 
    
    std::shared_ptr<bbt_page> read_bbt_root();
    std::shared_ptr<nbt_page> read_nbt_root();
    std::shared_ptr<bbt_page> read_bbt_page(ulonglong location);
    std::shared_ptr<nbt_page> read_nbt_page(ulonglong location);
    std::shared_ptr<nbt_leaf_page> read_nbt_leaf_page(ulonglong location);
    std::shared_ptr<bbt_leaf_page> read_bbt_leaf_page(ulonglong location);
    std::shared_ptr<nbt_nonleaf_page> read_nbt_nonleaf_page(ulonglong location);
    std::shared_ptr<bbt_nonleaf_page> read_bbt_nonleaf_page(ulonglong location);

    std::shared_ptr<block> read_block(block_id bid)
        { return read_block(lookup_block_info(bid)); }
    std::shared_ptr<data_block> read_data_block(block_id bid)
        { return read_data_block(lookup_block_info(bid)); }
    std::shared_ptr<extended_block> read_extended_block(block_id bid)
        { return read_extended_block(lookup_block_info(bid)); }
    std::shared_ptr<external_block> read_external_block(block_id bid)
        { return read_external_block(lookup_block_info(bid)); }
    std::shared_ptr<subnode_block> read_subnode_block(block_id bid)
        { return read_subnode_block(lookup_block_info(bid)); }
    std::shared_ptr<subnode_leaf_block> read_subnode_leaf_block(block_id bid)
        { return read_subnode_leaf_block(lookup_block_info(bid)); }
    std::shared_ptr<subnode_nonleaf_block> read_subnode_nonleaf_block(block_id bid)
        { return read_subnode_nonleaf_block(lookup_block_info(bid)); }

    std::shared_ptr<block> read_block(const block_info& bi);
    std::shared_ptr<data_block> read_data_block(const block_info& bi);
    std::shared_ptr<extended_block> read_extended_block(const block_info& bi);
    std::shared_ptr<external_block> read_external_block(const block_info& bi);
    std::shared_ptr<subnode_block> read_subnode_block(const block_info& bi);
    std::shared_ptr<subnode_leaf_block> read_subnode_leaf_block(const block_info& bi);
    std::shared_ptr<subnode_nonleaf_block> read_subnode_nonleaf_block(const block_info& bi);
   
protected:
    database_impl();
    database_impl(const std::wstring& filename);
    void validate_header();
    std::shared_ptr<nbt_leaf_page> read_nbt_leaf_page(ulonglong location, disk::nbt_leaf_page<T>& the_page);
    std::shared_ptr<bbt_leaf_page> read_bbt_leaf_page(ulonglong location, disk::bbt_leaf_page<T>& the_page);
    std::shared_ptr<nbt_nonleaf_page> read_nbt_nonleaf_page(ulonglong location, disk::nbt_nonleaf_page<T>& the_page);
    std::shared_ptr<bbt_nonleaf_page> read_bbt_nonleaf_page(ulonglong location, disk::bbt_nonleaf_page<T>& the_page);
    std::shared_ptr<subnode_leaf_block> read_subnode_leaf_block(const block_info& bi, disk::sub_leaf_block<T>& sub_block);
    std::shared_ptr<subnode_nonleaf_block> read_subnode_nonleaf_block(const block_info& bi, disk::sub_nonleaf_block<T>& sub_block);

    friend shared_db_ptr open_database(const std::wstring& filename);
    friend std::shared_ptr<small_pst> open_small_pst(const std::wstring& filename);
    friend std::shared_ptr<large_pst> open_large_pst(const std::wstring& filename);

    file m_file;
    disk::header<T> m_header;
};

template<>
inline void database_impl<ulong>::validate_header()
{
    if(m_header.wVer >= disk::database_format_unicode)
    {
        throw invalid_format();
    }

    ulong crc = disk::compute_crc(((byte*)&m_header) + disk::header_crc_locations<ulong>::start, disk::header_crc_locations<ulong>::length);

    if(crc != m_header.dwCRCPartial)
        throw crc_fail("header dwCRCPartial");
}

template<>
inline void database_impl<ulonglong>::validate_header()
{
    if(m_header.wVer < disk::database_format_unicode)
    {
        throw invalid_format();
    }

    ulong crc_partial = disk::compute_crc(((byte*)&m_header) + disk::header_crc_locations<ulonglong>::partial_start, disk::header_crc_locations<ulonglong>::partial_length);
    ulong crc_full = disk::compute_crc(((byte*)&m_header) + disk::header_crc_locations<ulonglong>::full_start, disk::header_crc_locations<ulonglong>::full_length);

    if(crc_partial != m_header.dwCRCPartial)
        throw crc_fail("header dwCRCPartial");

    if(crc_full != m_header.dwCRCFull)
        throw crc_fail("header dwCRCFull");
}

} // end namespace

inline fairport::shared_db_ptr fairport::open_database(const std::wstring& filename)
{
    try 
    {
        shared_db_ptr db = open_small_pst(filename);
        return db;
    }
    catch(invalid_format&)
    {
        // well, that didn't work
    }

    shared_db_ptr db = open_large_pst(filename);
    return db;
}

inline std::shared_ptr<fairport::small_pst> fairport::open_small_pst(const std::wstring& filename)
{
    std::shared_ptr<small_pst> db(new small_pst(filename));
    return db;
}

inline std::shared_ptr<fairport::large_pst> fairport::open_large_pst(const std::wstring& filename)
{
    std::shared_ptr<large_pst> db(new large_pst(filename));
    return db;
}

template<typename T>
inline std::shared_ptr<fairport::bbt_page> fairport::database_impl<T>::read_bbt_root()
{ 
    return read_bbt_page(m_header.root.brefBBT.ib); 
}

template<typename T>
inline std::shared_ptr<fairport::nbt_page> fairport::database_impl<T>::read_nbt_root()
{ 
    return read_nbt_page(m_header.root.brefNBT.ib);
}

template<typename T>
inline fairport::database_impl<T>::database_impl(const std::wstring& filename)
: m_file(filename)
{
    std::vector<byte> buffer(sizeof(m_header));
    m_file.read(buffer, 0);
    memcpy(&m_header, &buffer[0], sizeof(m_header));

    validate_header();
}

template<typename T>
inline std::shared_ptr<fairport::nbt_leaf_page> fairport::database_impl<T>::read_nbt_leaf_page(ulonglong location)
{
    std::vector<byte> buffer(disk::page_size);
    disk::page<T>* ppage = (disk::page<T>*)&buffer[0];

    m_file.read(buffer, location);
    
    if(ppage->trailer.page_type == disk::page_type_nbt)
    {
        disk::nbt_leaf_page<T>* leaf_page = (disk::nbt_leaf_page<T>*)ppage;

        if(leaf_page->level == 0)
            return read_nbt_leaf_page(location, *leaf_page);
    }

    throw unexpected_page("page_type != page_type_nbt");
}

template<typename T>
inline std::shared_ptr<fairport::nbt_leaf_page> fairport::database_impl<T>::read_nbt_leaf_page(ulonglong location, disk::nbt_leaf_page<T>& the_page)
{
    node_info ni;
    std::vector<std::pair<node_id, node_info>> nodes;

    for(int i = 0; i < the_page.num_entries; ++i)
    {
        ni.id = static_cast<node_id>(the_page.entries[i].nid);
        ni.data_bid = the_page.entries[i].data;
        ni.sub_bid = the_page.entries[i].sub;
        ni.parent_id = the_page.entries[i].parent_nid;

        nodes.push_back(std::make_pair(ni.id, ni));
    }

    return std::shared_ptr<nbt_leaf_page>(new nbt_leaf_page(this->shared_from_this(), the_page.trailer.bid, location, std::move(nodes)));
}

template<typename T>
inline std::shared_ptr<fairport::bbt_leaf_page> fairport::database_impl<T>::read_bbt_leaf_page(ulonglong location)
{
    std::vector<byte> buffer(disk::page_size);
    disk::page<T>* ppage = (disk::page<T>*)&buffer[0];

    m_file.read(buffer, location);
    
    if(ppage->trailer.page_type == disk::page_type_bbt)
    {
        disk::bbt_leaf_page<T>* leaf_page = (disk::bbt_leaf_page<T>*)ppage;

        if(leaf_page->level == 0)
            return read_bbt_leaf_page(location, *leaf_page);
    }

    throw unexpected_page("page_type != page_type_bbt");
}

template<typename T>
inline std::shared_ptr<fairport::bbt_leaf_page> fairport::database_impl<T>::read_bbt_leaf_page(ulonglong location, disk::bbt_leaf_page<T>& the_page)
{
    block_info bi;
    std::vector<std::pair<block_id, block_info>> blocks;
    
    for(int i = 0; i < the_page.num_entries; ++i)
    {
        bi.id = the_page.entries[i].ref.bid;
        bi.address = the_page.entries[i].ref.ib;
        bi.size = the_page.entries[i].size;
        bi.ref_count = the_page.entries[i].ref_count;

        blocks.push_back(std::make_pair(bi.id, bi));
    }

    return std::shared_ptr<bbt_leaf_page>(new bbt_leaf_page(this->shared_from_this(), the_page.trailer.bid, location, std::move(blocks)));
}

template<typename T>
inline std::shared_ptr<fairport::nbt_nonleaf_page> fairport::database_impl<T>::read_nbt_nonleaf_page(ulonglong location)
{
    std::vector<byte> buffer(disk::page_size);
    disk::page<T>* ppage = (disk::page<T>*)&buffer[0];

    m_file.read(buffer, location);
    
    if(ppage->trailer.page_type == disk::page_type_nbt)
    {
        disk::nbt_nonleaf_page<T>* nonleaf_page = (disk::nbt_nonleaf_page<T>*)ppage;

        if(nonleaf_page->level > 0)
            return read_nbt_nonleaf_page(location, *nonleaf_page);
    }

    throw unexpected_page("page_type != page_type_nbt");
}

template<typename T>
inline std::shared_ptr<fairport::nbt_nonleaf_page> fairport::database_impl<T>::read_nbt_nonleaf_page(ulonglong location, fairport::disk::nbt_nonleaf_page<T>& the_page)
{
    std::vector<std::pair<node_id, ulonglong>> nodes;
    
    for(int i = 0; i < the_page.num_entries; ++i)
    {
        nodes.push_back(std::make_pair((node_id)the_page.entries[i].key, the_page.entries[i].ref.ib));
    }

    return std::shared_ptr<nbt_nonleaf_page>(new nbt_nonleaf_page(this->shared_from_this(), the_page.trailer.bid, location, the_page.level, std::move(nodes)));
}

template<typename T>
inline std::shared_ptr<fairport::bbt_nonleaf_page> fairport::database_impl<T>::read_bbt_nonleaf_page(ulonglong location)
{
    std::vector<byte> buffer(disk::page_size);
    disk::page<T>* ppage = (disk::page<T>*)&buffer[0];

    m_file.read(buffer, location);
    
    if(ppage->trailer.page_type == disk::page_type_bbt)
    {
        disk::bbt_nonleaf_page<T>* nonleaf_page = (disk::bbt_nonleaf_page<T>*)ppage;

        if(nonleaf_page->level > 0)
            return read_bbt_nonleaf_page(location, *nonleaf_page);
    }

    throw unexpected_page("page_type != page_type_bbt");
}


template<typename T>
inline std::shared_ptr<fairport::bbt_nonleaf_page> fairport::database_impl<T>::read_bbt_nonleaf_page(ulonglong location, disk::bbt_nonleaf_page<T>& the_page)
{
    std::vector<std::pair<block_id, ulonglong>> blocks;
    
    for(int i = 0; i < the_page.num_entries; ++i)
    {
        blocks.push_back(std::make_pair(the_page.entries[i].key, the_page.entries[i].ref.ib));
    }

    return std::shared_ptr<bbt_nonleaf_page>(new bbt_nonleaf_page(this->shared_from_this(), the_page.trailer.bid, location, the_page.level, std::move(blocks)));

}

template<typename T>
inline std::shared_ptr<fairport::bbt_page> fairport::database_impl<T>::read_bbt_page(ulonglong location)
{
    std::vector<byte> buffer(disk::page_size);
    disk::page<T>* ppage = (disk::page<T>*)&buffer[0];

    m_file.read(buffer, location);

    if(ppage->trailer.page_type == disk::page_type_bbt)
    {
        disk::bbt_leaf_page<T>* leaf = (disk::bbt_leaf_page<T>*)ppage;
        if(leaf->level == 0)
        {
            // it really is a leaf!
            return read_bbt_leaf_page(location, *leaf);
        }
        else
        {
            disk::bbt_nonleaf_page<T>* nonleaf = (disk::bbt_nonleaf_page<T>*)ppage;
            return read_bbt_nonleaf_page(location, *nonleaf);
        }
    }
    else
    {
        throw unexpected_page("page_type != page_type_bbt");
    }  
}
        
template<typename T>
inline std::shared_ptr<fairport::nbt_page> fairport::database_impl<T>::read_nbt_page(ulonglong location)
{
    std::vector<byte> buffer(disk::page_size);
    disk::page<T>* ppage = (disk::page<T>*)&buffer[0];

    m_file.read(buffer, location);

    if(ppage->trailer.page_type == disk::page_type_nbt)
    {
        disk::nbt_leaf_page<T>* leaf = (disk::nbt_leaf_page<T>*)ppage;
        if(leaf->level == 0)
        {
            // it really is a leaf!
            return read_nbt_leaf_page(location, *leaf);
        }
        else
        {
            disk::nbt_nonleaf_page<T>* nonleaf = (disk::nbt_nonleaf_page<T>*)ppage;
            return read_nbt_nonleaf_page(location, *nonleaf);
        }
    }
    else
    {
        throw unexpected_page("page_type != page_type_nbt");
    }  
}

template<typename T>
inline fairport::node_info fairport::database_impl<T>::lookup_node_info(node_id nid)
{
    return read_nbt_root()->lookup(nid); 
}

template<typename T>
inline fairport::block_info fairport::database_impl<T>::lookup_block_info(block_id bid)
{
    if(bid == 0)
    {
        block_info bi;
        bi.id = bi.address = bi.size = bi.ref_count = 0;
        return bi;
    }
    else
    {
        return read_bbt_root()->lookup(bid);
    }
}

template<typename T>
inline std::shared_ptr<fairport::block> fairport::database_impl<T>::read_block(const block_info& bi)
{
    std::shared_ptr<block> pblock;

    try
    {
        pblock = read_data_block(bi);
    }
    catch(unexpected_block&)
    {
        pblock = read_subnode_block(bi);
    }

    return pblock;
}

template<typename T>
inline std::shared_ptr<fairport::data_block> fairport::database_impl<T>::read_data_block(const block_info& bi)
{
    if(disk::bid_is_external(bi.id))
        return read_external_block(bi);

    std::vector<byte> buffer(sizeof(disk::extended_block<T>));
    disk::extended_block<T>* peblock = (disk::extended_block<T>*)&buffer[0];
    m_file.read(buffer, bi.address);

    if(peblock->block_type != disk::block_type_extended)
        throw unexpected_block("Extended block expected");

    return read_extended_block(bi);
}

template<typename T>
inline std::shared_ptr<fairport::extended_block> fairport::database_impl<T>::read_extended_block(const block_info& bi)
{
    if(!disk::bid_is_internal(bi.id))
        throw unexpected_block("Internal BID expected");

    std::vector<byte> buffer(disk::align_disk<T>(bi.size));
    disk::extended_block<T>* peblock = (disk::extended_block<T>*)&buffer[0];
    std::vector<block_id> child_blocks;

    m_file.read(buffer, bi.address);

    for(int i = 0; i < peblock->count; ++i)
        child_blocks.push_back(peblock->bid[i]);
    
#ifdef __GNUC__
    // GCC gave link errors on extended_block<T> and external_block<T> max_size
    // with the below alernative
    uint sub_size = 0;
    if(peblock->level == 1)
        sub_size = disk::external_block<T>::max_size;
    else
        sub_size = disk::extended_block<T>::max_size;
#else
    uint sub_size = (peblock->level == 1 ? disk::external_block<T>::max_size : disk::extended_block<T>::max_size);
#endif
    uint sub_page_count = peblock->level == 1 ? 1 : disk::extended_block<T>::max_count;

    return std::shared_ptr<extended_block>(new extended_block(this->shared_from_this(), bi.size, sub_size, sub_page_count, peblock->total_size, peblock->level, bi.id, bi.address, std::move(child_blocks)));
}

template<typename T>
inline std::shared_ptr<fairport::external_block> fairport::database_impl<T>::read_external_block(const block_info& bi)
{
    if(!disk::bid_is_external(bi.id))
        throw unexpected_block("External BID expected");

    std::vector<byte> buffer(disk::align_disk<T>(bi.size));
    m_file.read(buffer, bi.address);

    if(m_header.bCryptMethod == disk::crypt_method_permute)
    {
        disk::permute(&buffer[0], bi.size, false);
    }
    else if(m_header.bCryptMethod == disk::crypt_method_cyclic)
    {
        disk::cyclic(&buffer[0], bi.size, (ulong)bi.id);
    }

    return std::shared_ptr<external_block>(new external_block(this->shared_from_this(), bi.size, disk::external_block<T>::max_size, bi.id, bi.address, std::move(buffer)));
}

template<typename T>
inline std::shared_ptr<fairport::subnode_block> fairport::database_impl<T>::read_subnode_block(const block_info& bi)
{
    if(bi.id == 0)
    {
        std::vector<std::pair<node_id, subnode_info>> empty;
        return std::shared_ptr<subnode_block>(new subnode_leaf_block(this->shared_from_this(), 0, 0, 0, std::move(empty)));
    }
    
    std::vector<byte> buffer(disk::align_disk<T>(bi.size));
    disk::sub_leaf_block<T>* psub = (disk::sub_leaf_block<T>*)&buffer[0];
    std::shared_ptr<subnode_block> sub_block;

    m_file.read(buffer, bi.address);

    if(psub->level == 0)
    {
        sub_block = read_subnode_leaf_block(bi, *psub);
    }
    else
    {
        sub_block = read_subnode_nonleaf_block(bi, *(disk::sub_nonleaf_block<T>*)&buffer[0]);
    }

    return sub_block;
}

template<typename T>
inline std::shared_ptr<fairport::subnode_leaf_block> fairport::database_impl<T>::read_subnode_leaf_block(const block_info& bi)
{
    std::vector<byte> buffer(disk::align_disk<T>(bi.size));
    disk::sub_leaf_block<T>* psub = (disk::sub_leaf_block<T>*)&buffer[0];
    std::shared_ptr<subnode_leaf_block> sub_block;

    m_file.read(buffer, bi.address);

    if(psub->level == 0)
    {
        sub_block = read_subnode_leaf_block(bi, *psub);
    }
    else
    {
        throw unexpected_block("psub->level != 0");
    }

    return sub_block; 
}

template<typename T>
inline std::shared_ptr<fairport::subnode_nonleaf_block> fairport::database_impl<T>::read_subnode_nonleaf_block(const block_info& bi)
{
    std::vector<byte> buffer(disk::align_disk<T>(bi.size));
    disk::sub_nonleaf_block<T>* psub = (disk::sub_nonleaf_block<T>*)&buffer[0];
    std::shared_ptr<subnode_nonleaf_block> sub_block;

    m_file.read(buffer, bi.address);

    if(psub->level != 0)
    {
        sub_block = read_subnode_nonleaf_block(bi, *psub);
    }
    else
    {
        throw unexpected_block("psub->level == 1");
    }

    return sub_block;
}

template<typename T>
inline std::shared_ptr<fairport::subnode_leaf_block> fairport::database_impl<T>::read_subnode_leaf_block(const block_info& bi, disk::sub_leaf_block<T>& sub_block)
{
    subnode_info ni;
    std::vector<std::pair<node_id, subnode_info>> subnodes;

    for(int i = 0; i < sub_block.count; ++i)
    {
        ni.id = sub_block.entry[i].nid;
        ni.data_bid = sub_block.entry[i].data;
        ni.sub_bid = sub_block.entry[i].sub;

        subnodes.push_back(std::make_pair(sub_block.entry[i].nid, ni));
    }

    return std::shared_ptr<subnode_leaf_block>(new subnode_leaf_block(this->shared_from_this(), bi.size, bi.id, bi.address, std::move(subnodes)));
}

template<typename T>
inline std::shared_ptr<fairport::subnode_nonleaf_block> fairport::database_impl<T>::read_subnode_nonleaf_block(const block_info& bi, disk::sub_nonleaf_block<T>& sub_block)
{
    std::vector<std::pair<node_id, block_id>> subnodes;

    for(int i = 0; i < sub_block.count; ++i)
    {
        subnodes.push_back(std::make_pair(sub_block.entry[i].nid_key, sub_block.entry[i].sub_block_bid));
    }

    return std::shared_ptr<subnode_nonleaf_block>(new subnode_nonleaf_block(this->shared_from_this(), bi.size, bi.id, bi.address, std::move(subnodes)));
}
#endif
