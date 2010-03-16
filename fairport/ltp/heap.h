#ifndef FAIRPORT_LTP_HEAP_H
#define FAIRPORT_LTP_HEAP_H

#include <vector>
#include <algorithm>
#include <boost/noncopyable.hpp>
#include <boost/iostreams/concepts.hpp>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244)
#endif
#include <boost/iostreams/stream.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "fairport/util/primatives.h"

#include "fairport/disk/disk.h"

#include "fairport/ndb/node.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250)
#endif

namespace fairport
{

template<typename K, typename V>
class bth_nonleaf_node;

template<typename K, typename V>
class bth_leaf_node;

template<typename K, typename V>
class bth_node;

class heap_impl;
typedef std::shared_ptr<heap_impl> heap_ptr;

class hid_stream_device : public boost::iostreams::device<boost::iostreams::input_seekable, byte>
{
public:
    hid_stream_device() : m_hid(0), m_pos(0) { }
    std::streamsize read(byte* pbuffer, std::streamsize n); 
    std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

private:
    friend class heap_impl;
    hid_stream_device(const heap_ptr& _heap, heap_id id) : m_pos(0), m_hid(id), m_pheap(_heap) { }

    std::streamsize m_pos;
    heap_id m_hid;
    heap_ptr m_pheap;
};

typedef boost::iostreams::stream<hid_stream_device> hid_stream;

class heap_impl : public std::enable_shared_from_this<heap_impl>
{
public:
    size_t size(heap_id id) const;
    uint get_page_count() const { return m_node.get_page_count(); }
    heap_id get_root_id() const;
    byte get_client_signature() const;
    size_t read(std::vector<byte>& buffer, heap_id id, ulong offset) const;
    std::vector<byte> read(heap_id id) const;
    hid_stream_device open_stream(heap_id id);
    const node& get_node() const { return m_node; }
    node& get_node() { return m_node; }

    template<typename K, typename V>
    std::unique_ptr<bth_node<K,V>> open_bth(heap_id root);

    friend class heap;

private:
    heap_impl();
    explicit heap_impl(const node& n);
    heap_impl(const node& n, alias_tag);
    heap_impl(const node& n, byte client_sig);
    heap_impl(const node& n, byte client_sig, alias_tag);
    heap_impl(const heap_impl& other) 
        : m_node(other.m_node) { }

    node m_node;
};

class heap
{
public:
    explicit heap(const node& n)
        : m_pheap(new heap_impl(n)) { }
    heap(const node& n, alias_tag)
        : m_pheap(new heap_impl(n, alias_tag())) { }
    heap(const node& n, byte client_sig)
        : m_pheap(new heap_impl(n, client_sig)) { }
    heap(const node& n, byte client_sig, alias_tag)
        : m_pheap(new heap_impl(n, client_sig, alias_tag())) { }
    heap(const heap& other)
        : m_pheap(new heap_impl(*(other.m_pheap))) { }
    heap(const heap& other, alias_tag)
        : m_pheap(other.m_pheap) { }
    heap(heap&& other)
        : m_pheap(std::move(other.m_pheap)) { }

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
    hid_stream_device open_stream(heap_id id)
        { return m_pheap->open_stream(id); }

    const node& get_node() const
        { return m_pheap->get_node(); }
    node& get_node()
        { return m_pheap->get_node(); }
    
