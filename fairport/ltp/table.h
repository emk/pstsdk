#ifndef FAIRPORT_LTP_TABLE_H
#define FAIRPORT_LTP_TABLE_H

#include <vector>
#include <unordered_map>

#include "fairport/util/primatives.h"

#include "fairport/ndb/node.h"

#include "fairport/ltp/object.h"
#include "fairport/ltp/heap.h"

namespace fairport
{

class table_impl;
typedef std::shared_ptr<table_impl> table_ptr;
typedef std::shared_ptr<const table_impl> const_table_ptr;

table_ptr open_table(const node& n);


class const_table_row : public const_property_object
{
public:
    const_table_row(const const_table_row& other)
        : m_position(other.m_position), m_table(other.m_table) { }
    const_table_row(ulong position, const const_table_ptr& table)
        : m_position(position), m_table(table) { }

    row_id row_id() const;

    // const_property_object
    std::vector<prop_id> get_prop_list() const;
    prop_type get_prop_type(prop_id id) const;
    bool prop_exists(prop_id id) const;

private:
    // const_property_object
    byte get_value_1(prop_id id) const;
    ushort get_value_2(prop_id id) const;
    ulong get_value_4(prop_id id) const;
    ulonglong get_value_8(prop_id id) const;
    std::vector<byte> get_value_variable(prop_id id) const;

    ulong m_position;
    const_table_ptr m_table;
};

class const_table_row_iter : public std::iterator<std::random_access_iterator_tag, const_table_row>
{
public:
    const_table_row_iter(ulong pos, const const_table_ptr& table) 
        : m_position(pos), m_table(table)  { }
    const_table_row_iter& operator++()
        { ++m_position; return (*this); }
    const_table_row_iter& operator+=(ulong off)
        { m_position += off; return (*this); }
    const_table_row_iter operator++(int)
        { const_table_row_iter iter = *this; ++*this; return iter; }
    const_table_row_iter operator+(ulong off)
        { const_table_row_iter iter = *this; iter += off; return iter; }
    const_table_row_iter& operator--()
        { --m_position; return (*this); }
    const_table_row_iter& operator-=(ulong off)
        { m_position -= off; return (*this); }
    const_table_row_iter operator--(int)
        { const_table_row_iter iter = *this; --*this; return iter; }
    const_table_row_iter operator-(ulong off)
        { const_table_row_iter iter = *this; iter -= off; return iter; }
    bool operator!=(const const_table_row_iter& other)
        { return !(*this == other); }
    bool operator==(const const_table_row_iter& other)
        { return ((m_position == other.m_position) && (m_table == other.m_table)); }
    const_table_row operator[](ulong off)
        { return (*(*this + off)); }
    const_table_row operator*()
        { return const_table_row(m_position, m_table); }
private:
    ulong m_position;
    const_table_ptr m_table;
};

class table_impl : public std::enable_shared_from_this<table_impl>
{
public:
    virtual ~table_impl() { }
    virtual ulong lookup_row(row_id id) const = 0;

    const_table_row operator[](ulong row) const
        { return const_table_row(row, shared_from_this()); }
    const_table_row_iter begin() const
        { return const_table_row_iter(0, shared_from_this()); }
    const_table_row_iter end() const
        { return const_table_row_iter(size(), shared_from_this()); }
    
    virtual node& get_node() = 0;
    virtual const node& get_node() const = 0;
    virtual ulonglong get_cell_value(ulong row, prop_id id) const = 0;
    virtual std::vector<byte> read_cell(ulong row, prop_id id) const = 0;
    virtual std::vector<prop_id> get_prop_list() const = 0;
    virtual prop_type get_prop_type(prop_id id) const = 0;
    virtual row_id get_row_id(ulong row) const = 0;
    virtual size_t size() const = 0;
    virtual bool prop_exists(ulong row, prop_id id) const = 0;
};

// implementation of an ANSI TC (64k rows) and a unicode TC
template<typename T>
class basic_table : public table_impl
{
public:
    node& get_node() 
        { return m_prows->get_node(); }
    const node& get_node() const
        { return m_prows->get_node(); }
    ulong lookup_row(row_id id) const
        { return (ulong)m_prows->lookup(id); }
    ulonglong get_cell_value(ulong row, prop_id id) const;
    std::vector<byte> read_cell(ulong row, prop_id id) const;
    std::vector<prop_id> get_prop_list() const;
    prop_type get_prop_type(prop_id id) const;
    row_id get_row_id(ulong row) const;
    size_t size() const;
    bool prop_exists(ulong row, prop_id id) const;

private:
    friend table_ptr open_table(const node& n);
    basic_table(const node& n);

