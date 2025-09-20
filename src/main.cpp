#include <CLI/CLI.hpp>
#include <parser.h>

using namespace lcc2ply;

int main(int argc, char *argv[]) {
	CLI::App app{ "lcc2ply by ZXPrism" };

	std::string scene_path;
	std::string output_file_path;
	size_t lod_level = 0;

	app.add_option("-i,--input", scene_path, "Scene path, e.g. scenes/PentHouse")
	    ->required()
	    ->check(CLI::ExistingFile);
	app.add_option("-o,--output", output_file_path, "Output file (ply) path")->required();
	app.add_option("-l,--lod", lod_level, "LOD level to extract");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}

	Parser parser(scene_path);
	if (!parser.parse_meta() || !parser.parse_index() || !parser.parse_fg(lod_level)) {
		return -1;
	}
	parser.write_ply(output_file_path);

	return 0;
}
