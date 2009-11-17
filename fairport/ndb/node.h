#ifndef FAIRPORT_NDB_NODE_H
#define FAIRPORT_NDB_NODE_H

#include <vector>
#include <algorithm>
#include <memory>
#include <cassert>

#include "fairport/util/util.h"
#include "fairport/util/btree.h"

#include "fairport/ndb/database_iface.h"

namespace fairport
{

class node_impl : public std::enable_shared_from_this<node_impl>
{
public:
    // constructor for top level nodes
    node_impl(const shared_db_ptr& db, const node_info& info)
        : m_db(db), m_id(info.id), m_original_data_id(info.data_bid), m_original_sub_id(info.sub_bid), m_original_parent_id(info.parent_id), m_parent_id(info.parent_id) { } 

    // constructor for subnodes
    node_impl(const std::shared_ptr<node_impl>& container_node, const subnode_info& info)
        : m_db(container_node->m_db), m_pcontainer_node(container_node), m_id(info.id), m_original_data_id(info.data_bid), m_original_sub_id(info.sub_bid), m_original_parent_id(0), m_parent_id(0) { } 

    node_impl& operator=(const node_impl& other)
        { m_pdata = other.m_pdata; m_psub = other.m_psub; return *this; }

    node_id get_id() const { return m_id; }
    block_id get_data_id() const;
    block_id get_sub_id() const; 

    node_id get_parent_id() const { return m_parent_id; }
    bool is_subnode() { return m_pcontainer_node; }

    std::shared_ptr<data_block> get_data_block() const
        { ensure_data_block(); return m_pdata; }
    std::shared_ptr<subnode_block> get_subnode_block() const 
        { ensure_sub_block(); return m_psub; }
   
    size_t read(std::vector<byte>& buffer, ulong offset) const;
    size_t read(std::vector<byte>& buffer, uint page_num, ulong offset) const;

    size_t size() const;
    size_t get_page_size(uint page_num) const;
    uint get_page_count() const;

    // iterate over subnodes
    subnode_iterator subnode_begin();
    const_subnode_iterator subnode_begin() const;
    subnode_iterator subnode_end();
    const_subnode_iterator subnode_end() const;
    node lookup(node_id id) const;

    const byte * get_ptr(uint page_num, ulong offset) const;

private:
    node_impl(); // = delete

    void ensure_data_block() const;
    void ensure_sub_block() const;
    
    const node_id m_id;
    block_id m_original_data_id;
    block_id m_original_sub_id;
    node_id m_original_parent_id;

    mutable std::shared_ptr<data_block> m_pdata;
    mutable std::shared_ptr<subnode_block> m_psub;
    node_id m_parent_id;
    std::shared_ptr<node_impl> m_pcontainer_node;

    shared_db_ptr m_db;
};

class node
{
public:
    // constructor for top level nodes
    node(const shared_db_ptr& db, const node_info& info)
        : m_pimpl(new node_impl(db, info)) { }

    // constructor for subnodes
    node(const node& container_node, const subnode_info& info)
        : m_pimpl(new node_impl(container_node.m_pimpl, info)) { }
    node(const std::shared_ptr<node_impl>& container_node, const subnode_info& info)
        : m_pimpl(new node_impl(container_node, info)) { }


    node(const node& other)
        : m_pimpl(new node_impl(*other.m_pimpl)) { }
    node(node&& other)
        : m_pimpl(std::move(other.m_pimpl)) { }

    node& operator=(const node& other)
        { m_pimpl = other.m_pimpl; return *this; }

    node_id get_id() const { return m_pimpl->get_id(); }
    block_id get_data_id() const { return m_pimpl->get_data_id(); }
    block_id get_sub_id() const { return m_pimpl->get_sub_id(); }

    node_id get_parent_id() const { return m_pimpl->get_parent_id(); } 
    bool is_subnode() { return m_pimpl->is_subnode(); } 

    std::shared_ptr<data_block> get_data_block() const
        { return m_pimpl->get_data_block(); }
    std::shared_ptr<subnode_block> get_subnode_block() const 
        { return m_pimpl->get_subnode_block(); } 
   
    size_t read(std::vector<byte>& buffer, ulong offset) const
        { return m_pimpl->read(buffer, offset); }
    size_t read(std::vector<byte>& buffer, uint page_num, ulong offset) const
        { return m_pimpl->read(buffer, page_num, offset); }

