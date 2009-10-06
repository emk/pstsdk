#ifndef FAIRPORT_NDB_DATABASE_IFACE_H
#define FAIRPORT_NDB_DATABASE_IFACE_H

#include <memory>

#include "fairport/util/util.h"
#include "fairport/util/primatives.h"

namespace fairport
{

class node;
struct block_info;

template<typename K, typename V>
class bt_page;
typedef bt_page<node_id, node> nbt_page;
typedef bt_page<block_id, block_info> bbt_page;

template<typename K, typename V>
class bt_nonleaf_page;
typedef bt_nonleaf_page<node_id, node> nbt_nonleaf_page;
typedef bt_nonleaf_page<block_id, block_info> bbt_nonleaf_page;

template<typename K, typename V>
class bt_leaf_page;
typedef bt_leaf_page<node_id, node> nbt_leaf_page;
typedef bt_leaf_page<block_id, block_info> bbt_leaf_page;

template<typename K, typename V>
class btree_node_iter;

template<typename K, typename V>
class const_btree_node_iter;

typedef btree_node_iter<node_id, node> node_iterator;
typedef const_btree_node_iter<node_id, node> const_node_iterator;
typedef btree_node_iter<block_id, block_info> block_iterator;
typedef const_btree_node_iter<block_id, block_info> const_block_iterator;

class block;
class data_block;
class extended_block;
class external_block;
class subnode_block;
class subnode_leaf_block;
class subnode_nonleaf_block;

// database external interface
class database
{
public:

    virtual node lookup_node(node_id nid) = 0;
    virtual block_info lookup_block_info(block_id bid) = 0;
   
    // page factory functions
    // the caller owns the objects returned from the following functions
	virtual std::shared_ptr<bbt_page> read_bbt_root() = 0;
	virtual std::shared_ptr<nbt_page> read_nbt_root() = 0;
	virtual std::shared_ptr<bbt_page> read_bbt_page(ulonglong location) = 0;
	virtual std::shared_ptr<nbt_page> read_nbt_page(ulonglong location) = 0;
	virtual std::shared_ptr<nbt_leaf_page> read_nbt_leaf_page(ulonglong location) = 0;
	virtual std::shared_ptr<nbt_leaf_page> read_nbt_leaf_page_for_key(node_id key) = 0;
	virtual std::shared_ptr<bbt_leaf_page> read_bbt_leaf_page(ulonglong location) = 0;
	virtual std::shared_ptr<bbt_leaf_page> read_bbt_leaf_page_for_key(block_id key) = 0;
	virtual std::shared_ptr<nbt_nonleaf_page> read_nbt_nonleaf_page(ulonglong location) = 0;
	virtual std::shared_ptr<bbt_nonleaf_page> read_bbt_nonleaf_page(ulonglong location) = 0;
 
    // block factory functions
    // the caller owns the objects returned from the following functions
	virtual std::shared_ptr<block> read_block(block_id bid) = 0;
	virtual std::shared_ptr<data_block> read_data_block(block_id bid) = 0;
	virtual std::shared_ptr<extended_block> read_extended_block(block_id bid) = 0;
	virtual std::shared_ptr<external_block> read_external_block(block_id bid) = 0;
	virtual std::shared_ptr<subnode_block> read_subnode_block(node_id root_node_id, block_id bid) = 0;
	virtual std::shared_ptr<subnode_leaf_block> read_subnode_leaf_block(node_id root_node_id, block_id bid) = 0;
	virtual std::shared_ptr<subnode_nonleaf_block> read_subnode_nonleaf_block(node_id root_node_id, block_id bid) = 0;

	virtual std::shared_ptr<block> read_block(block_info bi) = 0;
	virtual std::shared_ptr<data_block> read_data_block(block_info bi) = 0;
	virtual std::shared_ptr<extended_block> read_extended_block(block_info bi) = 0;
	virtual std::shared_ptr<external_block> read_external_block(block_info bi) = 0;
	virtual std::shared_ptr<subnode_block> read_subnode_block(node_id root_node_id, block_info bi) = 0;
	virtual std::shared_ptr<subnode_leaf_block> read_subnode_leaf_block(node_id root_node_id, block_info bi) = 0;
	virtual std::shared_ptr<subnode_nonleaf_block> read_subnode_nonleaf_block(node_id root_node_id, block_info bi) = 0;

};

typedef std::shared_ptr<database> shared_db_ptr;

}

#endif