    template<typename K, typename V>
    std::unique_ptr<bth_node<K,V>> open_bth(heap_id root)
        { return m_pheap->open_bth<K,V>(root); }

private:
    heap& operator=(const heap& other); // = delete
    heap_ptr m_pheap;
};

template<typename K, typename V>
class bth_node : 
    public virtual btree_node<K,V>, 
    private boost::noncopyable
{
public:
    static std::unique_ptr<bth_node<K,V>> open_root(const heap_ptr& h, heap_id bth_root);
    static std::unique_ptr<bth_nonleaf_node<K,V>> open_nonleaf(const heap_ptr& h, heap_id id, ushort level);
    static std::unique_ptr<bth_leaf_node<K,V>> open_leaf(const heap_ptr& h, heap_id id);

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
    heap_id m_id;
    ushort m_level;
};

template<typename K, typename V>
class bth_nonleaf_node : 
    public bth_node<K,V>, 
    public btree_node_nonleaf<K,V>
{
public:
    bth_nonleaf_node(const heap_ptr& h, heap_id id, ushort level, const std::vector<std::pair<K, heap_id>>& bth_info)
        : bth_node<K,V>(h, id, level), m_bth_info(bth_info), m_child_nodes(bth_info.size()) { }
    bth_nonleaf_node(const heap_ptr& h, heap_id id, ushort level, std::vector<std::pair<K, heap_id>>&& bth_info)
        : bth_node<K,V>(h, id, level), m_bth_info(bth_info), m_child_nodes(bth_info.size()) { }

    // btree_node_nonleaf implementation
    const K& get_key(uint pos) const { return m_bth_info[pos].first; }
    bth_node<K,V>* get_child(uint pos);
    const bth_node<K,V>* get_child(uint pos) const;
    uint num_values() const { return m_child_nodes.size(); }

private:
    std::vector<std::pair<K, heap_id>> m_bth_info;
    mutable std::vector<std::shared_ptr<bth_node<K,V>>> m_child_nodes;
};

template<typename K, typename V>
class bth_leaf_node : 
    public bth_node<K,V>, 
    public btree_node_leaf<K,V>
{
public:
    bth_leaf_node(const heap_ptr& h, heap_id id, const std::vector<std::pair<K,V>>& data)
        : bth_node<K,V>(h, id, 0), m_bth_data(data) { }
    bth_leaf_node(const heap_ptr& h, heap_id id, std::vector<std::pair<K,V>>&& data)
        : bth_node<K,V>(h, id, 0), m_bth_data(data) { }
    virtual ~bth_leaf_node() { }

    // btree_node_leaf implementation
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

template<typename K, typename V>
inline std::unique_ptr<fairport::bth_node<K,V>> fairport::bth_node<K,V>::open_root(const heap_ptr& h, heap_id bth_root)
{
    disk::bth_header* pheader;
    std::vector<byte> buffer(sizeof(disk::bth_header));
    pheader = (disk::bth_header*)&buffer[0];

    h->read(buffer, bth_root, 0);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(pheader->bth_signature != disk::heap_sig_bth)
        throw sig_mismatch("bth_signature expected", 0, bth_root, pheader->bth_signature, disk::heap_sig_bth);

    if(pheader->key_size != sizeof(K))
        throw std::logic_error("invalid key size");

    if(pheader->entry_size != sizeof(V))
        throw std::logic_error("invalid entry size");
#endif

    if(pheader->num_levels > 1)
        return open_nonleaf(h, pheader->root, pheader->num_levels-1);
    else
        return open_leaf(h, pheader->root);
}

template<typename K, typename V>
inline std::unique_ptr<fairport::bth_nonleaf_node<K,V>> fairport::bth_node<K,V>::open_nonleaf(const heap_ptr& h, heap_id id, ushort level)
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

    return std::unique_ptr<bth_nonleaf_node<K,V>>(new bth_nonleaf_node<K,V>(h, id, level, std::move(child_nodes)));
}
    
template<typename K, typename V>
inline std::unique_ptr<fairport::bth_leaf_node<K,V>> fairport::bth_node<K,V>::open_leaf(const heap_ptr& h, heap_id id)
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

        return std::unique_ptr<bth_leaf_node<K,V>>(new bth_leaf_node<K,V>(h, id, std::move(entries)));
    }
    else
    {
        // id == 0 means an empty tree
        return std::unique_ptr<bth_leaf_node<K,V>>(new bth_leaf_node<K,V>(h, id, entries));
    }
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

    return m_child_nodes[pos].get();
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

    return m_child_nodes[pos].get();
}

inline fairport::heap_impl::heap_impl(const node& n)
: m_node(n)
{
    // need to throw if the node is smaller than first_header
    disk::heap_first_header first_header = m_node.read<disk::heap_first_header>(0);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(first_header.signature != disk::heap_signature)
        throw sig_mismatch("invalid heap_sig", 0, n.get_id(), first_header.signature, disk::heap_signature);
#endif
}

inline fairport::heap_impl::heap_impl(const node& n, alias_tag)
: m_node(n, alias_tag())
{
    // need to throw if the node is smaller than first_header
    disk::heap_first_header first_header = m_node.read<disk::heap_first_header>(0);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(first_header.signature != disk::heap_signature)
        throw sig_mismatch("invalid heap_sig", 0, n.get_id(), first_header.signature, disk::heap_signature);
#endif
}