    size_t size() const { return m_pimpl->size(); }
    size_t get_page_size(uint page_num) const 
        { return m_pimpl->get_page_size(page_num); }
    uint get_page_count() const { return m_pimpl->get_page_count(); }

    // iterate over subnodes
    subnode_iterator subnode_begin() { return m_pimpl->subnode_begin(); }
    const_subnode_iterator subnode_begin() const
        { return std::const_pointer_cast<const node_impl>(m_pimpl)->subnode_begin(); }
    subnode_iterator subnode_end() { return m_pimpl->subnode_end(); }
    const_subnode_iterator subnode_end() const
        { return std::const_pointer_cast<const node_impl>(m_pimpl)->subnode_end(); } 
    node lookup(node_id id) const
        { return m_pimpl->lookup(id); }

    const byte * get_ptr(uint page_num, ulong offset) const
        { return m_pimpl->get_ptr(page_num, offset); }

private:
    node(); // = delete

    std::shared_ptr<node_impl> m_pimpl;
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
    virtual std::shared_ptr<external_block> get_page(uint page_num) const = 0;
    
    size_t total_size() const { return m_total_size; }

protected:
    size_t m_total_size;
};

class extended_block : public data_block, public std::enable_shared_from_this<extended_block>
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
    
    
    size_t read(std::vector<byte>& buffer, ulong offset) const;
    uint get_page_count() const;
    std::shared_ptr<external_block> get_page(uint page_num) const;

    ushort get_level() const { return m_level; }

private:
    extended_block& operator=(const extended_block& other); // = delete
    extended_block& operator=(extended_block&& other); // = delete

    std::shared_ptr<data_block> get_child_block(uint index) const;

    size_t m_sub_size;
    ulong m_sub_page_count;
    ushort m_level;
    std::vector<block_id> m_block_info;
    mutable std::vector<std::shared_ptr<data_block>> m_child_blocks;
};

class external_block : public data_block, public std::enable_shared_from_this<external_block>
{
public:
    external_block(const shared_db_ptr& db, size_t size, size_t max_size, block_id id, ulonglong address, const std::vector<byte>& buffer)
        : data_block(db, size, size, id, address), m_buffer(buffer), m_max_size(max_size) { }

    external_block(const shared_db_ptr& db, size_t size, size_t max_size, block_id id, ulonglong address, std::vector<byte>&& buffer)
        : data_block(db, size, size, id, address), m_buffer(buffer), m_max_size(max_size) { }

    size_t read(std::vector<byte>& buffer, ulong offset) const;
    uint get_page_count() const { return 1; }
    std::shared_ptr<external_block> get_page(uint page_num) const;

    const byte * get_ptr(ulong offset)
        { return &m_buffer[0] + offset; }

private:
    external_block(const external_block&); // = delete
    external_block& operator=(const external_block&); // = delete

    size_t m_max_size;
    std::vector<byte> m_buffer;
};

class subnode_block : public block, public virtual btree_node<node_id, subnode_info>
{
public:
    subnode_block(const shared_db_ptr& db, size_t size, block_id id, ulonglong address, ushort level)
        : block(db, size, id, address), m_level(level) { }

    virtual ~subnode_block() { }

    ushort get_level() const { return m_level; }
    
protected:
    ushort m_level;
};

class subnode_nonleaf_block : public subnode_block, public btree_node_nonleaf<node_id, subnode_info>
{
public:
    subnode_nonleaf_block(const shared_db_ptr& db, size_t size, block_id id, ulonglong address, const std::vector<std::pair<node_id, block_id>>& subnodes)
        : subnode_block(db, size, id, address, 1), m_subnode_info(subnodes) { }
    subnode_nonleaf_block(const shared_db_ptr& db, size_t size, block_id id, ulonglong address, std::vector<std::pair<node_id, block_id>>&& subnodes)
        : subnode_block(db, size, id, address, 1), m_subnode_info(subnodes) { }

    const node_id& get_key(uint pos) const
        { return m_subnode_info[pos].first; }

    btree_node<node_id, subnode_info>* get_child(uint pos);
    const btree_node<node_id, subnode_info>* get_child(uint pos) const;
    uint num_values() const { return m_subnode_info.size(); }
    
private:
    std::vector<std::pair<node_id, block_id>> m_subnode_info;
    mutable std::vector<std::shared_ptr<subnode_block>> m_child_blocks;
};

