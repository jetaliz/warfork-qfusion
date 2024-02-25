#pragma once






















#include <Rocket/Controls/DataSource.h>
#include <cstdint>
#include <vector>

namespace WSWUI
{

	typedef struct {
		char steamid[18];
		char name[16];
	} BlockedPlayer;

class BlockedPlayersDataSource :
	public Rocket::Controls::DataSource
{
public:
	BlockedPlayersDataSource( );

	virtual void GetRow( Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns );
	virtual int GetNumRows( const Rocket::Core::String& table );

private:
	std::vector<BlockedPlayer> blockedplayers;
};

}
