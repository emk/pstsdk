#ifndef FAIRPORT_UTIL_BTREE_H
#define FAIRPORT_UTIL_BTREE_H

#include <iterator>
#include <vector>
#include <boost/iterator/iterator_facade.hpp>

#include "fairport/util/primatives.h"
#include "fairport/util/errors.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250)
#endif

namespace fairport
{

template<typename K, typename V>
struct btree_iter_impl;

template<typename K, typename V>
class const_btree_node_iter;

template<typename K, typename V>
class btree_node_nonleaf;

// V must be LessThan Comparable
template<typename K, typename V>
class btree_node
{
public:
    typedef const_btree_node_iter<K,V> const_iterator;

    virtual ~btree_node() { }

    virtual const V& lookup(const K&) const = 0;
    
    virtual const K& get_key(uint pos) const = 0;
    virtual uint num_values() const = 0;

    const_iterator begin() const
        { return const_iterator(this, false); }

    const_iterator end() const
        { return const_iterator(this, true); }

    int binary_search(const K&) const;

protected:

    // iter support
    friend class const_btree_node_iter<K,V>;
    friend class btree_node_nonleaf<K,V>;
    virtual void first(btree_iter_impl<K,V>& iter) const = 0;
    virtual void last(btree_iter_impl<K,V>& iter) const = 0;
    virtual void next(btree_iter_impl<K,V>& iter) const = 0;
    virtual void prev(btree_iter_impl<K,V>& iter) const = 0;
};

template<typename K, typename V>
class btree_node_leaf : public virtual btree_node<K,V>
{
public:
    virtual ~btree_node_leaf() { }

    const V& lookup(const K&) const;

    virtual const V& get_value(uint pos) const = 0;

protected:
    // iter support
    friend class const_btree_node_iter<K,V>;
    void first(btree_iter_impl<K,V>& iter) const
        { iter.m_leaf = const_cast<btree_node_leaf<K,V>* >(this); iter.m_leaf_pos = 0; }
    void last(btree_iter_impl<K,V>& iter) const
        { iter.m_leaf = const_cast<btree_node_leaf<K,V>* >(this); iter.m_leaf_pos = this->num_values()-1; }
    void next(btree_iter_impl<K,V>& iter) const;
    void prev(btree_iter_impl<K,V>& iter) const;
};


template<typename K, typename V>
class btree_node_nonleaf : public virtual btree_node<K,V>
{
public:
    virtual ~btree_node_nonleaf() { }

    const V& lookup(const K&) const;

protected:
    // returns a non-owning pointer
    virtual btree_node<K,V>* get_child(uint i) = 0;
    virtual const btree_node<K,V>* get_child(uint i) const = 0;

    // iter support
    friend class const_btree_node_iter<K,V>;
    friend class btree_node_leaf<K,V>;
    void first(btree_iter_impl<K,V>& iter) const;
    void last(btree_iter_impl<K,V>& iter) const;
    void next(btree_iter_impl<K,V>& iter) const;
    void prev(btree_iter_impl<K,V>& iter) const;
};

template<typename K, typename V>
struct btree_iter_impl
{
    btree_node_leaf<K,V>* m_leaf;
    uint m_leaf_pos;

    std::vector<std::pair<btree_node_nonleaf<K,V>*, uint>> m_path;
};

template<typename K, typename V>
class const_btree_node_iter : public boost::iterator_facade<const_btree_node_iter<K,V>, const V, boost::bidirectional_traversal_tag>
{
public:
    const_btree_node_iter();
    const_btree_node_iter(const btree_node<K,V>* root, bool last);

private:
    friend class boost::iterator_core_access;

    void increment() { m_impl.m_leaf->next(m_impl); }
    bool equal(const const_btree_node_iter& other) const 
        { return ((m_impl.m_leaf == other.m_impl.m_leaf) && (m_impl.m_leaf_pos == other.m_impl.m_leaf_pos)); }
    const V& dereference() const
        { return m_impl.m_leaf->get_value(m_impl.m_leaf_pos); }
    void decrement() { m_impl.m_leaf->prev(m_impl); }

    mutable btree_iter_impl<K,V> m_impl;
};

} // end namespace

template<typename K, typename V>
int fairport::btree_node<K,V>::binary_search(const K& k) const
{
    uint end = num_values();
    uint start = 0;
    uint mid = (start + end) / 2;

    while(mid < end)
    {
        if(get_key(mid) < k)
        {
            start = mid + 1;
        }
        else if(get_key(mid) == k)
        {
            return mid; 
        }
        else
        {
            end = mid;
        }

        mid = (start + end) / 2;
    }

    return mid - 1;
}

