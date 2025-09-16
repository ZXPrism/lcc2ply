
#include <parser.h>

using namespace lcc2ply;

int main() {
	Parser parser("scenes/PentHouse");
	if (!parser.parse_meta() || !parser.parse_index() || !parser.parse_fg()) {
		return -1;
	}

	return 0;
}
