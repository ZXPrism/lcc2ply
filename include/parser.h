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

	size_t _MaxChunkPosX;
	size_t _MaxChunkPosY;
	std::map<std::pair<size_t, size_t>, std::vector<ChunkInfo>> _ChunkPosToChunkInfoVec;

	size_t _SplatCnt;
	std::vector<SplatPosition> _SplatPositionVec;
	std::vector<SplatColor> _SplatColorVec;
	std::vector<SplatCov> _SplatCovVec;

public:
	Parser(std::string scene_folder);

	[[nodiscard]] size_t get_lod_level_cnt() const;

	bool parse_meta();
	bool parse_index();
	bool parse_fg(size_t lod_level, bool extract_chunk, size_t chunk_pos_x, size_t chunk_pos_y);
	bool parse_bg();
	bool parse_sh();

	void analyze() const;

	void write_ply(const std::string &filename) const;
};

}  // namespace lcc2ply
