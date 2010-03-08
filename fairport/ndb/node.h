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
    template<typename T> T read(ulong offset) const;
    size_t read(std::vector<byte>& buffer, uint page_num, ulong offset) const;
    template<typename T> T read(uint page_num, ulong offset) const;
    
    size_t write(const std::vector<byte>& buffer, ulong offset);
    template<typename T> void write(const T& obj, ulong offset);
    size_t write(const std::vector<byte>& buffer, uint page_num, ulong offset);
    template<typename T> void write(const T& obj, uint page_num, ulong offset);
    
    size_t size() const;
    size_t resize(size_t size);
    
    size_t get_page_size(uint page_num) const;
    uint get_page_count() const;

    // iterate over subnodes
    const_subnodeinfo_iterator subnode_begin() const;
    const_subnodeinfo_iterator subnode_end() const;
    node lookup(node_id id) const;

private:
    data_block* ensure_data_block() const;
    subnode_block* ensure_sub_block() const;
    
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
    node(const node& other, alias_tag)
        : m_pimpl(other.m_pimpl) { }
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
    template<typename T> T read(ulong offset) const
        { return m_pimpl->read<T>(offset); }
    size_t read(std::vector<byte>& buffer, uint page_num, ulong offset) const
        { return m_pimpl->read(buffer, page_num, offset); }
    template<typename T> T read(uint page_num, ulong offset) const
        { return m_pimpl->read<T>(page_num, offset); }

    size_t write(std::vector<byte>& buffer, ulong offset) 
        { return m_pimpl->write(buffer, offset); }
    template<typename T> void write(const T& obj, ulong offset)
        { return m_pimpl->write<T>(obj, offset); }
    size_t write(std::vector<byte>& buffer, uint page_num, ulong offset) 
        { return m_pimpl->write(buffer, page_num, offset); }
    template<typename T> void write(const T& obj, uint page_num, ulong offset)
        { return m_pimpl->write<T>(obj, page_num, offset); }

    size_t resize(size_t size)
        { return m_pimpl->resize(size); }

    size_t size() const { return m_pimpl->size(); }
    size_t get_page_size(uint page_num) const 
        { return m_pimpl->get_page_size(page_num); }
    uint get_page_count() const { return m_pimpl->get_page_count(); }

    // iterate over subnodes
    const_subnodeinfo_iterator subnode_begin() const
        { return m_pimpl->subnode_begin(); }
    const_subnodeinfo_iterator subnode_end() const
        { return m_pimpl->subnode_end(); } 
    node lookup(node_id id) const
        { return m_pimpl->lookup(id); }

private:
    std::shared_ptr<node_impl> m_pimpl;
};

class block
{
public:
    block(const shared_db_ptr& db, const block_info& info)
        : m_db(db), m_size(info.size), m_id(info.id), m_address(info.address), m_modified(false) { }
    block(const block& block)
        : m_db(block.m_db), m_size(block.m_size), m_id(0), m_address(0), m_modified(false) { }

    virtual ~block() { }

    virtual bool is_internal() const = 0;

    size_t get_disk_size() const { return m_size; }
    void set_disk_size(size_t new_size) { m_size = new_size; }    

    block_id get_id() const { return m_id; }
    ulonglong get_address() const { return m_address; }
    void set_address(ulonglong new_address) { m_address = new_address; }
    
    void touch();

protected:
    shared_db_ptr get_db_ptr() const { return shared_db_ptr(m_db); }
    virtual void trim() { }

    bool m_modified;
    weak_db_ptr m_db;
    size_t m_size;          // the size of this specific block on disk at last save
    block_id m_id;
    ulonglong m_address;    // the address of this specific block on disk, 0 if unknown
};

class data_block : public block
{
public:
    data_block(const shared_db_ptr& db, const block_info& info, size_t total_size)
        : block(db, info), m_total_size(total_size) { }
    virtual ~data_block() { }

    size_t read(std::vector<byte>& buffer, ulong offset) const;
    template<typename T> T read(ulong offset) const;
    virtual size_t read_raw(byte* pdest_buffer, size_t size, ulong offset) const = 0;

    size_t write(const std::vector<byte>& buffer, ulong offset, std::shared_ptr<data_block>& presult);
    template<typename T> void write(const T& buffer, ulong offset, std::shared_ptr<data_block>& presult);
    virtual size_t write_raw(const byte* pdest_buffer, size_t size, ulong offset, std::shared_ptr<data_block>& presult) = 0;

