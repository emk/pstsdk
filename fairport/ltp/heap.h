#ifndef FAIRPORT_LTP_HEAP_H
#define FAIRPORT_LTP_HEAP_H

#include <vector>
#include <algorithm>

#include "fairport/util/primatives.h"

#include "fairport/disk/disk.h"

#include "fairport/ndb/node.h"

namespace fairport
{

template<typename K, typename V>
class bth_nonleaf_node;

template<typename K, typename V>
class bth_leaf_node;

template<typename K, typename V>
class bth_node;

class heap_impl : public std::enable_shared_from_this<heap_impl>
{
public:
    size_t size(heap_id id) const;
    uint get_page_count() const { return m_node.get_page_count(); }
    heap_id get_root_id() const;
    byte get_client_signature() const;
    size_t read(std::vector<byte>& buffer, heap_id id, ulong offset) const;
    std::vector<byte> read(heap_id id) const;
    const node& get_node() const { return m_node; }
    node& get_node() { return m_node; }
    template<typename K, typename V>
    bth_node<K,V>* open_bth(heap_id root);
    
    friend class heap;

private:
    heap_impl();
    explicit heap_impl(const node& n);
    heap_impl(const node& n, byte client_sig);
    heap_impl(const heap_impl& other) 
        : m_node(other.m_node) { }

    node m_node;
};

typedef std::shared_ptr<heap_impl> heap_ptr;

class heap
{
public:
    explicit heap(const node& n)
        : m_pheap(new heap_impl(n)) { }
    heap(const node& n, byte client_sig)
        : m_pheap(new heap_impl(n, client_sig)) { }
    heap(const heap& other)
        : m_pheap(new heap_impl(*(other.m_pheap))) { }
    heap(heap&& other)
        : m_pheap(std::move(other.m_pheap)) { }

    heap& operator=(const heap& other);
    heap& operator=(heap&& other)
        { std::swap(m_pheap, other.m_pheap); return *this; }

    size_t size(heap_id id) const
        { return m_pheap->size(id); }
    heap_id get_root_id() const
        { return m_pheap->get_root_id(); }
    byte get_client_signature() const
        { return m_pheap->get_client_signature(); }
    size_t read(std::vector<byte>& buffer, heap_id id, ulong offset) const
        { return m_pheap->read(buffer, id, offset); }
    std::vector<byte> read(heap_id id) const
        { return m_pheap->read(id); }

    const node& get_node() const
        { return m_pheap->get_node(); }
    node& get_node()
        { return m_pheap->get_node(); }
    
    // caller owns
    template<typename K, typename V>
    bth_node<K,V>* open_bth(heap_id root)
        { return m_pheap->open_bth<K,V>(root); }

private:
    heap_ptr m_pheap;
};

template<typename K, typename V>
class bth_node : public virtual btree_node<K,V>
{
public:
    static bth_node<K,V>* open_root(const heap_ptr& h, heap_id bth_root);
    static bth_nonleaf_node<K,V>* open_nonleaf(const heap_ptr& h, heap_id id, ushort level);
    static bth_leaf_node<K,V>* open_leaf(const heap_ptr& h, heap_id id);

    bth_node(const heap_ptr& h, heap_id id, ushort level)
        : m_heap(h), m_id(id), m_level(level) { }
    virtual ~bth_node() { }

    heap_id get_id() const { return m_id; }
    ushort get_level() const { return m_level; }
    size_t get_key_size() const { return sizeof(K); }
    size_t get_value_size() const { return sizeof(V); }

    const heap_ptr get_heap_ptr() const { return m_heap; }
    heap_ptr get_heap_ptr() { return m_heap; }

    const node& get_node() const { return m_heap->get_node(); }
    node& get_node() { return m_heap->get_node(); }

protected:
    heap_ptr m_heap;

private:
    bth_node(const bth_node& other); // = delete
    bth_node& operator=(const bth_node& other); // = delete
    heap_id m_id;
    ushort m_level;
};

template<typename K, typename V>
class bth_nonleaf_node : public bth_node<K,V>, public btree_node_nonleaf<K,V>
{
public:
    bth_nonleaf_node(const heap_ptr& h, heap_id id, ushort level, const std::vector<std::pair<K, heap_id>>& bth_info)
        : bth_node<K,V>(h, id, level), m_bth_info(bth_info), m_child_nodes(bth_info.size(), NULL) { }
    bth_nonleaf_node(const heap_ptr& h, heap_id id, ushort level, std::vector<std::pair<K, heap_id>>&& bth_info)
        : bth_node<K,V>(h, id, level), m_bth_info(bth_info), m_child_nodes(bth_info.size(), NULL) { }
    virtual ~bth_nonleaf_node();

