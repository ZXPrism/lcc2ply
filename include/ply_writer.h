#pragma once

#include <type.h>

#include <string>
#include <vector>

namespace lcc2ply {

class PlyWriter {
private:
	size_t _SplatCnt;
	std::vector<std::string> _ColumnNameVec;
	std::vector<std::vector<f32>> _EntryVec;

public:
	PlyWriter(const std::vector<std::string> &column_name_vec, size_t splat_cnt);

	void add_entry(const std::vector<f32> &entry_data);

	void write_to_file(const std::string &filename) const;
};

}  // namespace lcc2ply
