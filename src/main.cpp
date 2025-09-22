#include <CLI/CLI.hpp>
#include <parser.h>

using namespace lcc2ply;

int main(int argc, char *argv[]) {
	CLI::App app{ "lcc2ply by ZXPrism" };

	std::string scene_path;
	std::string output_file_path;
	std::string target_chunk;
	size_t lod_level = 0;
	bool analyze_mode = false;

	app.add_option("-i,--input", scene_path, "Scene path, e.g. scenes/PentHouse")->required();
	app.add_option("-o,--output", output_file_path, "Output file (ply) path")->default_val("output.ply");
	app.add_option("-l,--lod", lod_level, "LOD level to extract")->default_val(0);
	app.add_option("-c,--chunk", target_chunk, "Only extract scene data at chunk (x, y), usage: -c x,y");
	app.add_flag("-a,--analyze", analyze_mode, "Output basic information of the scene to stdout");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}

	Parser parser(scene_path);
	if (!parser.parse_meta() || !parser.parse_index()) {
		return -1;
	}

	if (analyze_mode) {
		parser.analyze();
	} else {
		if (!target_chunk.empty()) {
			const auto colon_pos = target_chunk.find(',');
			if (colon_pos == std::string::npos) {
				std::cout << "lcc2ply: invalid chunk pos\n";
				return -1;
			}
			const size_t chunk_pos_x = std::stoi(target_chunk.substr(0, colon_pos));
			const size_t chunk_pos_y = std::stoi(target_chunk.substr(colon_pos + 1));
			if (!parser.parse_fg(lod_level, true, chunk_pos_x, chunk_pos_y)) {
				return -1;
			}
		} else if (!parser.parse_fg(lod_level, false, 0, 0)) {
			return -1;
		}
		parser.write_ply(output_file_path);
	}

	return 0;
}
