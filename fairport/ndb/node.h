#ifndef FAIRPORT_NDB_NODE_H
#define FAIRPORT_NDB_NODE_H

#include <vector>
#include <algorithm>
#include <cassert>

#include "fairport/util/util.h"
#include "fairport/util/btree.h"

#include "fairport/ndb/database_iface.h"

namespace fairport
{

struct block_info
{
    block_id id;
    ulonglong address;
    ushort size;
    ushort ref_count;
};

class node
{
public:
    node(const shared_db_ptr& db, node_id id, node_id parent_id, node_id root_node_id, block_id data, block_id sub)
        : m_db(db), m_data(data), m_sub(sub), m_id(id), m_parent_id(parent_id), m_root_node_id(root_node_id) { } 
    node(const node& other);
    node(node&& other);

    node& operator=(const node& other);
    node& operator=(node&& other);

    node_id get_id() const { return m_id; }
    block_id get_data_id() const { return m_data; }
    block_id get_sub_id() const { return m_sub; }
    node_id get_parent_id() const { return m_parent_id; }
    node_id get_root_node_id() const { return m_root_node_id; }
    std::shared_ptr<data_block> get_data_block()
        { ensure_data_block(); return m_pdata; }
    std::shared_ptr<const data_block> get_data_block() const
        { ensure_data_block(); return m_pdata; }
    std::shared_ptr<subnode_block> get_subnode_block()
        { ensure_sub_block(); return m_psub; }
    std::shared_ptr<const subnode_block> get_subnode_block() const 
        { ensure_sub_block(); return m_psub; }
   
    size_t read(std::vector<byte>& buffer, ulong offset) const;
    size_t read(std::vector<byte>& buffer, uint page_num, ulong offset) const;

    size_t size() const;
    size_t get_page_size(uint page_num) const;
    uint get_page_count() const;

    // iterate over subnodes
    node_iterator subnode_begin();
    const_node_iterator subnode_begin() const;
    node_iterator subnode_end();
    const_node_iterator subnode_end() const;
    node& lookup(node_id id);
    const node& lookup(node_id id) const;

    const byte * get_ptr(uint page_num, ulong offset) const;

private:
    node();

    shared_db_ptr m_db;

    void ensure_data_block() const;
	mutable std::shared_ptr<data_block> m_pdata;
    block_id m_data;

    void ensure_sub_block() const;
	mutable std::shared_ptr<subnode_block> m_psub;
    block_id m_sub;

    node_id m_id;
    node_id m_parent_id;
    node_id m_root_node_id;
};

class block
{
public:
    block(const shared_db_ptr& db, size_t size, block_id id, ulonglong address)
        : m_db(db), m_size(size), m_id(id), m_address(address) { }

    virtual ~block() { }

    size_t size() const { return m_size; }
    block_id get_id() const { return m_id; }
    ulonglong get_address() const { return m_address; }
    
protected:
    shared_db_ptr m_db;
    size_t m_size;
    block_id m_id;
    ulonglong m_address;
};

class data_block : public block
{
public:
    data_block(const shared_db_ptr& db, size_t size, size_t t_size, block_id id, ulonglong address)
        : block(db, size, id, address), m_total_size(t_size) { }
    virtual ~data_block() { }

    virtual size_t read(std::vector<byte>& buffer, ulong offset) const = 0;
    virtual uint get_page_count() const = 0;
    virtual std::shared_ptr<external_block> get_page(uint page_num) = 0;
    virtual std::shared_ptr<const external_block> get_page(uint page_num) const = 0;
    virtual std::shared_ptr<data_block> clone() const = 0;
    
