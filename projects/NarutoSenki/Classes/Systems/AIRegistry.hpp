#pragma once
#include <string>
#include <unordered_map>

/**
 * Registry "clone AI" yang bisa diisi dari C++ maupun Lua.
 *
 *  - Mon  : setMon("Chibaku") memakai logika/AI milik "JibakuEX"
 *  - Bullet: setBullet("RinSRK") memakai logika/AI milik "TentenSRK"
 *  - Transform: Guy bisa transform menjadi "Guy8Gates" (CharacterBase::setTransform)
 *  - Trap : setTrap("Amaterasu2") memakai logika/AI milik "Amaterasu"
 *  - Hero : Provider::create("Madara") memakai class AI milik "Ino"
 *
 * Nama asli (Chibaku / Madara) tetap dipakai untuk data (XML, sprite, setName),
 * hanya perilaku/AI-nya yang dipinjam dari target.
 */
namespace AIRegistry
{
	using AliasMap = std::unordered_map<std::string, std::string>;

	inline AliasMap &monMap()
	{
		static AliasMap m;
		return m;
	}

	inline AliasMap &bulletMap()
	{
		static AliasMap m;
		return m;
	}

	inline AliasMap &transformMap()
	{
		static AliasMap m;
		return m;
	}

	inline AliasMap &trapMap()
	{
		static AliasMap m;
		return m;
	}

	/** Master (pemilik) mon: IndraSusano -> Indra. Kosong = bebas dipakai siapa pun. */
	inline AliasMap &monOwnerMap()
	{
		static AliasMap m;
		return m;
	}

	/** Mon sambungan: Kage2 -> KageHand2 (mon yang dimunculkan berikutnya). */
	inline AliasMap &monLinkMap()
	{
		static AliasMap m;
		return m;
	}

	inline AliasMap &heroMap()
	{
		static AliasMap m;
		return m;
	}

	/** Ikuti rantai alias (A=B, B=C -> C). Dibatasi supaya aman dari siklus. */
	inline std::string resolve(const AliasMap &map, const std::string &name)
	{
		std::string cur = name;
		for (int depth = 0; depth < 8; ++depth)
		{
			auto it = map.find(cur);
			if (it == map.end() || it->second == cur)
				break;
			cur = it->second;
		}
		return cur;
	}

	inline std::string resolveMon(const std::string &name) { return resolve(monMap(), name); }
	inline std::string resolveBullet(const std::string &name) { return resolve(bulletMap(), name); }
	inline std::string resolveTrap(const std::string &name) { return resolve(trapMap(), name); }
	inline std::string resolveHero(const std::string &name) { return resolve(heroMap(), name); }

	/** Transform: satu langkah saja (Guy -> Guy8Gates). Kosong bila tidak ada. */
	inline std::string nextTransform(const std::string &name)
	{
		auto it = transformMap().find(name);
		return it == transformMap().end() ? std::string() : it->second;
	}
	inline void setTransform(const std::string &name, const std::string &toName) { transformMap()[name] = toName; }

	inline std::string monOwner(const std::string &name)
	{
		auto it = monOwnerMap().find(name);
		return it == monOwnerMap().end() ? std::string() : it->second;
	}
	inline void setMonOwner(const std::string &name, const std::string &owner)
	{
		if (owner.empty())
			monOwnerMap().erase(name);
		else
			monOwnerMap()[name] = owner;
	}

	inline std::string monLink(const std::string &name)
	{
		auto it = monLinkMap().find(name);
		return it == monLinkMap().end() ? std::string() : it->second;
	}
	inline void setMonLink(const std::string &name, const std::string &next)
	{
		if (next.empty())
			monLinkMap().erase(name);
		else
			monLinkMap()[name] = next;
	}

	inline void cloneMon(const std::string &name, const std::string &aiName) { monMap()[name] = aiName; }
	inline void cloneBullet(const std::string &name, const std::string &aiName) { bulletMap()[name] = aiName; }
	inline void cloneTrap(const std::string &name, const std::string &aiName) { trapMap()[name] = aiName; }
	inline void cloneHero(const std::string &name, const std::string &aiName) { heroMap()[name] = aiName; }
} // namespace AIRegistry