    // btree_node_nonleaf implementation
    const K& get_key(uint pos) const { return m_bth_info[pos].first; }
    bth_node<K,V>* get_child(uint pos);
    const bth_node<K,V>* get_child(uint pos) const;
    uint num_values() const { return m_child_nodes.size(); }

private:
    std::vector<std::pair<K, heap_id>> m_bth_info;
    mutable std::vector<bth_node<K,V>*> m_child_nodes;
};

template<typename K, typename V>
class bth_leaf_node : public bth_node<K,V>, public btree_node_leaf<K,V>
{
public:
    bth_leaf_node(const heap_ptr& h, heap_id id, const std::vector<std::pair<K,V>>& data)
        : bth_node<K,V>(h, id, 0), m_bth_data(data) { }
    bth_leaf_node(const heap_ptr& h, heap_id id, std::vector<std::pair<K,V>>&& data)
        : bth_node<K,V>(h, id, 0), m_bth_data(data) { }
    virtual ~bth_leaf_node() { }

    // btree_node_leaf implementation
    V& get_value(uint pos)
        { return m_bth_data[pos].second; }
    const V& get_value(uint pos) const
        { return m_bth_data[pos].second; }
    const K& get_key(uint pos) const
        { return m_bth_data[pos].first; }
    uint num_values() const
        { return m_bth_data.size(); }

private:
    std::vector<std::pair<K,V>> m_bth_data;
};

} // end fairport namespace

inline fairport::heap& fairport::heap::operator=(const heap& other)
{
    if(this == &other)
        return *this;

    m_pheap = heap_ptr(new heap_impl(*other.m_pheap));
    return *this;
}

template<typename K, typename V>
inline fairport::bth_node<K,V>* fairport::bth_node<K,V>::open_root(const heap_ptr& h, heap_id bth_root)
{
    disk::bth_header* pheader;
    std::vector<byte> buffer(sizeof(disk::bth_header));
    pheader = (disk::bth_header*)&buffer[0];

    h->read(buffer, bth_root, 0);

    if(pheader->bth_signature != disk::heap_sig_bth)
        throw sig_mismatch("bth_signature expected");

    if(pheader->key_size != sizeof(K))
        throw std::logic_error("invalid key size");

    if(pheader->entry_size != sizeof(V))
        throw std::logic_error("invalid entry size");

    if(pheader->num_levels > 1)
        return open_nonleaf(h, pheader->root, pheader->num_levels-1);
    else
        return open_leaf(h, pheader->root);
}

template<typename K, typename V>
inline fairport::bth_nonleaf_node<K,V>* fairport::bth_node<K,V>::open_nonleaf(const heap_ptr& h, heap_id id, ushort level)
{
    uint num_entries = h->size(id) / sizeof(disk::bth_nonleaf_entry<K>);
    std::vector<byte> buffer(h->size(id));
    disk::bth_nonleaf_node<K>* pbth_nonleaf_node = (disk::bth_nonleaf_node<K>*)&buffer[0];
    std::vector<std::pair<K, heap_id>> child_nodes;

    h->read(buffer, id, 0);

    child_nodes.reserve(num_entries);

    for(uint i = 0; i < num_entries; ++i)
    {
        child_nodes.push_back(std::make_pair(pbth_nonleaf_node->entries[i].key, pbth_nonleaf_node->entries[i].page));
    }

    return new bth_nonleaf_node<K,V>(h, id, level, std::move(child_nodes));
}
    
template<typename K, typename V>
inline fairport::bth_leaf_node<K,V>* fairport::bth_node<K,V>::open_leaf(const heap_ptr& h, heap_id id)
{
    std::vector<std::pair<K, V>> entries; 

    if(id)
    {
        uint num_entries = h->size(id) / sizeof(disk::bth_leaf_entry<K,V>);
        std::vector<byte> buffer(h->size(id));
        disk::bth_leaf_node<K,V>* pbth_leaf_node = (disk::bth_leaf_node<K,V>*)&buffer[0];

        h->read(buffer, id, 0);

        entries.reserve(num_entries);

        for(uint i = 0; i < num_entries; ++i)
        {
            entries.push_back(std::make_pair(pbth_leaf_node->entries[i].key, pbth_leaf_node->entries[i].value));
        }

        return new bth_leaf_node<K,V>(h, id, std::move(entries));
    }
    else
    {
        // id == 0 means an empty tree
        return new bth_leaf_node<K,V>(h, id, entries);
    }
}