    virtual uint get_page_count() const = 0;
    virtual std::shared_ptr<external_block> get_page(uint page_num) const = 0;

    size_t get_total_size() const { return m_total_size; }
    virtual size_t resize(size_t size, std::shared_ptr<data_block>& presult) = 0;

protected:
    size_t m_total_size;    // the total or logical size (sum of all external child blocks)
};

class extended_block : 
    public data_block, 
    public std::enable_shared_from_this<extended_block>
{
public:
    // old block constructors (from disk)
    extended_block(const shared_db_ptr& db, const block_info& info, ushort level, size_t total_size, size_t child_max_total_size, ulong page_max_count, ulong child_page_max_count, const std::vector<block_id>& bi)
        : data_block(db, info, total_size), m_child_max_total_size(child_max_total_size), m_max_page_count(page_max_count), m_child_max_page_count(child_page_max_count), m_level(level), m_block_info(bi), m_child_blocks(bi.size()) { }
    extended_block(const shared_db_ptr& db, const block_info& info, ushort level, size_t total_size, size_t child_max_total_size, ulong page_max_count, ulong child_page_max_count, std::vector<block_id>&& bi)
        : data_block(db, info, total_size), m_child_max_total_size(child_max_total_size), m_max_page_count(page_max_count), m_child_max_page_count(child_page_max_count), m_level(level), m_block_info(bi)
        { m_child_blocks.resize(m_block_info.size()); }

    // new block constructors
    extended_block(const shared_db_ptr& db, ushort level, size_t total_size, size_t child_max_total_size, ulong page_max_count, ulong child_page_max_count, const std::vector<std::shared_ptr<data_block>>& child_blocks)
        : data_block(db, block_info(), total_size), m_child_max_total_size(child_max_total_size), m_max_page_count(page_max_count), m_child_max_page_count(child_page_max_count), m_level(level), m_block_info(child_blocks.size()), m_child_blocks(child_blocks) 
        { touch(); }
    extended_block(const shared_db_ptr& db, ushort level, size_t total_size, size_t child_max_total_size, ulong page_max_count, ulong child_page_max_count, std::vector<std::shared_ptr<data_block>>&& child_blocks)
        : data_block(db, block_info(), total_size), m_child_max_total_size(child_max_total_size), m_max_page_count(page_max_count), m_child_max_page_count(child_page_max_count), m_level(level), m_child_blocks(child_blocks)
        { m_block_info.resize(m_child_blocks.size()); touch(); }
    extended_block(const shared_db_ptr& db, ushort level, size_t total_size, size_t child_max_total_size, ulong page_max_count, ulong child_page_max_count);   
    
    size_t read_raw(byte* pdest_buffer, size_t size, ulong offset) const;
    size_t write_raw(const byte* pdest_buffer, size_t size, ulong offset, std::shared_ptr<data_block>& presult);
    
    uint get_page_count() const;
    std::shared_ptr<external_block> get_page(uint page_num) const;
    
    size_t resize(size_t size, std::shared_ptr<data_block>& presult);
    
    ushort get_level() const { return m_level; }
    bool is_internal() const { return true; }

private:
    extended_block& operator=(const extended_block& other); // = delete
    data_block* get_child_block(uint index) const;

    const size_t m_child_max_total_size;    // maximum (logical) size of a child block
    const ulong m_child_max_page_count;     // maximum number of child blocks a child can contain
    const ulong m_max_page_count;           // maximum number of child blocks this block can contain
    size_t get_max_size() const { return m_child_max_total_size * m_max_page_count; }

    const ushort m_level;
    std::vector<block_id> m_block_info;
    mutable std::vector<std::shared_ptr<data_block>> m_child_blocks;
};

