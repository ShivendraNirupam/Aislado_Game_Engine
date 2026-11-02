// The "game" library. It is a separate DLL / shared object so it can be
// reloaded while the testbed is running (hot reload, added in a later step).
// For now it only proves the engine -> game linking works.

#include <engine/core/logger.h>

#include <testbed/game_export.h>

GAME_API int game_value() {
    ELOG_INFO("game_value() called from the game library.");
    return 1;
}
