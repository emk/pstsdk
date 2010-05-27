#include <iostream>
#include <cassert>
#include "pstsdk/disk/disk.h"
#include "pstsdk/ndb.h"

struct node_info
{
    pstsdk::node_id node;
    pstsdk::node_id parent;
};

const node_info node_info_uni[] = {
    { 33, 0 }, { 97, 0 }, { 290, 290 }, { 301, 0 }, { 302, 0 }, { 303, 0 },
    { 481, 0 }, { 513, 0 }, { 609, 0 }, { 641, 0 }, { 673, 0 }, { 801, 0 },
    { 1549, 0 }, { 1550, 0 }, { 1551, 0 }, { 1552, 0 }, { 1579, 0 },
    { 1612, 0 }, { 1649, 0 }, { 1682, 0 }, { 1718, 0 }, { 1751, 0 },
    { 1784, 0 }, { 3073, 0 }, { 3585, 0 }, { 3649, 0 }, { 8739, 290 }, 
    { 8742, 0 }, { 8743, 0 }, { 8752, 0 }, { 32802, 290 }, {32813, 0 },
    { 32814, 0 }, { 32815, 0 }, { 32834, 290 }, { 32845, 0 }, { 32846, 0 },
    { 32847, 0 }, { 32866, 32802 }, { 32877, 0 }, { 32878, 0 }, { 32879, 0 },
    { 32898, 32802 }, { 32909, 0 }, { 32910, 0 }, { 32911, 0 },
    { 2097188, 32802 }, { 2097220, 32898 }
};

const node_info node_info_ansi[] = {
    { 33, 0 }, { 97, 0 }, { 290, 290 }, { 301, 0 }, { 302, 0 }, { 303, 0 },
    { 481, 0 }, { 513, 0 }, { 609, 0 }, { 641, 0 }, { 673, 0 }, { 801, 0 },
    { 1549, 0 }, { 1550, 0 }, { 1551, 0 }, { 1552, 0 }, { 1579, 0 },
    { 1612, 0 }, { 1649, 0 }, { 1682, 0 }, { 1718, 0 }, { 1751, 0 },
    { 1784, 0 }, { 3073, 0 }, { 3585, 0 }, { 3649, 0 }, { 8739, 290 }, 
    { 8742, 0 }, { 8743, 0 }, { 8752, 0 }, { 32802, 290 }, {32813, 0 },
    { 32814, 0 }, { 32815, 0 }, { 32834, 290 }, { 32845, 0 }, { 32846, 0 },
    { 32847, 0 }, { 32866, 32802 }, { 32877, 0 }, { 32878, 0 }, { 32879, 0 },
    { 32898, 32802 }, { 32909, 0 }, { 32910, 0 }, { 32911, 0 },
    { 2097188, 32898 } 
};

struct block_info
{
    pstsdk::block_id block;
    pstsdk::ushort size;
    pstsdk::ushort refs;
};

const block_info block_info_uni[] = {
    { 4, 156, 4 }, { 8, 268, 4 }, { 12, 172, 4 }, { 16, 204, 3 },
    { 20, 164, 2 }, { 24, 100, 2 }, { 36, 92, 2 }, { 40, 124, 2 },
    { 44, 84, 2 }, { 48, 114, 2 }, { 72, 34, 2 }, { 100, 62, 2 },
    { 104, 86, 2 }, { 120, 88, 2 }, { 132, 104, 2 }, { 140, 38, 2 },
    { 156, 140, 2 }, { 176, 274, 2 }, { 188, 252, 2 }, { 192, 228, 2 },
    { 216, 4, 2 }, { 220, 188, 2 }, { 228, 3506, 2 }, { 232, 1312, 2 },
    { 238, 32, 2 }, { 240, 1655, 2 }, { 246, 32, 2 }, { 252, 1248, 2 },
    { 260, 852, 2 }, { 272, 464, 2 }, { 284, 484, 2 }, { 324, 1655, 2 }, 
    { 330, 32, 2 }, { 336, 1248, 2 }, { 344, 852, 2 }, { 352, 118, 2 },
    { 356, 510, 2 }, { 360, 116, 2 }, { 364, 228, 2 }, { 368, 142, 2 },
    { 372, 132, 2 }
};