	std::unique_ptr<bth_node<row_id, T>> m_prows;

    // only one of the following two items is valid
    std::vector<byte> m_vec_rowarray;
    std::unique_ptr<node> m_pnode_rowarray;

    std::unordered_map<prop_id, disk::column_description> m_columns; 
    typedef std::unordered_map<prop_id, disk::column_description>::iterator column_iter;
    typedef std::unordered_map<prop_id, disk::column_description>::const_iterator const_column_iter;

    ushort m_offsets[disk::tc_offsets_max];

    // helper functions
    ulong cb_per_row() const { return m_offsets[disk::tc_offsets_bitmap]; }
    ulong rows_per_page() const { return (m_pnode_rowarray ? m_pnode_rowarray->get_page_size(1) / cb_per_row() : m_vec_rowarray.size() / cb_per_row()); }
    const byte* get_raw_row(ulong row) const;
};

typedef basic_table<ushort> small_table;
typedef basic_table<ulong> large_table;

// actual table object that clients instiantiate
class table
{
public:
    explicit table(const node& n);
    table(const table& other);

    const_table_row operator[](ulong row) const
        { return (*m_ptable)[row]; }
    const_table_row_iter begin() const
        { return m_ptable->begin(); }
    const_table_row_iter end() const
        { return m_ptable->end(); }
    
    node& get_node() 
        { return m_ptable->get_node(); }
    const node& get_node() const
        { return m_ptable->get_node(); }
    ulonglong get_cell_value(ulong row, prop_id id) const
        { return m_ptable->get_cell_value(row, id); }
    std::vector<byte> read_cell(ulong row, prop_id id) const
        { return m_ptable->read_cell(row, id); }
    std::vector<prop_id> get_prop_list() const
        { return m_ptable->get_prop_list(); }
    prop_type get_prop_type(prop_id id) const
        { return m_ptable->get_prop_type(id); }
    row_id get_row_id(ulong row) const
        { return m_ptable->get_row_id(row); }
    size_t size() const
        { return m_ptable->size(); }
private:
    table();

    table_ptr m_ptable;
};

} // end fairport namespace

inline fairport::table_ptr fairport::open_table(const node& n)
{
    if(n.get_id() == nid_all_message_search_contents)
    {
        //return table_ptr(new gust(n));
        throw not_implemented("gust table");
    }

    heap h(n);
    std::vector<byte> table_info = h.read(h.get_root_id());
    disk::tc_header* pheader = (disk::tc_header*)&table_info[0];

    std::vector<byte> bth_info = h.read(pheader->row_btree_id);
    disk::bth_header* pbthheader = (disk::bth_header*)&bth_info[0];

    if(pbthheader->entry_size == 4)
       return table_ptr(new large_table(n));
    else
       return table_ptr(new small_table(n));
}

inline std::vector<fairport::prop_id> fairport::const_table_row::get_prop_list() const
{ 
    return m_table->get_prop_list(); 
}

inline fairport::prop_type fairport::const_table_row::get_prop_type(prop_id id) const
{ 
    return m_table->get_prop_type(id); 
}

inline bool fairport::const_table_row::prop_exists(prop_id id) const
{
    return m_table->prop_exists(m_position, id);
}
    
inline fairport::row_id fairport::const_table_row::row_id() const
{ 
    return m_table->get_row_id(m_position); 
}

inline fairport::byte fairport::const_table_row::get_value_1(prop_id id) const
{
    return (byte)m_table->get_cell_value(m_position, id); 
}

inline fairport::ushort fairport::const_table_row::get_value_2(prop_id id) const
{
    return (ushort)m_table->get_cell_value(m_position, id); 
}

inline fairport::ulong fairport::const_table_row::get_value_4(prop_id id) const
{
    return (ulong)m_table->get_cell_value(m_position, id); 
}

inline fairport::ulonglong fairport::const_table_row::get_value_8(prop_id id) const
{
    return m_table->get_cell_value(m_position, id); 
}

inline std::vector<fairport::byte> fairport::const_table_row::get_value_variable(prop_id id) const
{ 
    return m_table->read_cell(m_position, id); 
}