template<typename K, typename V>
const V& fairport::btree_node_leaf<K,V>::lookup(const K& k) const
{
    int location = this->binary_search(k);

    if(location == -1)
        throw key_not_found<K>(k);
    
    if(this->get_key(location) != k)
        throw key_not_found<K>(k);

    return get_value(location);
}
    
template<typename K, typename V>
void fairport::btree_node_leaf<K,V>::next(fairport::btree_iter_impl<K,V>& iter) const
{
    if(++iter.m_leaf_pos == this->num_values())
    {
        if(iter.m_path.size() > 0)
        {
            for(auto piter = iter.m_path.cbegin();
                piter != iter.m_path.cend(); 
                ++piter)
            {
                if((*piter).second + 1 < (*piter).first->num_values())
                {
                    // we're done with this leaf
                    iter.m_leaf = NULL;
                    
                    iter.m_path.back().first->next(iter);
                    break;
                }
            }
        }
    }
}

template<typename K, typename V>
void fairport::btree_node_leaf<K,V>::prev(fairport::btree_iter_impl<K,V>& iter) const
{
    if(iter.m_leaf_pos == 0)
    {
        if(iter.m_path.size() > 0)
        {
            for(auto piter = iter.m_path.cbegin();
                piter != iter.m_path.cend();
                ++piter)
            {
                // we're done with this leaf
                iter.m_leaf = NULL;
                
                if((*piter).second != 0 && (*piter).first->num_values() > 1)
                {
                    iter.m_path.back().first->prev(iter);
                    break;
                }
            }
        }
    }
    else
    {
        --iter.m_leaf_pos;
    }
}

template<typename K, typename V>
const V& fairport::btree_node_nonleaf<K,V>::lookup(const K& k) const
{
    int location = this->binary_search(k);

    if(location == -1)
        throw key_not_found<K>(k);

    return get_child(location)->lookup(k);
}

template<typename K, typename V>
void fairport::btree_node_nonleaf<K,V>::first(fairport::btree_iter_impl<K,V>& iter) const
{
    iter.m_path.push_back(std::make_pair(const_cast<btree_node_nonleaf<K,V>*>(this), 0));
    get_child(0)->first(iter);
}

template<typename K, typename V>
void fairport::btree_node_nonleaf<K,V>::last(fairport::btree_iter_impl<K,V>& iter) const
{
    iter.m_path.push_back(std::make_pair(const_cast<btree_node_nonleaf<K,V>*>(this), this->num_values()-1));
    get_child(this->num_values()-1)->last(iter);
}

template<typename K, typename V>
void fairport::btree_node_nonleaf<K,V>::next(fairport::btree_iter_impl<K,V>& iter) const
{
    std::pair<btree_node_nonleaf<K,V>*, uint>& me = iter.m_path.back();

    if(++me.second == this->num_values())
    {
        // we're done, pop us off the path and call up
        if(iter.m_path.size() > 1)
        {
            iter.m_path.pop_back();
            iter.m_path.back().first->next(iter);
        }
    }
    else
    {
        // call into the next leaf
        get_child(me.second)->first(iter);
    }
}

template<typename K, typename V>
void fairport::btree_node_nonleaf<K,V>::prev(fairport::btree_iter_impl<K,V>& iter) const
{
    std::pair<btree_node_nonleaf<K,V>*, uint>& me = iter.m_path.back();

    if(me.second == 0)
    {
        // we're done, pop us off the path and call up
        if(iter.m_path.size() > 1)
        {
            iter.m_path.pop_back();
            iter.m_path.back().first->prev(iter);
        }
    }
    else
    {
        // call into the next child
        get_child(--me.second)->last(iter);
    }
}

template<typename K, typename V>
fairport::const_btree_node_iter<K,V>::const_btree_node_iter()
{
    m_impl.m_leaf_pos = 0;
    m_impl.m_leaf = nullptr;
}

template<typename K, typename V>
fairport::const_btree_node_iter<K,V>::const_btree_node_iter(const fairport::btree_node<K,V>* root, bool last)
{
    if(last)
    {
        root->last(m_impl);
        ++m_impl.m_leaf_pos;
    }
    else
    {
        root->first(m_impl);
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