const block_info block_info_ansi[] = {
    { 4, 156, 4 }, { 8, 268, 5 }, { 12, 172, 4 }, { 16, 204, 3 },
    { 20, 164, 2 }, { 24, 100, 2 }, { 36, 92, 2 }, { 40, 124, 2 },
    { 44, 84, 2 }, { 48, 112, 2 }, { 72, 34, 2 }, { 100, 62, 2 },
    { 104, 76, 2 }, { 120, 88, 2 }, { 132, 84, 2 }, { 140, 38, 2 },
    { 156, 108, 2 }, { 172, 404, 2 }, { 176, 258, 2 }, { 188, 252, 2 },
    { 192, 228, 2 }, { 220, 4, 2 }, { 224, 484, 2 }, { 228, 188, 2 },
    { 268, 2788, 2 }, { 272, 1312, 2 }, { 276, 716, 2 }, { 282, 28, 2 },
    { 284, 1655, 2 }, { 290, 16, 2 }, { 296, 1024, 2 }, { 304, 818, 2 },
    { 312, 104, 2 }, { 316, 480, 2 }, { 320, 102, 2 }, { 324, 228, 2 }, 
    { 328, 104, 2 }, { 332, 132, 2 }
};

void process_node(const pstsdk::node& n)
{
    using namespace std;
    using namespace pstsdk;

    for(const_subnodeinfo_iterator iter = n.subnode_info_begin();
                    iter != n.subnode_info_end();
                    ++iter)
    {
        process_node(node(n, *iter));
    }
    
}

size_t step_size_up(size_t i)
{
    if(i >= 1000000) return 1000000;
    if(i >= 100000) return 100000;
    if(i >= 10000) return 10000;
    return 1000;
}

size_t step_size_down(size_t i)
{
    if(i > 1000000) return 1000000;
    if(i > 100000) return 100000;
    if(i > 10000) return 10000;
    return 1000;
}

template<typename T>
void test_node_impl(pstsdk::node& n, size_t expected)
{
    using namespace pstsdk;

    assert(n.size() == expected);

    if(expected > 0)
    {
        pstsdk::uint expected_page_count = expected / disk::external_block<T>::max_size;
        if(expected % disk::external_block<T>::max_size != 0)
            expected_page_count++;

        pstsdk::uint actual_page_count = n.get_page_count();
        assert(expected_page_count == actual_page_count);

        pstsdk::uint test_value = 0xdeadbeef;
        size_t offset = expected-sizeof(test_value);
        n.write(test_value, offset);

        pstsdk::uint read_test_value = n.read<pstsdk::uint>(offset);

        assert(test_value == read_test_value);
    }
}

template<typename T>
void test_node_resize(pstsdk::node n)
{
    // ramp up
    for(size_t i = 1000; i < 10000000; i += step_size_up(i))
    {
        n.resize(i);
        test_node_impl<T>(n, i);
    }

    // ramp down
    for(size_t i = 10000000; i > 0; i -= step_size_down(i))
    {
        n.resize(i);
        test_node_impl<T>(n, i);
    }
}