    size_t total_size() const { return m_total_size; }

protected:
    size_t m_total_size;
};

class extended_block : public data_block
{
public:
    extended_block(const shared_db_ptr& db, size_t size, size_t total_size, size_t sub_size, ulong sub_page_count, ushort level, block_id id, ulonglong address, std::vector<block_id>& bi)
        : data_block(db, size, total_size, id, address), m_sub_size(sub_size), m_sub_page_count(sub_page_count), m_level(level), m_block_info(bi), m_child_blocks(bi.size()) { }
    extended_block(const shared_db_ptr& db, size_t size, size_t total_size, size_t sub_size, ulong sub_page_count, ushort level, block_id id, ulonglong address, std::vector<block_id>&& bi)
        : data_block(db, size, total_size, id, address), m_sub_size(sub_size), m_sub_page_count(sub_page_count), m_level(level), m_block_info(bi), m_child_blocks(bi.size()) { }
    extended_block(const extended_block& other)
        : data_block(other), m_sub_size(other.m_sub_size), m_sub_page_count(other.m_sub_page_count), m_level(other.m_level), m_block_info(other.m_block_info), m_child_blocks(other.m_child_blocks.size()) { }
    extended_block(extended_block&& other)
        : data_block(other), m_sub_size(other.m_sub_size), m_sub_page_count(other.m_sub_page_count), m_level(other.m_level), m_block_info(std::move(other.m_block_info)), m_child_blocks(std::move(other.m_child_blocks)) { }
    
    extended_block& operator=(const extended_block& other);
    extended_block& operator=(extended_block&& other);
    
    size_t read(std::vector<byte>& buffer, ulong offset) const;
    uint get_page_count() const;
    std::shared_ptr<external_block> get_page(uint page_num);
    std::shared_ptr<const external_block> get_page(uint page_num) const;
    std::shared_ptr<data_block> clone() const;

    ushort get_level() const { return m_level; }

private:
    std::shared_ptr<data_block> get_child_block(uint index) const;

    size_t m_sub_size;
    ulong m_sub_page_count;
    ushort m_level;
    std::vector<block_id> m_block_info;
    mutable std::vector<std::shared_ptr<data_block>> m_child_blocks;
};

class external_block : public std::enable_shared_from_this<external_block>, public data_block
{
public:
    external_block(const shared_db_ptr& db, size_t size, size_t max_size, block_id id, ulonglong address, const std::vector<byte>& buffer)
        : data_block(db, size, size, id, address), m_buffer(buffer), m_max_size(max_size) { }

    external_block(const shared_db_ptr& db, size_t size, size_t max_size, block_id id, ulonglong address, std::vector<byte>&& buffer)
        : data_block(db, size, size, id, address), m_buffer(buffer), m_max_size(max_size) { }

    external_block(const external_block& other)
        : data_block(other), m_max_size(other.m_max_size), m_buffer(other.m_buffer) { }

    external_block(external_block&& other)
        : data_block(other), m_max_size(other.m_max_size), m_buffer(std::move(other.m_buffer)) { }

    external_block& operator=(const external_block& other);
    external_block& operator=(external_block&& other);

    size_t read(std::vector<byte>& buffer, ulong offset) const;
    uint get_page_count() const { return 1; }
    std::shared_ptr<external_block> get_page(uint page_num);
    std::shared_ptr<const external_block> get_page(uint page_num) const;
    std::shared_ptr<data_block> clone() const;

    const byte * get_ptr(ulong offset)
        { return &m_buffer[0] + offset; }

private:
    size_t m_max_size;
    std::vector<byte> m_buffer;
};

class subnode_block : public block, public virtual btree_node<node_id, node>
{
public:
    subnode_block(const shared_db_ptr& db, size_t size, node_id root_node_id, block_id id, ulonglong address, ushort level)
        : block(db, size, id, address), m_level(level), m_root_node_id(root_node_id) { }

    virtual ~subnode_block() { }

    ushort get_level() const { return m_level; }
    node_id get_root_node_id() const { return m_root_node_id; }
    
