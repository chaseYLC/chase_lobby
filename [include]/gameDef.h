#pragma once

namespace ChaseGameDef {

	typedef struct tagPlayerType {
		using Ty = int;
		enum {
			NORMAL = 0,
			CREATURE = 1,
			VIRTUAL_CREATURE = 2,
			COMPUTER = 3,
		};
	}PLAYER_TYPE;

}