template<typename T>
inline fairport::basic_table<T>::basic_table(const node& n)
{
    heap h(n);

    std::vector<byte> table_info = h.read(h.get_root_id());
    disk::tc_header* pheader = (disk::tc_header*)&table_info[0];

    if(pheader->signature != disk::heap_sig_tc)
        throw sig_mismatch("heap_sig_tc expected");

    m_prows = h.open_bth<row_id, T>(pheader->row_btree_id);

    for(int i = 0; i < pheader->num_columns; ++i)
        m_columns[pheader->columns[i].id] = pheader->columns[i];

    for(int i = 0; i < disk::tc_offsets_max; ++i)
        m_offsets[i] = pheader->size_offsets[i];

    if(is_subnode_id(pheader->row_matrix_id))
    {
        m_pnode_rowarray.reset(new node(n.lookup(pheader->row_matrix_id)));
    }
    else if(pheader->row_matrix_id)
    {
        m_vec_rowarray = h.read(pheader->row_matrix_id);
    }
}

template<typename T>
inline size_t fairport::basic_table<T>::size() const
{
    if(m_pnode_rowarray)
    {
        return m_pnode_rowarray->get_page_count()-1 * rows_per_page() + m_pnode_rowarray->get_page_size(m_pnode_rowarray->get_page_count()-1) / cb_per_row();
    }
    else 
    {
        return m_vec_rowarray.size() / cb_per_row();
    }
}

template<typename T>
inline std::vector<fairport::prop_id> fairport::basic_table<T>::get_prop_list() const
{
    std::vector<prop_id> props;

    for(const_column_iter i = m_columns.begin(); i != m_columns.end(); ++i)
        props.push_back(i->first);

    return props;
}
    
template<typename T>
inline fairport::ulonglong fairport::basic_table<T>::get_cell_value(ulong row, prop_id id) const
{
    if(!prop_exists(row, id))
        throw key_not_found<prop_id>(id);

    const_column_iter column = m_columns.find(id);
    const byte* prow = get_raw_row(row);

    ulonglong value;

    switch(column->second.size)
    {
        case 8:
            {
            value = prow[column->second.offset];
            break;
            }
        case 4:
            {
            ulong cell_value = prow[column->second.offset];
            value = (ulonglong)cell_value;
            break;
            }
        case 2:
            {
            ushort cell_value = prow[column->second.offset];
            value = (ulonglong)cell_value;
            break;
            }
        case 1:
            {
            byte cell_value = prow[column->second.offset];
            value = (ulonglong)cell_value;
            break;
            }
        default:
            throw database_corrupt("get_cell_value: invalid cell size");
    }

    return value;
}
    
template<typename T>
inline std::vector<fairport::byte> fairport::basic_table<T>::read_cell(ulong row, prop_id id) const
{
    heapnode_id hid = (heapnode_id)get_cell_value(row, id);
    std::vector<byte> buffer;

    if(is_subnode_id(hid))
    {
        node sub(get_node().lookup(hid));
        buffer.resize(sub.size());
        sub.read(buffer, 0);
    }
    else
    {
        buffer = m_prows->get_heap_ptr()->read(hid);
    }
    return buffer;
}

template<typename T>
inline fairport::prop_type fairport::basic_table<T>::get_prop_type(prop_id id) const
{
    const_column_iter iter = m_columns.find(id);

    if(iter == m_columns.end())
        throw key_not_found<prop_id>(id);

    return (prop_type)iter->second.type;
}

template<typename T>
inline fairport::row_id fairport::basic_table<T>::get_row_id(ulong row) const
{
    const byte* prow = get_raw_row(row);
    return *(row_id*)prow;
}

template<typename T>
inline const fairport::byte* fairport::basic_table<T>::get_raw_row(ulong row) const
{
    if(row >= size())
        throw std::out_of_range("row >= size()");

    if(m_pnode_rowarray)
    {
        ulong page_num = row / rows_per_page();
        ulong page_offset = (row % rows_per_page()) * cb_per_row();

        return m_pnode_rowarray->get_ptr(page_num, page_offset);
    }
    else
    {
        return &m_vec_rowarray[ row * cb_per_row() ];
    }
}

template<typename T>
inline bool fairport::basic_table<T>::prop_exists(ulong row, prop_id id) const
{
    const_column_iter column = m_columns.find(id);

    if(column == m_columns.end())
        return false;

    const byte* pexists_map = get_raw_row(row) + m_offsets[disk::tc_offsets_one];

    return test_bit(pexists_map, column->second.bit_offset);
}

inline fairport::table::table(const node& n)
{
    m_ptable = open_table(n);
}

inline fairport::table::table(const table& other)
{
    m_ptable = open_table(other.m_ptable->get_node());
}

#endif