    virtual std::shared_ptr<subnode_block> clone() const = 0;

protected:
    ushort m_level;
    node_id m_root_node_id;
};

class subnode_nonleaf_block : public subnode_block, public btree_node_nonleaf<node_id, node>
{
public:
    subnode_nonleaf_block(const shared_db_ptr& db, size_t size, node_id root_node_id, block_id id, ulonglong address, ushort level, const std::vector<std::pair<node_id, block_id>>& subnode_info)
        : subnode_block(db, size, root_node_id, id, address, level), m_subnode_info(subnode_info) { }
     subnode_nonleaf_block(const shared_db_ptr& db, size_t size, node_id root_node_id, block_id id, ulonglong address, ushort level, std::vector<std::pair<node_id, block_id>>&& subnode_info)
        : subnode_block(db, size, root_node_id, id, address, level), m_subnode_info(subnode_info) { }

    const node_id& get_key(const uint pos) const
        { return m_subnode_info[pos].first; }

    btree_node<node_id, node>* get_child(const uint pos);
    const btree_node<node_id, node>* get_child(const uint pos) const;
    uint num_values() const { return m_subnode_info.size(); }
    
    virtual std::shared_ptr<subnode_block> clone() const;

private:
    std::vector<std::pair<node_id, block_id>> m_subnode_info;
    mutable std::vector<std::shared_ptr<subnode_block>> m_child_blocks;
};

class subnode_leaf_block : public subnode_block, public btree_node_leaf<node_id, node>
{
public:
    subnode_leaf_block(const shared_db_ptr& db, size_t size, node_id root_node_id, block_id id, ulonglong address, const std::vector<std::pair<node_id, node>>& subnodes)
        : subnode_block(db, size, root_node_id, id, address, 0), m_subnodes(subnodes) { }
     subnode_leaf_block(const shared_db_ptr& db, size_t size, node_id root_node_id, block_id id, ulonglong address, std::vector<std::pair<node_id, node>>&& subnodes)
        : subnode_block(db, size, root_node_id, id, address, 0), m_subnodes(subnodes) { }
    node& get_value(const uint pos)
        { return m_subnodes[pos].second; }
    const node& get_value(const uint pos) const 
        { return m_subnodes[pos].second; }
    const node_id& get_key(const uint pos) const
        { return m_subnodes[pos].first; }
    uint num_values() const
        { return m_subnodes.size(); }

	virtual std::shared_ptr<subnode_block> clone() const;
private:
    std::vector<std::pair<node_id, node>> m_subnodes;
};

} // end fairport namespace

inline fairport::node::node(const node& other)
: m_db(other.m_db), m_data(other.m_data), m_sub(other.m_sub), m_id(other.m_id), m_parent_id(other.m_parent_id), m_root_node_id(other.m_root_node_id) 
{ 
    if(other.m_pdata)
        m_pdata = other.m_pdata->clone();

    if(other.m_psub)
        m_psub = other.m_psub->clone();
}

inline fairport::node::node(node&& other)
: m_db(other.m_db), m_pdata(std::move(other.m_pdata)), m_data(other.m_data), m_psub(std::move(other.m_psub)), m_sub(other.m_sub), m_id(other.m_id), m_parent_id(other.m_parent_id), m_root_node_id(other.m_root_node_id) 
{ 
}

inline fairport::node& fairport::node::operator=(const node& other)
{
    if(this != &other)
    {
        m_db = other.m_db;
        
        m_pdata = other.m_pdata ? other.m_pdata->clone() : other.m_pdata;
        m_data = other.m_data;
        
        m_psub = other.m_psub ? other.m_psub->clone() : other.m_psub;
        m_sub = other.m_sub;
        m_id = other.m_id;
        m_parent_id = other.m_parent_id;
        m_root_node_id = other.m_root_node_id;
    }
}

inline fairport::node& fairport::node::operator=(node&& other)
{
    m_db = std::move(other.m_db);
    std::swap(m_pdata, other.m_pdata);
    m_data = other.m_data;
    std::swap(m_psub, other.m_psub);
    m_sub = other.m_sub;
    m_id = other.m_id;
    m_parent_id = other.m_parent_id;
    m_root_node_id = other.m_root_node_id;
}

