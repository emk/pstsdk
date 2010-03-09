#ifndef FAIRPORT_NDB_PAGE_H
#define FAIRPORT_NDB_PAGE_H

#include <vector>

#include "fairport/util/btree.h"
#include "fairport/util/util.h"

#include "fairport/ndb/database_iface.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250)
#endif

namespace fairport
{

class page
{
public:
    page(const shared_db_ptr& db, page_id pid, ulonglong address)
        : m_db(db), m_pid(pid), m_address(address) { }
    page_id get_page_id() const { return m_pid; }
    ulonglong get_address() const { return m_address; }

protected:
    shared_db_ptr get_db_ptr() const { return shared_db_ptr(m_db); }
    weak_db_ptr m_db;
    page_id m_pid;
    ulonglong m_address;
};

template<typename K, typename V>
class bt_page : 
    public page, 
    public virtual btree_node<K,V>
{
public:
    bt_page(const shared_db_ptr& db, page_id pid, ulonglong address, ushort level)
        : page(db, pid, address), m_level(level) { }

    ushort get_level() const { return m_level; }

private:
    ushort m_level;
};

template<typename K, typename V>
class bt_nonleaf_page : 
    public bt_page<K,V>, 
    public btree_node_nonleaf<K,V>, 
    public std::enable_shared_from_this<bt_nonleaf_page<K,V>>
{
public:
    bt_nonleaf_page(const shared_db_ptr& db, page_id pid, ulonglong address, ushort level, const std::vector<std::pair<K, ulonglong>>& page_info)
        : bt_page<K,V>(db, pid, address, level), m_page_info(page_info), m_child_pages(page_info.size()) { }
    bt_nonleaf_page(const shared_db_ptr& db, page_id pid, ulonglong address, ushort level, std::vector<std::pair<K, ulonglong>>&& page_info)
        : bt_page<K,V>(db, pid, address, level), m_page_info(page_info), m_child_pages(page_info.size()) { }

    // btree_node_nonleaf implementation
    const K& get_key(uint pos) const { return m_page_info[pos].first; }
    bt_page<K,V>* get_child(uint pos);
    const bt_page<K,V>* get_child(uint pos) const;
    uint num_values() const { return m_child_pages.size(); }

private:
    std::vector<std::pair<K, ulonglong>> m_page_info;
    mutable std::vector<std::shared_ptr<bt_page<K,V>>> m_child_pages;
};

template<typename K, typename V>
class bt_leaf_page : 
    public bt_page<K,V>, 
    public btree_node_leaf<K,V>, 
    public std::enable_shared_from_this<bt_leaf_page<K,V>>
{
public:
    bt_leaf_page(const shared_db_ptr& db, page_id pid, ulonglong address, const std::vector<std::pair<K,V>>& data)
        : bt_page<K,V>(db, pid, address, 0), m_page_data(data) { }
    bt_leaf_page(const shared_db_ptr& db, page_id pid, ulonglong address, std::vector<std::pair<K,V>>&& data)
        : bt_page<K,V>(db, pid, address, 0), m_page_data(data) { }

    // btree_node_leaf implementation
    const V& get_value(uint pos) const
        { return m_page_data[pos].second; }
    const K& get_key(uint pos) const
        { return m_page_data[pos].first; }
    uint num_values() const
        { return m_page_data.size(); }

private:
    std::vector<std::pair<K,V>> m_page_data;
};

template<>
inline bt_page<block_id, block_info>* bt_nonleaf_page<block_id, block_info>::get_child(uint pos)
{
    if(m_child_pages[pos] == NULL)
    {
        m_child_pages[pos] = this->get_db_ptr()->read_bbt_page(m_page_info[pos].second);
    }

    return m_child_pages[pos].get();
}

template<>
inline const bt_page<block_id, block_info>* bt_nonleaf_page<block_id, block_info>::get_child(uint pos) const
{
    if(m_child_pages[pos] == NULL)
    {
        m_child_pages[pos] = this->get_db_ptr()->read_bbt_page(m_page_info[pos].second);
    }

    return m_child_pages[pos].get();
}

template<>
inline bt_page<node_id, node_info>* bt_nonleaf_page<node_id, node_info>::get_child(uint pos)
{
    if(m_child_pages[pos] == NULL)
    {
        m_child_pages[pos] = this->get_db_ptr()->read_nbt_page(m_page_info[pos].second); 
    }

    return m_child_pages[pos].get();
}

template<>
inline const bt_page<node_id, node_info>* bt_nonleaf_page<node_id, node_info>::get_child(uint pos) const
{
    if(m_child_pages[pos] == NULL)
    {
        m_child_pages[pos] = this->get_db_ptr()->read_nbt_page(m_page_info[pos].second); 
    }

    return m_child_pages[pos].get();
}

} // end namespace

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
