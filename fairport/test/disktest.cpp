#include <iostream>
#include <fstream>
#include <cassert>
#include "fairport/disk.h"
#include "fairport/util.h"

template<typename T>
void test_block(fairport::file& file, fairport::disk::block_reference<T>& ref, fairport::ushort size, fairport::byte) 
{
    using namespace fairport;
    using namespace fairport::disk;
    using namespace std;
    size_t aligned_size = align_disk<T>(size);

    std::vector<byte> buffer(aligned_size);
    block_trailer<T>* bt = (block_trailer<T>*)(&buffer[0] + aligned_size - sizeof(block_trailer<T>));

    file.read(buffer, ref.ib);

    assert(bt->cb == size);
    assert(bt->signature == compute_signature(ref));
    assert(bt->crc == compute_crc(&buffer[0], size));
}

template<typename T>
void test_page(fairport::file& file, fairport::disk::block_reference<T> ref, fairport::byte crypt_method)
{
    using namespace fairport;
    using namespace fairport::disk;
    using namespace std;

    std::vector<byte> buffer(page_size);
    page<T>* ppage = (page<T>*)&buffer[0];
    file.read(buffer, ref.ib);
    
    assert(ppage->trailer.crc == compute_crc(ppage, page<T>::page_data_size));
    assert(ppage->trailer.signature == compute_signature(ref));
    assert(ppage->trailer.page_type == ppage->trailer.page_type_repeat);
    
    switch(ppage->trailer.page_type)
    {
        case page_type_bbt:
            if(((bbt_leaf_page<T>*)(ppage))->level != 0)
            {
                bbt_nonleaf_page<T>* nonleaf = (bbt_nonleaf_page<T>*)ppage;
                for(int i = 0; i < nonleaf->num_entries; ++i)
                {
                    test_page<T>(file, nonleaf->entries[i].ref, crypt_method);
                }
            }
            else
            {
                bbt_leaf_page<T>* leaf = (bbt_leaf_page<T>*)ppage;
                for(int i = 0; i < leaf->num_entries; ++i)
                {
                    test_block<T>(file, leaf->entries[i].ref, leaf->entries[i].size, crypt_method);
                }
            }
            break;
        case page_type_nbt:
            if(((nbt_leaf_page<T>*)ppage)->level != 0)
            {
                nbt_nonleaf_page<T>* nonleaf = (nbt_nonleaf_page<T>*)ppage;
                for(int i = 0; i < nonleaf->num_entries; ++i)
                {
                    test_page<T>(file, nonleaf->entries[i].ref, crypt_method);
                }
            }
            break;
    }
}

template<typename T>
void test_disk_structures(fairport::file& file)
{
    using namespace fairport;
    using namespace fairport::disk;
    using namespace std;

    std::vector<byte> buffer(sizeof(header<T>));
    header<T>* pheader = (header<T>*)&buffer[0];

    file.read(buffer, 0); 

    test_page<T>(file, pheader->root.brefNBT, pheader->bCryptMethod);
    test_page<T>(file, pheader->root.brefBBT, pheader->bCryptMethod);
}

void test_disk() 
{
    using namespace std;
    using namespace fairport;
    file uni(L"test_unicode.pst");
    file ansi(L"test_ansi.pst");

    assert(offsetof(disk::header<unsigned long long>, dwCRCFull) == 524);

    test_disk_structures<unsigned long long>(uni);
    test_disk_structures<unsigned long>(ansi);
}
