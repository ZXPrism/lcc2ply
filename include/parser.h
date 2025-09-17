#pragma once

#include <chunk_info.h>
#include <splat_decode.h>
#include <splat_info.h>
#include <util.h>

#include <json/json.h>

#include <string>
#include <vector>

namespace lcc2ply {

class Parser {
private:
	const fs::path _SceneFolder;

	Json::Value _MetaRoot;
	std::vector<size_t> _SplatCntPerLodLevel;
	size_t _LodLevelCnt;

public:
	Parser(std::string scene_folder);

	[[nodiscard]] size_t get_lod_level_cnt() const;

	bool parse_meta();
	bool parse_index();
	bool parse_fg();
	bool parse_bg();
	bool parse_sh();

	void write_ply() const;
};

}  // namespace lcc2ply
