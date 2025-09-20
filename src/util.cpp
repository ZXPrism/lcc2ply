#include <util.h>

#include <fstream>
#include <iostream>

namespace lcc2ply {

std::vector<char> read_binary(const std::filesystem::path &path) {
	const auto data_size_bytes = std::filesystem::file_size(path);
	std::vector<char> res(data_size_bytes);

	std::ifstream fin(path, std::ios::binary);
	fin.read(res.data(), data_size_bytes);
	fin.close();

	_println("lcc2ply: read file {} with size {} bytes", path.string(), data_size_bytes);

	return res;
}

}  // namespace lcc2ply