template<typename T>
void test_node_stream(pstsdk::node n)
{
    using namespace std;
    using namespace pstsdk;

    vector<byte> contents(n.size());
    byte b;
    int i = 0;
    node_stream stream(n.open_as_stream());
    stream.unsetf(ios::skipws);

    (void)n.read(contents, 0);

    // pick a larger node if this fires. I just want to make sure it's non-trivial.
    assert(n.size() > 100);

    while(stream >> b)
    {
        byte c = contents[i];
        assert(b == c);
        ++i;
    }

    // test seeking from the beginning
    stream.clear();
    stream.seekg(0, ios_base::beg);
    stream.seekg( 10, ios_base::beg );
    assert((int)stream.tellg() == 10);
    stream >> b;
    assert((int)stream.tellg() == 11);
    assert(b == contents[10]);

    // test seeking from current
    stream.seekg( 50, ios_base::cur );
    assert((int)stream.tellg() == 61);
    stream >> b;
    assert(b == contents[61]);

    // test seeking from end
    stream.seekg( -20, ios_base::end );
    assert((int)stream.tellg() == (int)(n.size()-20));
    stream >> b;
    assert(b == contents[ n.size() - 5 ]);
    
}

void test_db()
{
    using namespace std;
    using namespace std::tr1;
    using namespace pstsdk;
    bool caught_invalid_format = false;
    pstsdk::uint node = 0;
    pstsdk::uint block = 0;

    try
    {
        shared_db_ptr db = open_large_pst(L"test_ansi.pst");
    }
    catch(invalid_format&)
    {
        caught_invalid_format = true;
    }
    assert(caught_invalid_format);
    
    caught_invalid_format = false;
    try
    {
        shared_db_ptr db = open_small_pst(L"test_unicode.pst");
    }
    catch(invalid_format&)
    {
        caught_invalid_format = true;
    }
    assert(caught_invalid_format);

    {
    shared_db_ptr db_large = open_large_pst(L"test_unicode.pst");
    shared_db_ptr db_small = open_small_pst(L"test_ansi.pst");
    }
    shared_db_ptr db_2 = open_database(L"test_unicode.pst");
    shared_db_ptr db_3 = open_database(L"test_ansi.pst");

    node = 0;
    std::tr1::shared_ptr<const nbt_page> nbt_root = db_2->read_nbt_root();
    for(const_nodeinfo_iterator iter = nbt_root->begin();
                    iter != nbt_root->end();
                    ++iter, ++node)
    {
        assert(iter->id == node_info_uni[node].node);
        assert(iter->parent_id == node_info_uni[node].parent);
        pstsdk::node n(db_2, *iter);
        process_node(n);
    }
    test_node_resize<ulonglong>(db_2->lookup_node(nid_message_store));
    test_node_stream<ulonglong>(db_2->lookup_node(nid_message_store));

    block = 0;
    std::tr1::shared_ptr<const bbt_page> bbt_root = db_2->read_bbt_root();
    for(const_blockinfo_iterator iter = bbt_root->begin();
                    iter != bbt_root->end();
                    ++iter, ++block)
    {
        assert(iter->id == block_info_uni[block].block);
        assert(iter->size == block_info_uni[block].size);
        assert(iter->ref_count == block_info_uni[block].refs);
    }
  
    node = 0;
    std::tr1::shared_ptr<const nbt_page> nbt_root2 = db_3->read_nbt_root();
    for(const_nodeinfo_iterator iter = nbt_root2->begin();
                    iter != nbt_root2->end();
                    ++iter, ++node)
    {
        assert(iter->id == node_info_ansi[node].node);
        assert(iter->parent_id == node_info_ansi[node].parent);
        pstsdk::node n(db_3, *iter);
        process_node(n);
    }
    test_node_resize<pstsdk::ulong>(db_3->lookup_node(nid_message_store));
    test_node_stream<pstsdk::ulong>(db_2->lookup_node(nid_message_store));

    block = 0;
    std::tr1::shared_ptr<const bbt_page> bbt_root2 = db_3->read_bbt_root();
    for(const_blockinfo_iterator iter = bbt_root2->begin();
                    iter != bbt_root2->end();
                    ++iter, ++block)
    {
        assert(iter->id == block_info_ansi[block].block);
        assert(iter->size == block_info_ansi[block].size);
        assert(iter->ref_count == block_info_ansi[block].refs);
    }
}