inline const fairport::byte * fairport::node::get_ptr(uint page_num, ulong offset) const
{
    ensure_data_block();
    return m_pdata->get_page(page_num)->get_ptr(offset);
}

inline size_t fairport::node::size() const
{
    ensure_data_block();
    return m_pdata->total_size();
}

inline size_t fairport::node::get_page_size(uint page_num) const
{
    ensure_data_block();
    return m_pdata->get_page(page_num)->size();
}
    
inline fairport::uint fairport::node::get_page_count() const 
{ 
    ensure_data_block(); 
    return m_pdata->get_page_count(); 
}

inline size_t fairport::node::read(std::vector<byte>& buffer, ulong offset) const
{ 
    ensure_data_block(); 
    return m_pdata->read(buffer, offset); 
}
    
inline size_t fairport::node::read(std::vector<byte>& buffer, uint page_num, ulong offset) const
{ 
    ensure_data_block(); 
    return m_pdata->get_page(page_num)->read(buffer, offset); 
}

inline void fairport::node::ensure_data_block() const
{ 
    if(!m_pdata) 
        m_pdata = m_db->read_data_block(m_data); 
}
    
inline void fairport::node::ensure_sub_block() const
{ 
    if(!m_psub) 
        m_psub = m_db->read_subnode_block(get_root_node_id(), m_sub); 
}

inline fairport::btree_node<fairport::node_id, fairport::node>* fairport::subnode_nonleaf_block::get_child(const uint pos)
{
    if(m_child_blocks[pos] == NULL)
    {
        m_child_blocks[pos] = m_db->read_subnode_block(get_root_node_id(), m_subnode_info[pos].second);
    }

    return m_child_blocks[pos].get();
}

inline const fairport::btree_node<fairport::node_id, fairport::node>* fairport::subnode_nonleaf_block::get_child(const uint pos) const
{
    if(m_child_blocks[pos] == NULL)
    {
        m_child_blocks[pos] = m_db->read_subnode_block(get_root_node_id(), m_subnode_info[pos].second);
    }

    return m_child_blocks[pos].get();
}

inline fairport::extended_block& fairport::extended_block::operator=(const extended_block& other)
{
    if(this != &other)
    {
        data_block::operator=(other);
        m_sub_size = other.m_sub_size;
        m_sub_page_count = other.m_sub_page_count;
        m_level = other.m_level;
        m_block_info = other.m_block_info;
   
        m_child_blocks.resize(0); 
        m_child_blocks.resize(other.m_child_blocks.size());
    }

    return *this;
}

inline fairport::extended_block& fairport::extended_block::operator=(extended_block&& other)
{
    data_block::operator=(other);

    m_sub_size = other.m_sub_size;
    m_sub_page_count = other.m_sub_page_count;
    m_level = other.m_level;
    m_block_info = std::move(other.m_block_info);
    m_child_blocks = std::move(other.m_child_blocks);
}

inline fairport::uint fairport::extended_block::get_page_count() const
{
    assert(m_sub_size % m_sub_page_count == 0);
    uint page_size = m_sub_size / m_sub_page_count;
    uint page_count = (total_size() / page_size) + ((total_size() % page_size) != 0 ? 1 : 0);
    assert(get_level() == 2 || page_count == m_block_info.size());

    return page_count;
}

inline std::shared_ptr<fairport::data_block> fairport::extended_block::get_child_block(uint index) const
{
    if(index >= m_child_blocks.size())
        throw std::out_of_range("index >= m_child_blocks.size()");

    if(m_child_blocks[index] == NULL)
        m_child_blocks[index] = m_db->read_data_block(m_block_info[index]);

    return m_child_blocks[index];
}

inline std::shared_ptr<const fairport::external_block> fairport::extended_block::get_page(uint page_num) const
{
    uint page = page_num / m_sub_page_count;
    return get_child_block(page)->get_page(page_num % m_sub_page_count);
}

inline std::shared_ptr<fairport::external_block> fairport::extended_block::get_page(uint page_num)
{
    uint page = page_num / m_sub_page_count;
    return get_child_block(page)->get_page(page_num % m_sub_page_count);
}