template<typename K, typename V>
inline fairport::bth_nonleaf_node<K,V>::~bth_nonleaf_node()
{
    std::for_each(m_child_nodes.cbegin(), m_child_nodes.cend(), [](bth_node<K,V>* pnode) 
    {
        delete pnode;
    });
   
}

template<typename K, typename V>
inline fairport::bth_node<K,V>* fairport::bth_nonleaf_node<K,V>::get_child(uint pos)
{
    if(m_child_nodes[pos] == NULL)
    {
        if(this->get_level() > 1)
            m_child_nodes[pos] = bth_node<K,V>::open_nonleaf(this->get_heap_ptr(), m_bth_info[pos].second, this->get_level()-1);
        else
            m_child_nodes[pos] = bth_node<K,V>::open_leaf(this->get_heap_ptr(), m_bth_info[pos].second);
    }

    return m_child_nodes[pos];
}

template<typename K, typename V>
inline const fairport::bth_node<K,V>* fairport::bth_nonleaf_node<K,V>::get_child(uint pos) const
{
    if(m_child_nodes[pos] == NULL)
    {
        if(this->get_level() > 1)
            m_child_nodes[pos] = bth_node<K,V>::open_nonleaf(this->get_heap_ptr(), m_bth_info[pos].second, this->get_level()-1);
        else
            m_child_nodes[pos] = bth_node<K,V>::open_leaf(this->get_heap_ptr(), m_bth_info[pos].second);
    }

    return m_child_nodes[pos];
}

inline fairport::heap_impl::heap_impl(const node& n)
: m_node(n)
{
    // need to throw if the node is smaller than first_header
    disk::heap_first_header* pfirst_header = (disk::heap_first_header*)m_node.get_ptr(0, 0);

    if(pfirst_header->signature != disk::heap_signature)
        throw sig_mismatch("not a heap");
}

inline fairport::heap_impl::heap_impl(const node& n, byte client_sig)
: m_node(n)
{
    // need to throw if the node is smaller than first_header
    disk::heap_first_header* pfirst_header = (disk::heap_first_header*)m_node.get_ptr(0, 0);

    if(pfirst_header->signature != disk::heap_signature)
        throw sig_mismatch("not a heap");

    if(pfirst_header->signature != client_sig)
        throw sig_mismatch("mismatch client_sig");
}

inline fairport::heap_id fairport::heap_impl::get_root_id() const
{
    disk::heap_first_header* pfirst_header = (disk::heap_first_header*)m_node.get_ptr(0, 0);
    return pfirst_header->root_id;
}

inline fairport::byte fairport::heap_impl::get_client_signature() const
{
    disk::heap_first_header* pfirst_header = (disk::heap_first_header*)m_node.get_ptr(0, 0);
    return pfirst_header->client_signature;
}

inline size_t fairport::heap_impl::size(heap_id id) const
{
    disk::heap_page_header* pheader = (disk::heap_page_header*)m_node.get_ptr(get_heap_page(id), 0);

    if(pheader->page_map_offset > m_node.get_page_size(get_heap_page(id)))
        throw std::length_error("page_map_offset > node size");

    disk::heap_page_map* pmap = (disk::heap_page_map*)((byte*)pheader + pheader->page_map_offset);

    if(get_heap_index(id) > pmap->num_allocs)
        throw std::length_error("index > num_allocs");

    return pmap->allocs[get_heap_index(id) + 1] - pmap->allocs[get_heap_index(id)];
}

inline size_t fairport::heap_impl::read(std::vector<byte>& buffer, heap_id id, ulong offset) const
{
    if(buffer.size() > size(id))
        throw std::length_error("buffer.size() > size()");

    if(offset > size(id))
        throw std::length_error("offset > size()");

    if(offset + buffer.size() > size(id))
        throw std::length_error("size + offset > size()");

    disk::heap_page_header* pheader = (disk::heap_page_header*)m_node.get_ptr(get_heap_page(id), 0);

    disk::heap_page_map* pmap = (disk::heap_page_map*)((byte*)pheader + pheader->page_map_offset);

    return m_node.read(buffer, get_heap_page(id), pmap->allocs[get_heap_index(id)]);
}

inline std::vector<fairport::byte> fairport::heap_impl::read(heap_id id) const
{
    std::vector<byte> result(size(id));
    read(result, id, 0);
    return result;
}

template<typename K, typename V>
inline fairport::bth_node<K,V>* fairport::heap_impl::open_bth(heap_id root)
{ 
    return bth_node<K,V>::open_root(shared_from_this(), root); 
}

#endif
