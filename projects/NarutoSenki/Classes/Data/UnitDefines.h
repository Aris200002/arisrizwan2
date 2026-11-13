#pragma once
#include "Enums/HeroEnum.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

constexpr const char *kHeroList[] = {
	HeroEnum::Asuma,
	HeroEnum::Chiyo,
	HeroEnum::Choji,
	HeroEnum::Deidara,
	HeroEnum::Gaara,
	HeroEnum::Hidan,
	HeroEnum::Hinata,
	HeroEnum::Hiruzen,
	HeroEnum::Itachi,
	HeroEnum::Jiraiya,
	HeroEnum::Jugo,
	HeroEnum::Kakashi,
	HeroEnum::Kakuzu,
	HeroEnum::Kankuro,
	HeroEnum::Karin,
	HeroEnum::Kiba,
	HeroEnum::Kimimaro,
	HeroEnum::Kisame,
	HeroEnum::Konan,
	HeroEnum::Lee,
	HeroEnum::Minato,
	HeroEnum::Naruto,
	HeroEnum::Neji,
	HeroEnum::Ino,
	HeroEnum::Orochimaru,
	HeroEnum::Pain,
	HeroEnum::Sai,
	HeroEnum::Sakura,
	HeroEnum::Sasuke,
	HeroEnum::Shikamaru,
	HeroEnum::Shino,
	HeroEnum::Suigetsu,
	HeroEnum::Tenten,
	HeroEnum::Tobi,
	HeroEnum::Tobirama,
	HeroEnum::Tsunade,
};

constexpr size_t kHeroNum =  std::size(kHeroList);

/**
 * Pool karakter yang boleh masuk ke battle (random pick / com / enemy).
 * Isi awal = kHeroList. Bisa ditambah/dikurangi saat runtime (mis. dari Lua).
 * Pointer yang dikembalikan stabil (string di-intern, tidak pernah dihapus).
 */
class HeroPool
{
	static inline std::vector<const char *> &pool()
	{
		static std::vector<const char *> v(std::begin(kHeroList), std::end(kHeroList));
		return v;
	}

	static inline const char *intern(const std::string &name)
	{
		static std::unordered_set<std::string> store;
		return store.insert(name).first->c_str();
	}

public:
	static inline size_t size() { return pool().size(); }
	static inline const char *at(size_t i) { return pool()[i]; }
	static inline const std::vector<const char *> &list() { return pool(); }

	static inline bool has(const std::string &name)
	{
		for (auto n : pool())
			if (name == n)
				return true;
		return false;
	}

	/** @return true bila baru ditambahkan */
	static inline bool add(const std::string &name)
	{
		if (name.empty() || has(name))
			return false;
		pool().push_back(intern(name));
		return true;
	}

	/** @return true bila ada yang dihapus */
	static inline bool remove(const std::string &name)
	{
		auto &v = pool();
		auto it = std::remove_if(v.begin(), v.end(), [&](const char *n)
								 { return name == n; });
		bool removed = it != v.end();
		v.erase(it, v.end());
		return removed;
	}

	/** Kembalikan ke isi awal kHeroList */
	static inline void reset()
	{
		pool().assign(std::begin(kHeroList), std::end(kHeroList));
	}
};
