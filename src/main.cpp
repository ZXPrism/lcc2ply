#include <CLI/CLI.hpp>
#include <parser.h>

using namespace lcc2ply;

int main(int argc, char *argv[]) {
	CLI::App app{ "lcc2ply by ZXPrism" };

	std::string scene_path;
	std::string output_file_path;
	size_t lod_level = 0;
	bool analyze_mode = false;

	app.add_option("-i,--input", scene_path, "Scene path, e.g. scenes/PentHouse")->required();
	app.add_option("-o,--output", output_file_path, "Output file (ply) path")->default_val("output.ply");
	app.add_option("-l,--lod", lod_level, "LOD level to extract")->default_val(0);
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
		if (!parser.parse_fg(lod_level)) {
			return -1;
		}
		parser.write_ply(output_file_path);
	}

	return 0;
}
