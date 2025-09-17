#include <parser.h>
#include <ply_writer.h>

#include <cmath>
#include <fstream>
#include <iostream>

namespace lcc2ply {

Parser::Parser(std::string scene_folder)
    : _SceneFolder(std::move(scene_folder)) {
}

size_t Parser::get_lod_level_cnt() const {
	return _LodLevelCnt;
}

bool Parser::parse_meta() {
	const fs::path meta_file_path = _SceneFolder / "meta.lcc";

	Json::CharReaderBuilder builder;
	std::string meta_errs;
	std::fstream meta_fin(meta_file_path);

	if (!Json::parseFromStream(builder, meta_fin, &_MetaRoot, &meta_errs)) {
		_println("lcc2ply: error: unable to read meta file from {}", meta_file_path.string());
		_println("lcc2ply: {}", meta_errs);
		return false;
	}

	// parse lod distribution
	const auto &lod_distribution_node = _MetaRoot["splats"];
	_LodLevelCnt = lod_distribution_node.size();
	for (const auto &splat_cnt_per_lod_node : lod_distribution_node) {
		const int splat_cnt_per_lod = splat_cnt_per_lod_node.asInt();
		_SplatCntPerLodLevel.push_back(splat_cnt_per_lod);
	}

	_println("lcc2ply: this scene contains {} lod levels", _LodLevelCnt);
	std::cout << "distribution: [";
	for (size_t i = 0; i < _LodLevelCnt; i++) {
		std::cout << _SplatCntPerLodLevel[i] << " ]"[i == _LodLevelCnt - 1];
	}
	std::cout << '\n';

	return true;
}

bool Parser::parse_index() {
	const fs::path scene_index_file_path = _SceneFolder / "index.bin";

	const std::vector<char> index_data = read_binary(scene_index_file_path);

	const size_t index_data_size_bytes = index_data.size();
	const auto *index_data_base_addr = index_data.data();
	size_t offset = 0;

	// consistency check: sum of previous sizes should equal to current offset
	size_t presum_size_bytes = 0;

	// consistency check: number of splats in each level should correspond to the data in meta.lcc
	std::vector<size_t> splat_cnt_per_lod_level(_SplatCntPerLodLevel.size());

	while (offset < index_data_size_bytes) {
		const auto chunk_pos = reinterpret_data<ChunkPos>(index_data_base_addr + offset);
		offset += sizeof(ChunkPos);

		if (DEBUG_OUTPUT) {
			_println("chunk at ({}, {})", chunk_pos.x, chunk_pos.y);
		}

		auto &chunk_info_vec = _ChunkPosToChunkInfoVec[{ chunk_pos.x, chunk_pos.y }];
		for (size_t i = 0; i < _LodLevelCnt; i++) {
			const auto chunk_info = reinterpret_data<ChunkInfo>(index_data_base_addr + offset);
			offset += sizeof(ChunkInfo);

			if (DEBUG_OUTPUT) {
				std::println(std::cout,
				             "[{}] splat_cnt: {}, offset_bytes = {}, n_bytes = {}",
				             i,
				             chunk_info.splat_cnt,
				             chunk_info.offset_bytes,
				             chunk_info.n_bytes);
			}

			splat_cnt_per_lod_level[i] += chunk_info.splat_cnt;

			if (chunk_info.splat_cnt != 0) {
				if (chunk_info.n_bytes % chunk_info.splat_cnt != 0 || chunk_info.n_bytes / chunk_info.splat_cnt != 32) {
					_println("lcc2ply: warning: consistency check failed (splat cnt & size mismatch), index.bin may be corrupted");
				}
				if (presum_size_bytes != chunk_info.offset_bytes) {
					_println("lcc2ply: warning: consistency check failed (prefix sum mismatch), index.bin may be corrupted");
				}
			}
			presum_size_bytes += chunk_info.n_bytes;

			chunk_info_vec.push_back(chunk_info);
		}

		if (DEBUG_OUTPUT) {
			std::cout << '\n';
		}
	}

	if (splat_cnt_per_lod_level != _SplatCntPerLodLevel) {
		_println("lcc2ply: warning: consistency check failed (splat cnt mismatch), index.bin may be corrupted");

		std::cout << "current distribution: [";
		for (size_t i = 0; i < _LodLevelCnt; i++) {
			std::cout << splat_cnt_per_lod_level[i] << " ]"[i == _LodLevelCnt - 1];
		}
		std::cout << '\n';
	}

	return true;
}

bool Parser::parse_fg(size_t lod_level) {
	if (lod_level >= _LodLevelCnt) {
		_println("lcc2ply: error: the requested lod_level {} is invalid, the scene only has {} lod levels", lod_level, _LodLevelCnt);
		return false;
	}

	std::array<f32, 3> scale_lb;
	std::array<f32, 3> scale_ub;

	const auto &attr_root_node = _MetaRoot["attributes"];
	for (const auto &attr_node : attr_root_node) {
		const std::string attr_name = attr_node["name"].asString();
		if (attr_name == "scale") {
			const auto &lower_bound_node = attr_node["min"];
			const auto &upper_bound_node = attr_node["max"];
			for (size_t i = 0; const auto &scale_lb_node : lower_bound_node) {
				scale_lb[i] = scale_lb_node.asFloat();
				++i;
			}
			for (size_t i = 0; const auto &scale_ub_node : upper_bound_node) {
				scale_ub[i] = scale_ub_node.asFloat();
				++i;
			}

			break;
		}
	}
	const Range<f32> scale_range_x(scale_lb[0], scale_ub[0]);
	const Range<f32> scale_range_y(scale_lb[1], scale_ub[1]);
	const Range<f32> scale_range_z(scale_lb[2], scale_ub[2]);
	if (DEBUG_OUTPUT) {
		_println("scale_range_x: {}", scale_range_x.string());
		_println("scale_range_y: {}", scale_range_y.string());
		_println("scale_range_z: {}", scale_range_z.string());
	}

	const fs::path scene_fg_data_file_path = _SceneFolder / "data.bin";

	const std::vector<char> fg_data = read_binary(scene_fg_data_file_path);
	const size_t data_size_bytes = fg_data.size();
	if (data_size_bytes % 32 != 0) {
		_println("lcc2ply: error: the file size is not divisible by 32");
		return false;
	}

	const auto *data_base_addr = fg_data.data();
	size_t splat_id = 0;

	for (const auto &[chunk_pos, all_lod_level_splat_vec] : _ChunkPosToChunkInfoVec) {
		const auto &chunk_info = all_lod_level_splat_vec[lod_level];
		size_t offset = chunk_info.offset_bytes;
		const size_t max_offset = offset + chunk_info.n_bytes;

		while (offset < max_offset) {
			const auto packed_splat_data = reinterpret_data<PackedSplatData>(data_base_addr + offset);
			const auto packed_splat_color = reinterpret_data<PackedSplatColor>(&packed_splat_data.color);
			const auto packed_splat_cov = reinterpret_data<PackedSplatCov>(&packed_splat_data.cov_1);

			if (DEBUG_OUTPUT) {
				_println("### SPLAT {} ###", splat_id);
				_print("packed_position = ({}, {}, {}) ", packed_splat_data.x, packed_splat_data.y, packed_splat_data.z);
				_print("packed_color = {} ", packed_splat_data.color);
				_print("packed_cov = ({}, {}, {}) ", packed_splat_data.cov_1, packed_splat_data.cov_2, packed_splat_data.cov_3);
				_println("unused = {}", packed_splat_data.unused);
			}

			const auto position = decode_splat_position(packed_splat_data);
			const auto color = decode_splat_color(packed_splat_color);
			const auto cov = decode_splat_cov(packed_splat_cov, scale_range_x, scale_range_y, scale_range_z);

			if (DEBUG_OUTPUT) {
				_println("position: ({}, {}, {})", position.x, position.y, position.z);
				_println("color: ({}, {}, {})", color.r, color.g, color.b);
				_println("scale: ({}, {}, {})", cov.s_x, cov.s_y, cov.s_z);
				_println("rotation: ({}, {}, {}, {})", cov.quat_x, cov.quat_y, cov.quat_z, cov.quat_w);
				_println("opacity: {}", color.a);
			}

			_SplatPositionVec.push_back(position);
			_SplatColorVec.push_back(color);
			_SplatCovVec.push_back(cov);

			offset += 32;
			++splat_id;

			if (DEBUG_OUTPUT) {
				std::cout << '\n';
			}
		}
	}

	_SplatCnt = _SplatPositionVec.size();

	return true;
}

bool Parser::parse_bg() {
	return true;
}

bool Parser::parse_sh() {
	return true;
}

void Parser::write_ply(const std::string &filename) const {
	const std::vector<std::string> column_name_vec{
		"x",
		"y",
		"z",
		"f_dc_0",
		"f_dc_1",
		"f_dc_2",
		"opacity",
		"scale_0",
		"scale_1",
		"scale_2",
		"rot_0",
		"rot_1",
		"rot_2",
		"rot_3"
	};

	PlyWriter writer(column_name_vec, _SplatCnt);
	for (size_t i = 0; i < _SplatCnt; i++) {
		std::vector<f32> entry(column_name_vec.size());

		// position
		entry[0] = _SplatPositionVec[i].x;
		entry[1] = _SplatPositionVec[i].y;
		entry[2] = _SplatPositionVec[i].z;

		// f_dc & opacity
		constexpr f32 C0 = 0.28209479177387814f;
		entry[3] = (_SplatColorVec[i].r - 0.5f) / C0;
		entry[4] = (_SplatColorVec[i].g - 0.5f) / C0;
		entry[5] = (_SplatColorVec[i].b - 0.5f) / C0;
		entry[6] = std::logf(_SplatColorVec[i].a / (1 - _SplatColorVec[i].a));

		// scale
		entry[7] = std::logf(_SplatCovVec[i].s_x);
		entry[8] = std::logf(_SplatCovVec[i].s_y);
		entry[9] = std::logf(_SplatCovVec[i].s_z);

		// rot
		entry[10] = _SplatCovVec[i].quat_x;
		entry[11] = _SplatCovVec[i].quat_y;
		entry[12] = _SplatCovVec[i].quat_z;
		entry[13] = _SplatCovVec[i].quat_w;

		writer.add_entry(entry);
	}

	writer.write_to_file(filename);
}

}  // namespace lcc2ply
