#include "ui_precompiled.h"
#include "kernel/ui_common.h"
#include "datasources/ui_blockedplayers_datasource.h"

#define TABLE_NAME "blockedplayers"

namespace WSWUI
{
    BlockedPlayersDataSource::BlockedPlayersDataSource():Rocket::Controls::DataSource( "blockedplayers" )
	{
		const char *blocked = trap::Cvar_String("cg_chatBlocklist");
		// stored in the format `:name      steamid` (name is padded out to match max)

		for (int i = 0 ;; i += 15+17+1) {
			if ( (i+15+17) >= strlen(blocked)) break;
			BlockedPlayer blockedplayer = {0};
			strncpy(blockedplayer.steamid, &blocked[i+1], 17);
			strncpy(blockedplayer.name, &blocked[i+18], 15);

			printf("|%s|%s|\n",blockedplayer.name,blockedplayer.steamid);

			blockedplayers.push_back(blockedplayer);
		}
	}
	
	void BlockedPlayersDataSource::GetRow( Rocket::Core::StringList &row, const Rocket::Core::String&, int row_index, const Rocket::Core::StringList& cols )
	{
		if( row_index < 0 || (size_t)row_index > blockedplayers.size() )
			return;

		for( Rocket::Core::StringList::const_iterator it = cols.begin(); it != cols.end(); ++it )
		{
			if(*it == "name") row.push_back(blockedplayers[row_index].name);
			else if(*it == "steamid") row.push_back(blockedplayers[row_index].steamid);
			else if(*it == "index") row.push_back(va("%i", row_index));
			else row.push_back("");
		}
	}

	int BlockedPlayersDataSource::GetNumRows(const Rocket::Core::String&)
	{
		return blockedplayers.size();
	}
}
