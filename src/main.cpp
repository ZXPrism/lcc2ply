#include <parser.h>

using namespace lcc2ply;

int main() {
	Parser parser("scenes/PentHouse");
	if (!parser.parse_meta() || !parser.parse_index() || !parser.parse_fg(0)) {
		return -1;
	}
	parser.write_ply("output.ply");

	return 0;
}
