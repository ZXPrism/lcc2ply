#pragma once

#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace lcc2ply {

constexpr bool DEBUG_OUTPUT [[maybe_unused]] = false;

template<typename TargetType>
[[nodiscard]] TargetType reinterpret_data(const void *data_base_addr) {
	TargetType res;
	std::memcpy(&res, data_base_addr, sizeof(TargetType));
	return res;
}

template<typename... Args>
void _print(std::format_string<Args...> fmt, Args &&...args) {
	std::cout << std::format(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void _println(std::format_string<Args...> fmt, Args &&...args) {
	std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
}

template<typename Ty>
class Range {
private:
	Ty _Left;
	Ty _Right;

public:
	Range(Ty left, Ty right)
	    : _Left(left)
	    , _Right(right) {}

	[[nodiscard]] Ty get(Ty t) const {
		return (_Left * (1 - t)) + (_Right * t);
	}

	[[nodiscard]] std::string string() const {
		return std::format("[{}, {}]", _Left, _Right);
	}
};

[[nodiscard]] std::vector<char> read_binary(const std::filesystem::path &path);

}  // namespace lcc2ply
