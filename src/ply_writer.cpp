#include <ply_writer.h>
#include <util.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <print>

namespace lcc2ply {

PlyWriter::PlyWriter(const std::vector<std::string> &column_name_vec, size_t splat_cnt)
    : _SplatCnt(splat_cnt)
    , _ColumnNameVec(column_name_vec) {
}

void PlyWriter::add_entry(const std::vector<f32> &entry_data) {
	if (_ColumnNameVec.size() != entry_data.size()) {
		std::print(std::cout, "lcc2ply: error: entry data does not match with the columns");
		return;
	}

	_EntryVec.push_back(entry_data);
}

void PlyWriter::write_to_file(const std::string &filename) const {
	if (_SplatCnt != _EntryVec.size()) {
		std::print(std::cout, "lcc2ply: error: splat cnt does not match with entry cnt");
		return;
	}

	std::ofstream fout(filename, std::ios::binary);

	// ply
	// format binary_little_endian 1.0
	// element vertex 7
	// property float x
	// property float y
	// property float z
	// property float f_dc_0
	// property float f_dc_1
	// property float f_dc_2
	// property float opacity
	// property float scale_0
	// property float scale_1
	// property float scale_2
	// property float rot_0
	// property float rot_1
	// property float rot_2
	// property float rot_3
	// end_header
	// data...

	// write header
	fout << "ply\n";
	fout << "format binary_little_endian 1.0\n";
	fout << "element vertex" << ' ' << _SplatCnt << '\n';
	for (const auto &column_name : _ColumnNameVec) {
		fout << "property float" << ' ' << column_name << '\n';
	}
	fout << "end_header\n";

	// prepare data
	const size_t entry_size_bytes = _ColumnNameVec.size() * sizeof(f32);
	std::vector<char> binary(_SplatCnt * entry_size_bytes);

	std::println(std::cout, "lcc2ply: writing file {}...", filename);
	std::println(std::cout, "lcc2ply: file size: {} bytes", binary.size());

	for (size_t i = 0, offset = 0; i < _SplatCnt; i++, offset += entry_size_bytes) {
		std::memcpy(binary.data() + offset, _EntryVec[i].data(), entry_size_bytes);
	}

	// write data
	fout.write(binary.data(), binary.size());

	fout.close();
}

}  // namespace lcc2ply
