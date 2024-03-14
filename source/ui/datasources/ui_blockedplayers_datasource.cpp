#include "ui_precompiled.h"
#include "kernel/ui_common.h"
#include "datasources/ui_blockedplayers_datasource.h"
#include "kernel/ui_syscalls.h"

#define TABLE_NAME "blockedplayers"

static int blocklistsize() {
	int num = 0;
	for (int i = 0; i < MAX_BLOCKS; i++) {
		if ((*WSWUI::UI_IMPORT.GetBlocklist())[i].steamid != 0) {
			num++;
		}
	}
	return num;
}
namespace WSWUI
{
    BlockedPlayersDataSource::BlockedPlayersDataSource():Rocket::Controls::DataSource( "blockedplayers" )
	{

	}
	
	void BlockedPlayersDataSource::GetRow( Rocket::Core::StringList &row, const Rocket::Core::String&, int row_index, const Rocket::Core::StringList& cols )
	{
		if( row_index < 0 || (size_t)row_index > blocklistsize() )
			return;

		blockentry_t entry = (*UI_IMPORT.GetBlocklist())[row_index];
		for( Rocket::Core::StringList::const_iterator it = cols.begin(); it != cols.end(); ++it )
		{
			if (entry.steamid == 0) continue;

			if(*it == "name") row.push_back(entry.name);
			else if(*it == "steamid") row.push_back(va("%llu", entry.steamid) );
			else if(*it == "index") row.push_back(va("%i", row_index));
			else row.push_back("");
		}
	}

	int BlockedPlayersDataSource::GetNumRows(const Rocket::Core::String&)
	{
		return blocklistsize();
	}
}