inline std::shared_ptr<const fairport::external_block> fairport::external_block::get_page(uint index) const
{
    if(index != 0)
        throw std::out_of_range("index > 0");

    return this->shared_from_this();
}

inline std::shared_ptr<fairport::external_block> fairport::external_block::get_page(uint index)
{
    if(index != 0)
        throw std::out_of_range("index > 0");

    return this->shared_from_this();
}

inline fairport::external_block& fairport::external_block::operator=(const external_block& other)
{
    if(this != &other)
    {
        // call base operator=
        data_block::operator=(other);

        m_buffer = other.m_buffer;
        m_max_size = other.m_max_size;
    }
}

inline fairport::external_block& fairport::external_block::operator=(external_block&& other)
{
    // call base operator=
    data_block::operator=(other);

    m_buffer = std::move(other.m_buffer);
    m_max_size = other.m_max_size;
}

inline size_t fairport::external_block::read(std::vector<byte>& buffer, ulong offset) const
{
    size_t read_size = buffer.size();
    
    if(size() > 0)
    {
        if(offset > size())
            throw std::out_of_range("offset > size()");

        if(offset + buffer.size() > size())
            read_size = size() - offset;

        memcpy(&buffer[0], &m_buffer[0] + offset, read_size);
    }

    return read_size;
}

inline std::shared_ptr<fairport::data_block> fairport::external_block::clone() const
{
    return std::shared_ptr<external_block>(new external_block(*this));
}

inline size_t fairport::extended_block::read(std::vector<byte>& buffer, ulong offset) const
{
    std::vector<byte> sub_page_buffer(m_sub_size);
    size_t total_bytes_read = 0, bytes_read = 0;
    size_t size = buffer.size();

    if(offset > total_size())
        throw std::out_of_range("offset > total_size()");

    if(offset + size > total_size())
        size = total_size() - offset;

    uint pos = offset / m_sub_size;
    uint pos_starting_offset = pos * m_sub_size;

    while(size > 0)
    {
        bytes_read += get_child_block(pos)->read(sub_page_buffer, offset - pos_starting_offset);
        memcpy(&buffer[0] + total_bytes_read, &sub_page_buffer[0], bytes_read);

        offset += bytes_read;
        size -= bytes_read;
        total_bytes_read += bytes_read;
        pos_starting_offset += m_sub_size;
        ++pos;
    }

    return total_bytes_read;
}

inline std::shared_ptr<fairport::data_block> fairport::extended_block::clone() const
{
    return std::shared_ptr<data_block>(new extended_block(*this));
}

inline fairport::node_iterator fairport::node::subnode_begin() 
{
    ensure_sub_block();
    return m_psub->begin();
}

inline fairport::const_node_iterator fairport::node::subnode_begin() const
{
    ensure_sub_block();
    return std::const_pointer_cast<const subnode_block>(m_psub)->begin();
}

inline fairport::node_iterator fairport::node::subnode_end()
{
    ensure_sub_block();
    return m_psub->end();
}

inline fairport::const_node_iterator fairport::node::subnode_end() const
{
    ensure_sub_block();
    return std::const_pointer_cast<const subnode_block>(m_psub)->end();
}

inline fairport::node& fairport::node::lookup(node_id id)
{
    ensure_sub_block();
    return m_psub->lookup(id);
}

inline const fairport::node& fairport::node::lookup(node_id id) const
{
    ensure_sub_block();
    return std::const_pointer_cast<const subnode_block>(m_psub)->lookup(id);
}

inline std::shared_ptr<fairport::subnode_block> fairport::subnode_leaf_block::clone() const
{
	return std::shared_ptr<subnode_block>(new subnode_leaf_block(*this));
}

inline std::shared_ptr<fairport::subnode_block> fairport::subnode_nonleaf_block::clone() const
{
	return std::shared_ptr<subnode_block>(new subnode_nonleaf_block(*this));
}

#endif
