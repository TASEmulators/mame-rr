#ifndef _LUAENGINE_H
#define _LUAENGINE_H
#include "render.h"

enum LuaCallID
{
	LUACALL_BEFOREEMULATION,
	LUACALL_AFTEREMULATION,
	LUACALL_BEFOREEXIT,
	LUACALL_ONSTART,

	LUACALL_HOTKEY_1,
	LUACALL_HOTKEY_2,
	LUACALL_HOTKEY_3,
	LUACALL_HOTKEY_4,
	LUACALL_HOTKEY_5,

	LUACALL_COUNT
};
void CallRegisteredLuaFunctions(int calltype);

//void MAME_LuaWrite(UINT32 addr);
void MAME_LuaFrameBoundary(running_machine &machine);
int MAME_LoadLuaCode(const char *filename);
void MAME_ReloadLuaCode();
void MAME_LuaStop();
void MAME_OpenLuaConsole();
int MAME_LuaRunning();

int MAME_LuaUsingJoypad();
UINT32 MAME_LuaReadJoypad();
int MAME_LuaSpeed();
//int MAME_LuaFrameskip();
int MAME_LuaRerecordCountSkip();

void MAME_LuaGui();

void MAME_LuaWriteInform();

void MAME_LuaClearGui();
void MAME_LuaEnableGui(UINT8 enabled);

char* MAME_GetLuaScriptName();
struct lua_State* MAME_GetLuaState();

void luasav_save(const char *filename);
void luasav_load(const char *filename);
void lua_init(running_machine &machine);

#endif
