#pragma once
/**
 * Binding Lua (manual, header-only) untuk:
 *
 *  1. CharacterBase::setMon      -> MonAI    (clone AI monster)
 *  2. Provider.hpp               -> HeroAI   (clone AI karakter)
 *  3. UnitDefines.h (HeroPool)   -> Battle   (karakter masuk ke battle)
 *
 * Contoh (Lua):
 *
 *   MonAI.Chibaku  = "JibakuEX"      -- Chibaku pakai AI JibakuEX
 *   HeroAI.Madara  = "Ino"           -- Madara pakai AI Ino
 *   Battle.addChar("Madara")         -- Madara ikut masuk ke battle
 *
 * Panggil `NarutoBindings::registerAll(L)` sekali setelah Lua engine dibuat.
 */

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

#include "Data/UnitDefines.h"
#include "Systems/AIRegistry.hpp"
#include "cocos2d.h"

#include <string>

namespace NarutoBindings
{
	// ------------------------------------------------------------------
	// Proxy alias: MonAI / HeroAI / BulletAI
	//   X.Name = "Target"   -> clone AI
	//   X.Name = nil        -> hapus clone
	//   X.Name              -> baca target (nil jika tidak ada)
	//   X{ Name = "Target" }-> set banyak sekaligus
	//   X.clone / X.remove / X.get / X.clear / X.list
	// ------------------------------------------------------------------
	namespace detail
	{
		inline AIRegistry::AliasMap *aliasUpvalue(lua_State *L)
		{
			return static_cast<AIRegistry::AliasMap *>(lua_touserdata(L, lua_upvalueindex(1)));
		}

		// X.clone(name, aiName)
		inline int alias_clone(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			const char *name = luaL_checkstring(L, 1);
			const char *ai = luaL_checkstring(L, 2);
			(*map)[name] = ai;
			// Argumen ke-3 dan ke-4 (opsional, khusus MonAI): master/pemilik mon, mon sambungan
			if (map == &AIRegistry::monMap())
			{
				AIRegistry::setMonOwner(name, lua_isstring(L, 3) ? lua_tostring(L, 3) : "");
				AIRegistry::setMonLink(name, lua_isstring(L, 4) ? lua_tostring(L, 4) : "");
			}
			lua_pushboolean(L, 1);
			return 1;
		}

		// X.remove(name)
		inline int alias_remove(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			const char *name = luaL_checkstring(L, 1);
			if (map == &AIRegistry::monMap())
			{
				AIRegistry::setMonOwner(name, "");
				AIRegistry::setMonLink(name, "");
			}
			lua_pushboolean(L, map->erase(name) > 0);
			return 1;
		}

		// X.get(name) -> aiName | nil
		inline int alias_get(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			auto it = map->find(luaL_checkstring(L, 1));
			if (it == map->end())
				lua_pushnil(L);
			else
				lua_pushstring(L, it->second.c_str());
			return 1;
		}

		// X.clear()
		inline int alias_clear(lua_State *L)
		{
			aliasUpvalue(L)->clear();
			return 0;
		}

		// X.list() -> { Name = "Target", ... }
		inline int alias_list(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			lua_newtable(L);
			for (auto &kv : *map)
			{
				lua_pushstring(L, kv.second.c_str());
				lua_setfield(L, -2, kv.first.c_str());
			}
			return 1;
		}

		// metatable.__index(tbl, key)  (hanya dipanggil bila key bukan method)
		inline int alias_index(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			if (lua_type(L, 2) != LUA_TSTRING)
			{
				lua_pushnil(L);
				return 1;
			}
			auto it = map->find(lua_tostring(L, 2));
			if (it == map->end())
				lua_pushnil(L);
			else
				lua_pushstring(L, it->second.c_str());
			return 1;
		}

		// metatable.__newindex(tbl, key, value)
		inline int alias_newindex(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			const char *name = luaL_checkstring(L, 2);
			if (lua_isnil(L, 3))
			{
				map->erase(name);
			}
			else
			{
				const char *ai = luaL_checkstring(L, 3);
				(*map)[name] = ai;
			}
			return 0;
		}

		// metatable.__call(tbl, { Name = "Target", ... })
		inline int alias_call(lua_State *L)
		{
			auto *map = aliasUpvalue(L);
			luaL_checktype(L, 2, LUA_TTABLE);
			lua_pushnil(L);
			while (lua_next(L, 2) != 0)
			{
				// key = -2, value = -1
				if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING)
					(*map)[lua_tostring(L, -2)] = lua_tostring(L, -1);
				lua_pop(L, 1);
			}
			return 0;
		}

		inline void pushWithMap(lua_State *L, lua_CFunction fn, AIRegistry::AliasMap *map)
		{
			lua_pushlightuserdata(L, map);
			lua_pushcclosure(L, fn, 1);
		}

