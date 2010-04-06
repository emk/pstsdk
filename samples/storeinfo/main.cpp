#include <iostream>
#include <vector>

#include "pstsdk/ndb/database.h"

#include "pstsdk/ltp/propbag.h"

using namespace pstsdk;

void wmain(int argc, wchar_t* argv[])
{
    shared_db_ptr db(open_database(argv[1]));
    property_bag store(db->lookup_node(nid_message_store));

    std::vector<prop_id> props = store.get_prop_list();
    for(int i = 0; i < props.size(); ++i)
    {
        printf("0x%X\n", props[i]);
    }
}
