#include <parser.h>
#include <ply_writer.h>

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
		std::println(std::cout, "lcc2ply: error: unable to read meta file from {}", meta_file_path.string());
		std::println(std::cout, "lcc2ply: {}", meta_errs);
		return false;
	}

	// parse lod distribution
	const auto &lod_distribution_node = _MetaRoot["splats"];
	_LodLevelCnt = lod_distribution_node.size();
	for (const auto &splat_cnt_per_lod_node : lod_distribution_node) {
		const int splat_cnt_per_lod = splat_cnt_per_lod_node.asInt();
		_SplatCntPerLodLevel.push_back(splat_cnt_per_lod);
	}

	std::println(std::cout, "lcc2ply: this scene contains {} lod levels", _LodLevelCnt);

	return true;
}

bool Parser::parse_index() {
	const fs::path scene_index_file_path = _SceneFolder / "index.bin";

	const std::vector<char> index_data = read_binary(scene_index_file_path);

	std::map<std::pair<size_t, size_t>, std::vector<ChunkInfo>> chunk_pos_to_chunk_info_vec;
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
			std::println(std::cout, "chunk at ({}, {})", chunk_pos.x, chunk_pos.y);
		}

		auto &chunk_info_vec = chunk_pos_to_chunk_info_vec[{ chunk_pos.x, chunk_pos.y }];
		for (size_t i = 0; i < _LodLevelCnt; i++) {
			const auto chunk_info = reinterpret_data<ChunkInfo>(index_data_base_addr + offset);
			offset += sizeof(ChunkInfo);

			if (DEBUG_OUTPUT) {
				std::println(std::cout,
				             "splat_cnt: {}, offset_bytes = {}, n_bytes = {}",
				             chunk_info.splat_cnt,
				             chunk_info.offset_bytes,
				             chunk_info.n_bytes);
			}

			splat_cnt_per_lod_level[i] += chunk_info.splat_cnt;

			if (chunk_info.splat_cnt != 0 && presum_size_bytes != chunk_info.offset_bytes) {
				std::println(std::cout, "lcc2ply: error: consistency check failed (prefix sum mismatch), index.bin may be corrupted");
				return false;
			}
			presum_size_bytes += chunk_info.n_bytes;

			chunk_info_vec.push_back(chunk_info);
		}

		if (DEBUG_OUTPUT) {
			std::cout << '\n';
		}
	}

	if (splat_cnt_per_lod_level != _SplatCntPerLodLevel) {
		std::println(std::cout, "lcc2ply: error: consistency check failed (splat cnt mismatch), index.bin may be corrupted");
		return false;
	}

	return true;
}

bool Parser::parse_fg() {
	// 1.2. parse scale range
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
		std::println(std::cout, "scale_range_x: {}", scale_range_x.string());
		std::println(std::cout, "scale_range_y: {}", scale_range_y.string());
		std::println(std::cout, "scale_range_z: {}", scale_range_z.string());
	}

	const fs::path scene_fg_data_file_path = _SceneFolder / "data.bin";

	const std::vector<char> fg_data = read_binary(scene_fg_data_file_path);
	const size_t data_size_bytes = fg_data.size();
	if (data_size_bytes % 32 != 0) {
		std::println(std::cout, "lcc2ply: error: the file size is not divisible by 32");
		return false;
	}

	const auto *data_base_addr = fg_data.data();
	size_t offset = 0;
	size_t splat_id = 0;

	while (offset < data_size_bytes) {
		const auto packed_splat_data = reinterpret_data<PackedSplatData>(data_base_addr + offset);
		const auto packed_splat_color = reinterpret_data<PackedSplatColor>(&packed_splat_data.color);
		const auto packed_splat_cov = reinterpret_data<PackedSplatCov>(&packed_splat_data.cov_1);

		if (DEBUG_OUTPUT) {
			std::println(std::cout, "### SPLAT {} ###", splat_id);
			std::print(std::cout, "packed_position = ({}, {}, {}) ", packed_splat_data.x, packed_splat_data.y, packed_splat_data.z);
			std::print(std::cout, "packed_color = {} ", packed_splat_data.color);
			std::print(std::cout, "packed_cov = ({}, {}, {}) ", packed_splat_data.cov_1, packed_splat_data.cov_2, packed_splat_data.cov_3);
			std::println(std::cout, "unused = {}", packed_splat_data.unused);
		}

		const auto position = decode_splat_position(packed_splat_data);
		const auto color = decode_splat_color(packed_splat_color);
		const auto cov = decode_splat_cov(packed_splat_cov, scale_range_x, scale_range_y, scale_range_z);

		if (DEBUG_OUTPUT) {
			std::println(std::cout, "position: ({}, {}, {})", position.x, position.y, position.z);
			std::println(std::cout, "color: ({}, {}, {})", color.r, color.g, color.b);
			std::println(std::cout, "scale: ({}, {}, {})", cov.s_x, cov.s_y, cov.s_z);
			std::println(std::cout, "rotation: ({}, {}, {}, {})", cov.quat_x, cov.quat_y, cov.quat_z, cov.quat_w);
			std::println(std::cout, "opacity: {}", color.a);
		}

		offset += 32;
		++splat_id;

		if (DEBUG_OUTPUT) {
			std::cout << '\n';
		}
	}

	return true;
}

bool Parser::parse_bg() {
	return true;
}

bool Parser::parse_sh() {
	return true;
}

void Parser::write_ply() const {
}

}  // namespace lcc2ply