class external_block : 
    public data_block, 
    public std::enable_shared_from_this<external_block>
{
public:
    // old block constructors (from disk)
    external_block(const shared_db_ptr& db, const block_info& info, size_t max_size, const std::vector<byte>& buffer)
        : data_block(db, info, info.size), m_buffer(buffer), m_max_size(max_size) { }
    external_block(const shared_db_ptr& db, const block_info& info, size_t max_size, std::vector<byte>&& buffer)
        : data_block(db, info, info.size), m_buffer(buffer), m_max_size(max_size) { }

    // new block constructors
    external_block(const shared_db_ptr& db, size_t max_size, size_t current_size)
        : data_block(db, block_info(), current_size), m_buffer(current_size), m_max_size(max_size) 
        { touch(); }

    size_t read_raw(byte* pdest_buffer, size_t size, ulong offset) const;
    size_t write_raw(const byte* pdest_buffer, size_t size, ulong offset, std::shared_ptr<data_block>& presult);

    uint get_page_count() const { return 1; }
    std::shared_ptr<external_block> get_page(uint page_num) const;

    size_t resize(size_t size, std::shared_ptr<data_block>& presult);

    bool is_internal() const { return false; }

private:
    external_block& operator=(const external_block& other); // = delete

    const size_t m_max_size;
    size_t get_max_size() const { return m_max_size; }

    std::vector<byte> m_buffer;
};

class subnode_block : 
    public block, 
    public virtual btree_node<node_id, subnode_info>
{
public:
    subnode_block(const shared_db_ptr& db, const block_info& info, ushort level)
        : block(db, info), m_level(level) { }

    virtual ~subnode_block() { }

    ushort get_level() const { return m_level; }

    bool is_internal() const { return true; }
    
protected:
    ushort m_level;
};

class subnode_nonleaf_block : 
    public subnode_block, 
    public btree_node_nonleaf<node_id, subnode_info>, 
    public std::enable_shared_from_this<subnode_nonleaf_block>
{
public:
    subnode_nonleaf_block(const shared_db_ptr& db, const block_info& info, const std::vector<std::pair<node_id, block_id>>& subnodes)
        : subnode_block(db, info, 1), m_subnode_info(subnodes) { }
    subnode_nonleaf_block(const shared_db_ptr& db, const block_info& info, std::vector<std::pair<node_id, block_id>>&& subnodes)
        : subnode_block(db, info, 1), m_subnode_info(subnodes) { }

    // btree_node_nonleaf implementation
    const node_id& get_key(uint pos) const
        { return m_subnode_info[pos].first; }
    subnode_block* get_child(uint pos);
    const subnode_block* get_child(uint pos) const;
    uint num_values() const { return m_subnode_info.size(); }
    
private:
    std::vector<std::pair<node_id, block_id>> m_subnode_info;
    mutable std::vector<std::shared_ptr<subnode_block>> m_child_blocks;
};