inline fairport::heap_impl::heap_impl(const node& n, byte client_sig)
: m_node(n)
{
    // need to throw if the node is smaller than first_header
    disk::heap_first_header first_header = m_node.read<disk::heap_first_header>(0);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(first_header.signature != disk::heap_signature)
        throw sig_mismatch("invalid heap_sig", 0, n.get_id(), first_header.signature, disk::heap_signature);

    if(first_header.client_signature != client_sig)
        throw sig_mismatch("invalid client_sig", 0, n.get_id(), first_header.client_signature, client_sig);
#endif
}

inline fairport::heap_impl::heap_impl(const node& n, byte client_sig, alias_tag)
: m_node(n, alias_tag())
{
    // need to throw if the node is smaller than first_header
    disk::heap_first_header first_header = m_node.read<disk::heap_first_header>(0);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(first_header.signature != disk::heap_signature)
        throw sig_mismatch("invalid heap_sig", 0, n.get_id(), first_header.signature, disk::heap_signature);

    if(first_header.client_signature != client_sig)
        throw sig_mismatch("invalid client_sig", 0, n.get_id(), first_header.client_signature, client_sig);
#endif
}

inline fairport::heap_id fairport::heap_impl::get_root_id() const
{
    disk::heap_first_header first_header = m_node.read<disk::heap_first_header>(0);
    return first_header.root_id;
}

inline fairport::byte fairport::heap_impl::get_client_signature() const
{
    disk::heap_first_header first_header = m_node.read<disk::heap_first_header>(0);
    return first_header.client_signature;
}

inline size_t fairport::heap_impl::size(heap_id id) const
{
    disk::heap_page_header header = m_node.read<disk::heap_page_header>(get_heap_page(id), 0);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(header.page_map_offset > m_node.get_page_size(get_heap_page(id)))
        throw std::length_error("page_map_offset > node size");
#endif

    std::vector<byte> buffer(m_node.get_page_size(get_heap_page(id)) - header.page_map_offset);
    m_node.read(buffer, get_heap_page(id), header.page_map_offset);
    disk::heap_page_map* pmap = reinterpret_cast<disk::heap_page_map*>(&buffer[0]);

#ifdef FAIRPORT_VALIDATION_LEVEL_WEAK
    if(get_heap_index(id) > pmap->num_allocs)
        throw std::length_error("index > num_allocs");
#endif

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

    disk::heap_page_header header = m_node.read<disk::heap_page_header>(get_heap_page(id), 0);
    std::vector<byte> map_buffer(m_node.get_page_size(get_heap_page(id)) - header.page_map_offset);
    m_node.read(map_buffer, get_heap_page(id), header.page_map_offset);
    disk::heap_page_map* pmap = reinterpret_cast<disk::heap_page_map*>(&map_buffer[0]);

    return m_node.read(buffer, get_heap_page(id), pmap->allocs[get_heap_index(id)]);
}

inline fairport::hid_stream_device fairport::heap_impl::open_stream(heap_id id)
{
    return hid_stream_device(shared_from_this(), id);
}

inline std::streamsize fairport::hid_stream_device::read(byte* pbuffer, std::streamsize n)
{
    if(m_hid && (m_pos + n > m_pheap->size(m_hid)))
        n = m_pheap->size(m_hid) - m_pos;

    if(n == 0 || m_hid == 0)
        return -1;

    std::vector<byte> buff(static_cast<uint>(n));
    size_t read = m_pheap->read(buff, m_hid, static_cast<ulong>(m_pos));

    memcpy(pbuffer, &buff[0], read);
    return read;
}

inline std::streampos fairport::hid_stream_device::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
{
    if(way == std::ios_base::beg)
            m_pos = off;
    else if(way == std::ios_base::end)
        m_pos = m_pheap->size(m_hid) + off;
    else
        m_pos += off;

    if(m_pos < 0)
        m_pos = 0;
    else if(m_pos > m_pheap->size(m_hid))
        m_pos = m_pheap->size(m_hid);

    return m_pos;
}

inline std::vector<fairport::byte> fairport::heap_impl::read(heap_id id) const
{
    std::vector<byte> result(size(id));
    read(result, id, 0);
    return result;
}

template<typename K, typename V>
inline std::unique_ptr<fairport::bth_node<K,V>> fairport::heap_impl::open_bth(heap_id root)
{ 
    return bth_node<K,V>::open_root(shared_from_this(), root); 
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