		inline void registerAliasTable(lua_State *L, const char *globalName, AIRegistry::AliasMap *map)
		{
			lua_newtable(L); // proxy

			pushWithMap(L, alias_clone, map);
			lua_setfield(L, -2, "clone");
			pushWithMap(L, alias_remove, map);
			lua_setfield(L, -2, "remove");
			pushWithMap(L, alias_get, map);
			lua_setfield(L, -2, "get");
			pushWithMap(L, alias_clear, map);
			lua_setfield(L, -2, "clear");
			pushWithMap(L, alias_list, map);
			lua_setfield(L, -2, "list");

			lua_newtable(L); // metatable
			pushWithMap(L, alias_index, map);
			lua_setfield(L, -2, "__index");
			pushWithMap(L, alias_newindex, map);
			lua_setfield(L, -2, "__newindex");
			pushWithMap(L, alias_call, map);
			lua_setfield(L, -2, "__call");
			lua_setmetatable(L, -2);

			lua_setglobal(L, globalName);
		}

		// ------------------------------------------------------------------
		// Battle
		// ------------------------------------------------------------------

		/** Cek resource karakter (sama dengan urutan pencarian Hero::setID). */
		inline bool heroResourceExists(const std::string &name)
		{
			auto fu = cocos2d::FileUtils::sharedFileUtils();
			static const char *kDirs[] = {"Ninja", "Kuchiyose", "Kugutsu", "Guardian"};
			for (auto dir : kDirs)
			{
				auto path = std::string("Unit/") + dir + "/" + name + "/" + name + ".xml";
				if (fu->isFileExist(fu->fullPathForFilename(path.c_str())))
					return true;
			}
			return false;
		}

		inline bool addBattleChar(const std::string &name)
		{
			if (!heroResourceExists(name))
			{
				CCLOG("[Lua] Battle.addChar('%s') ditolak: resource Unit/*/%s/%s.xml tidak ditemukan",
					  name.c_str(), name.c_str(), name.c_str());
				return false;
			}
			return HeroPool::add(name);
		}

		// Battle.addChar(name) -> true bila berhasil ditambahkan
		inline int battle_addChar(lua_State *L)
		{
			lua_pushboolean(L, addBattleChar(luaL_checkstring(L, 1)));
			return 1;
		}

		// Battle.addChars({ "A", "B" }) -> jumlah yang berhasil ditambahkan
		inline int battle_addChars(lua_State *L)
		{
			luaL_checktype(L, 1, LUA_TTABLE);
			int added = 0;
			int n = (int)lua_objlen(L, 1);
			for (int i = 1; i <= n; ++i)
			{
				lua_rawgeti(L, 1, i);
				if (lua_type(L, -1) == LUA_TSTRING && addBattleChar(lua_tostring(L, -1)))
					++added;
				lua_pop(L, 1);
			}
			lua_pushinteger(L, added);
			return 1;
		}

		// Battle.removeChar(name)
		inline int battle_removeChar(lua_State *L)
		{
			lua_pushboolean(L, HeroPool::remove(luaL_checkstring(L, 1)));
			return 1;
		}

		// Battle.hasChar(name)
		inline int battle_hasChar(lua_State *L)
		{
			lua_pushboolean(L, HeroPool::has(luaL_checkstring(L, 1)));
			return 1;
		}

		// Battle.chars() -> { "Asuma", ... }
		inline int battle_chars(lua_State *L)
		{
			lua_newtable(L);
			const auto &list = HeroPool::list();
			for (size_t i = 0; i < list.size(); ++i)
			{
				lua_pushstring(L, list[i]);
				lua_rawseti(L, -2, (int)i + 1);
			}
			return 1;
		}

		// Battle.resetChars()
		inline int battle_resetChars(lua_State *)
		{
			HeroPool::reset();
			return 0;
		}
	} // namespace detail

	inline void registerAll(lua_State *L)
	{
		detail::registerAliasTable(L, "MonAI", &AIRegistry::monMap());
		detail::registerAliasTable(L, "HeroAI", &AIRegistry::heroMap());
		detail::registerAliasTable(L, "BulletAI", &AIRegistry::bulletMap());
		detail::registerAliasTable(L, "Transform", &AIRegistry::transformMap());
		detail::registerAliasTable(L, "TrapAI", &AIRegistry::trapMap());

		static const luaL_Reg battleFuncs[] = {
			{"addChar", detail::battle_addChar},
			{"addChars", detail::battle_addChars},
			{"removeChar", detail::battle_removeChar},
			{"hasChar", detail::battle_hasChar},
			{"chars", detail::battle_chars},
			{"resetChars", detail::battle_resetChars},
			{nullptr, nullptr}};
		lua_newtable(L);
		luaL_register(L, nullptr, battleFuncs);
		lua_setglobal(L, "Battle");
	}
} // namespace NarutoBindings