class subnode_leaf_block : 
    public subnode_block, 
    public btree_node_leaf<node_id, subnode_info>, 
    public std::enable_shared_from_this<subnode_leaf_block>
{
public:
    subnode_leaf_block(const shared_db_ptr& db, const block_info& info, const std::vector<std::pair<node_id, subnode_info>>& subnodes)
        : subnode_block(db, info, 0), m_subnodes(subnodes) { }
    subnode_leaf_block(const shared_db_ptr& db, const block_info& info, std::vector<std::pair<node_id, subnode_info>>&& subnodes)
        : subnode_block(db, info, 0), m_subnodes(subnodes) { }

    // btree_node_leaf implementation
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

inline size_t fairport::node_impl::size() const
{
    return ensure_data_block()->get_total_size();
}

inline size_t fairport::node_impl::get_page_size(uint page_num) const
{
    return ensure_data_block()->get_page(page_num)->get_total_size();
}
    
inline fairport::uint fairport::node_impl::get_page_count() const 
{ 
    return ensure_data_block()->get_page_count(); 
}

inline size_t fairport::node_impl::read(std::vector<byte>& buffer, ulong offset) const
{ 
    return ensure_data_block()->read(buffer, offset); 
}

template<typename T> 
inline T fairport::node_impl::read(ulong offset) const
{
    return ensure_data_block()->read<T>(offset); 
}
    
inline size_t fairport::node_impl::read(std::vector<byte>& buffer, uint page_num, ulong offset) const
{ 
    return ensure_data_block()->get_page(page_num)->read(buffer, offset); 
}

template<typename T> 
inline T fairport::node_impl::read(uint page_num, ulong offset) const
{
    return ensure_data_block()->get_page(page_num)->read<T>(offset); 
}

inline size_t fairport::node_impl::write(const std::vector<byte>& buffer, ulong offset)
{
    return ensure_data_block()->write(buffer, offset, m_pdata);
}

template<typename T> 
inline void fairport::node_impl::write(const T& obj, ulong offset)
{
    return ensure_data_block()->write<T>(obj, offset, m_pdata);
}

inline size_t fairport::node_impl::write(const std::vector<byte>& buffer, uint page_num, ulong offset)
{
    return ensure_data_block()->write(buffer, page_num * get_page_size(0) + offset, m_pdata);
}

template<typename T> 
inline void fairport::node_impl::write(const T& obj, uint page_num, ulong offset)
{
    return ensure_data_block()->write<T>(obj, page_num * get_page_size(0) + offset, m_pdata);
}

inline size_t fairport::node_impl::resize(size_t size)
{
    return ensure_data_block()->resize(size, m_pdata);
}

inline fairport::data_block* fairport::node_impl::ensure_data_block() const
{ 
    if(!m_pdata) 
        m_pdata = m_db->read_data_block(m_original_data_id); 

    return m_pdata.get();
}
    
inline fairport::subnode_block* fairport::node_impl::ensure_sub_block() const
{ 
    if(!m_psub) 
        m_psub = m_db->read_subnode_block(m_original_sub_id); 

    return m_psub.get();
}

inline void fairport::block::touch()
{ 
    if(!m_modified)
    {
        m_modified = true; 
        m_address = 0;
        m_size = 0;
        m_id = get_db_ptr()->alloc_bid(is_internal()); 
    }
}

inline fairport::subnode_block* fairport::subnode_nonleaf_block::get_child(uint pos)
{
    if(m_child_blocks[pos] == NULL)
    {
        m_child_blocks[pos] = get_db_ptr()->read_subnode_block(m_subnode_info[pos].second);
    }

    return m_child_blocks[pos].get();
}

inline const fairport::subnode_block* fairport::subnode_nonleaf_block::get_child(uint pos) const
{
    if(m_child_blocks[pos] == NULL)
    {
        m_child_blocks[pos] = get_db_ptr()->read_subnode_block(m_subnode_info[pos].second);
    }

    return m_child_blocks[pos].get();
}

inline size_t fairport::data_block::read(std::vector<byte>& buffer, ulong offset) const
{
    size_t read_size = buffer.size();
    
    if(read_size > 0)
    {
        if(offset >= get_total_size())
            throw std::out_of_range("offset >= size()");

        read_size = read_raw(&buffer[0], read_size, offset);
    }

    return read_size;
}

template<typename T> 
inline T fairport::data_block::read(ulong offset) const
{
    if(offset >= get_total_size())
        throw std::out_of_range("offset >= size()");
    if(sizeof(T) + offset > get_total_size())
        throw std::out_of_range("sizeof(T) + offset >= size()");

    T t;
    read_raw(reinterpret_cast<byte*>(&t), sizeof(T), offset);

    return t;
}

inline size_t fairport::data_block::write(const std::vector<byte>& buffer, ulong offset, std::shared_ptr<data_block>& presult)
{
    size_t write_size = buffer.size();
    
    if(write_size > 0)
    {
        if(offset >= get_total_size())
            throw std::out_of_range("offset >= size()");

        write_size = write_raw(&buffer[0], write_size, offset, presult);
    }

    return write_size;
}

template<typename T> 
void fairport::data_block::write(const T& buffer, ulong offset, std::shared_ptr<data_block>& presult)
{
    if(offset >= get_total_size())
        throw std::out_of_range("offset >= size()");
    if(sizeof(T) + offset > get_total_size())
        throw std::out_of_range("sizeof(T) + offset >= size()");

    (void)write_raw(reinterpret_cast<const byte*>(&buffer), sizeof(T), offset, presult);
}

inline fairport::uint fairport::extended_block::get_page_count() const
{
    assert(m_child_max_total_size % m_child_max_page_count == 0);
    uint page_size = m_child_max_total_size / m_child_max_page_count;
    uint page_count = (get_total_size() / page_size) + ((get_total_size() % page_size) != 0 ? 1 : 0);
    assert(get_level() == 2 || page_count == m_block_info.size());

    return page_count;
}

inline fairport::extended_block::extended_block(const shared_db_ptr& db, ushort level, size_t total_size, size_t child_max_total_size, ulong page_max_count, ulong child_page_max_count)
: data_block(db, block_info(), total_size), m_child_max_total_size(child_max_total_size), m_max_page_count(page_max_count), m_child_max_page_count(child_page_max_count), m_level(level)
{
    int total_subblocks = total_size / m_child_max_total_size;
    if(total_size % m_child_max_total_size != 0)
        total_subblocks++;

    m_child_blocks.resize(total_subblocks);
    m_block_info.resize(total_subblocks, 0);

    touch();
}

inline fairport::data_block* fairport::extended_block::get_child_block(uint index) const
{
    if(index >= m_child_blocks.size())
        throw std::out_of_range("index >= m_child_blocks.size()");

    if(m_child_blocks[index] == NULL)
    {
        if(m_block_info[index] == 0)
        {
            if(get_level() == 1)
                m_child_blocks[index] = get_db_ptr()->create_external_block(m_child_max_total_size);
            else
                m_child_blocks[index] = get_db_ptr()->create_extended_block(m_child_max_total_size);
        }
        else
            m_child_blocks[index] = get_db_ptr()->read_data_block(m_block_info[index]);
    }

    return m_child_blocks[index].get();
}

inline std::shared_ptr<fairport::external_block> fairport::extended_block::get_page(uint page_num) const
{
    uint page = page_num / m_child_max_page_count;
    return get_child_block(page)->get_page(page_num % m_child_max_page_count);
}

inline std::shared_ptr<fairport::external_block> fairport::external_block::get_page(uint index) const
{
    if(index != 0)
        throw std::out_of_range("index > 0");

    return std::const_pointer_cast<external_block>(this->shared_from_this());
}

inline size_t fairport::external_block::read_raw(byte* pdest_buffer, size_t buf_size, ulong offset) const
{
    size_t read_size = buf_size;

    assert(offset < get_total_size());

    if(offset + buf_size > get_total_size())
        read_size = get_total_size() - offset;

    memcpy(pdest_buffer, &m_buffer[offset], read_size);

    return read_size;
}

inline size_t fairport::external_block::write_raw(const byte* psrc_buffer, size_t buf_size, ulong offset, std::shared_ptr<fairport::data_block>& presult)
{
    std::shared_ptr<fairport::external_block> pblock = shared_from_this();
    if(pblock.use_count() > 2) // one for me, one for the caller
    {
        std::shared_ptr<fairport::external_block> pnewblock(new external_block(*this));
        return pnewblock->write_raw(psrc_buffer, buf_size, offset, presult);
    }
    touch(); // mutate ourselves inplace

    assert(offset < get_total_size());

    size_t write_size = buf_size;

    if(offset + buf_size > get_total_size())
        write_size = get_total_size() - offset;

    memcpy(&m_buffer[0]+offset, psrc_buffer, write_size);

    // assign out param
    presult = std::move(pblock);

    return write_size;
}

inline size_t fairport::extended_block::read_raw(byte* pdest_buffer, size_t buf_size, ulong offset) const
{
    assert(offset < get_total_size());

    if(offset + buf_size > get_total_size())
        buf_size = get_total_size() - offset;

    byte* pend = pdest_buffer + buf_size;

    size_t total_bytes_read = 0;

    while(pdest_buffer != pend)
    {
        // the child this read starts on
        uint child_pos = offset / m_child_max_total_size;
        // offset into the child block this read starts on
        ulong child_offset = offset % m_child_max_total_size;

        // call into our child to read the data
        size_t bytes_read = get_child_block(child_pos)->read_raw(pdest_buffer, buf_size, child_offset);
        assert(bytes_read <= buf_size);
    
        // adjust pointers accordingly
        pdest_buffer += bytes_read;
        offset += bytes_read;
        buf_size -= bytes_read;
        total_bytes_read += bytes_read;

        assert(pdest_buffer <= pend);
    }

    return total_bytes_read;
}

inline size_t fairport::extended_block::write_raw(const byte* psrc_buffer, size_t buf_size, ulong offset, std::shared_ptr<fairport::data_block>& presult)
{
    std::shared_ptr<extended_block> pblock = shared_from_this();
    if(pblock.use_count() > 2) // one for me, one for the caller
    {
        std::shared_ptr<extended_block> pnewblock(new extended_block(*this));
        return pnewblock->write_raw(psrc_buffer, buf_size, offset, presult);
    }
    touch(); // mutate ourselves inplace

    assert(offset < get_total_size());

    if(offset + buf_size > get_total_size())
        buf_size = get_total_size() - offset;

    const byte* pend = psrc_buffer + buf_size;
    size_t total_bytes_written = 0;

    while(psrc_buffer != pend)
    {
        // the child this read starts on
        uint child_pos = offset / m_child_max_total_size;
        // offset into the child block this read starts on
        ulong child_offset = offset % m_child_max_total_size;

        // call into our child to write the data
        size_t bytes_written = get_child_block(child_pos)->write_raw(psrc_buffer, buf_size, child_offset, m_child_blocks[child_pos]);
        assert(bytes_written <= buf_size);
    
        // adjust pointers accordingly
        psrc_buffer += bytes_written;
        offset += bytes_written;
        buf_size -= bytes_written;
        total_bytes_written += bytes_written;

        assert(psrc_buffer <= pend);
    }

    // assign out param
    presult = std::move(pblock);

    return total_bytes_written;
}

inline size_t fairport::external_block::resize(size_t size, std::shared_ptr<data_block>& presult)
{
    std::shared_ptr<external_block> pblock = shared_from_this();
    if(pblock.use_count() > 2) // one for me, one for the caller
    {
        std::shared_ptr<external_block> pnewblock(new external_block(*this));
        return pnewblock->resize(size, presult);
    }
    touch(); // mutate ourselves inplace

    m_buffer.resize(size > m_max_size ? m_max_size : size);
    m_total_size = m_buffer.size();

    if(size > get_max_size())
    {
        // we need to create an extended_block with us as the first entry
        std::shared_ptr<extended_block> pnewxblock = get_db_ptr()->create_extended_block(pblock);
        return pnewxblock->resize(size, presult);
    }

    // assign out param
    presult = std::move(pblock);

    return size;
}

inline size_t fairport::extended_block::resize(size_t size, std::shared_ptr<data_block>& presult)
{
    // calculate the number of subblocks needed
    uint old_num_subblocks = m_block_info.size();
    uint num_subblocks = size / m_child_max_total_size;
    
    if(size % m_child_max_total_size != 0)
        num_subblocks++;

    if(num_subblocks > m_max_page_count)
        num_subblocks = m_max_page_count;

    // defer to child if it's 1 (or less)
    assert(!m_child_blocks.empty());
    if(num_subblocks < 2)
        return get_child_block(0)->resize(size, presult);

    std::shared_ptr<extended_block> pblock = shared_from_this();
    if(pblock.use_count() > 2) // one for me, one for the caller
    {
        std::shared_ptr<extended_block> pnewblock(new extended_block(*this));
        return pnewblock->resize(size, presult);
    }
    touch(); // mutate ourselves inplace

    // set the total number of subblocks needed
    m_block_info.resize(num_subblocks, 0);
    m_child_blocks.resize(num_subblocks);

    if(old_num_subblocks < num_subblocks)
        get_child_block(old_num_subblocks-1)->resize(m_child_max_total_size, m_child_blocks[old_num_subblocks-1]);

    // size the last subblock appropriately
    size_t last_child_size = size - (num_subblocks-1) * m_child_max_total_size;
    get_child_block(num_subblocks-1)->resize(last_child_size, m_child_blocks[num_subblocks-1]);

    if(size > get_max_size())
    {
        m_total_size = get_max_size();

        if(get_level() == 2)
            throw can_not_resize("size > max_size");

        // we need to create a level 2 extended_block with us as the first entry
        std::shared_ptr<extended_block> pnewxblock = get_db_ptr()->create_extended_block(pblock);
        return pnewxblock->resize(size, presult);
    }
    
    // assign out param
    m_total_size = size;
    presult = std::move(pblock);

    return size;
}

inline fairport::const_subnodeinfo_iterator fairport::node_impl::subnode_begin() const
{
    const subnode_block* pblock = ensure_sub_block();
    return pblock->begin();
}

inline fairport::const_subnodeinfo_iterator fairport::node_impl::subnode_end() const
{
    const subnode_block* pblock = ensure_sub_block();
    return pblock->end();
}

inline fairport::node fairport::node_impl::lookup(node_id id) const
{
    return node(std::const_pointer_cast<node_impl>(shared_from_this()), ensure_sub_block()->lookup(id));
}
#endif