class subnode_leaf_block : public subnode_block, public btree_node_leaf<node_id, subnode_info>
{
public:
    subnode_leaf_block(const shared_db_ptr& db, size_t size, block_id id, ulonglong address, const std::vector<std::pair<node_id, subnode_info>>& subnodes)
        : subnode_block(db, size, id, address, 0), m_subnodes(subnodes) { }
    subnode_leaf_block(const shared_db_ptr& db, size_t size, block_id id, ulonglong address, std::vector<std::pair<node_id, subnode_info>>&& subnodes)
        : subnode_block(db, size, id, address, 0), m_subnodes(subnodes) { }
    subnode_info& get_value(uint pos)
        { return m_subnodes[pos].second; }
    const subnode_info& get_value(uint pos) const 
        { return m_subnodes[pos].second; }
    const node_id& get_key(uint pos) const
        { return m_subnodes[pos].first; }
    uint num_values() const
        { return m_subnodes.size(); }

private:
    std::vector<std::pair<node_id, subnode_info>> m_subnodes;
};

} // end fairport namespace

inline fairport::block_id fairport::node_impl::get_data_id() const
{ 
    if(m_pdata)
        return m_pdata->get_id();
    
    return m_original_data_id;
}

inline fairport::block_id fairport::node_impl::get_sub_id() const
{ 
    if(m_psub)
        return m_psub->get_id();
    
    return m_original_sub_id;
}

inline const fairport::byte * fairport::node_impl::get_ptr(uint page_num, ulong offset) const
{
    return get_data_block()->get_page(page_num)->get_ptr(offset);
}

inline size_t fairport::node_impl::size() const
{
    return get_data_block()->total_size();
}

inline size_t fairport::node_impl::get_page_size(uint page_num) const
{
    return get_data_block()->get_page(page_num)->size();
}
    
inline fairport::uint fairport::node_impl::get_page_count() const 
{ 
    return get_data_block()->get_page_count(); 
}

inline size_t fairport::node_impl::read(std::vector<byte>& buffer, ulong offset) const
{ 
    return get_data_block()->read(buffer, offset); 
}
    
inline size_t fairport::node_impl::read(std::vector<byte>& buffer, uint page_num, ulong offset) const
{ 
    return get_data_block()->get_page(page_num)->read(buffer, offset); 
}

inline void fairport::node_impl::ensure_data_block() const
{ 
    if(!m_pdata) 
        m_pdata = m_db->read_data_block(m_original_data_id); 
}
    
inline void fairport::node_impl::ensure_sub_block() const
{ 
    if(!m_psub) 
        m_psub = m_db->read_subnode_block(m_original_sub_id); 
}

inline fairport::btree_node<fairport::node_id, fairport::subnode_info>* fairport::subnode_nonleaf_block::get_child(uint pos)
{
    if(m_child_blocks[pos] == NULL)
    {
        m_child_blocks[pos] = m_db->read_subnode_block(m_subnode_info[pos].second);
    }

    return m_child_blocks[pos].get();
}

inline const fairport::btree_node<fairport::node_id, fairport::subnode_info>* fairport::subnode_nonleaf_block::get_child(uint pos) const
{
    if(m_child_blocks[pos] == NULL)
    {
        m_child_blocks[pos] = m_db->read_subnode_block(m_subnode_info[pos].second);
    }

    return m_child_blocks[pos].get();
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

inline std::shared_ptr<fairport::external_block> fairport::extended_block::get_page(uint page_num) const
{
    uint page = page_num / m_sub_page_count;
    return get_child_block(page)->get_page(page_num % m_sub_page_count);
}

inline std::shared_ptr<fairport::external_block> fairport::external_block::get_page(uint index) const
{
    if(index != 0)
        throw std::out_of_range("index > 0");

    return std::const_pointer_cast<external_block>(this->shared_from_this());
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

inline fairport::subnode_iterator fairport::node_impl::subnode_begin() 
{
    return get_subnode_block()->begin();
}

inline fairport::const_subnode_iterator fairport::node_impl::subnode_begin() const
{
    return std::const_pointer_cast<const subnode_block>(get_subnode_block())->begin();
}

inline fairport::subnode_iterator fairport::node_impl::subnode_end()
{
    return get_subnode_block()->end();
}

inline fairport::const_subnode_iterator fairport::node_impl::subnode_end() const
{
    return std::const_pointer_cast<const subnode_block>(get_subnode_block())->end();
}

inline fairport::node fairport::node_impl::lookup(node_id id) const
{
    return node(std::const_pointer_cast<node_impl>(shared_from_this()), get_subnode_block()->lookup(id));
}
#endif
