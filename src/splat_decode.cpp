#include <splat_decode.h>

#include <util.h>

#include <array>
#include <cassert>
#include <cmath>
#include <numbers>

namespace lcc2ply {

SplatPosition decode_splat_position(const PackedSplatData &packed_splat_data) {
	SplatPosition res;

	res.x = reinterpret_data<float>(&packed_splat_data.x);
	res.y = reinterpret_data<float>(&packed_splat_data.y);
	res.z = reinterpret_data<float>(&packed_splat_data.z);

	return res;
}

SplatColor decode_splat_color(const PackedSplatColor &packed_splat_color) {
	SplatColor res;

	res.r = static_cast<f32>(packed_splat_color.r) / 255.0f;
	res.g = static_cast<f32>(packed_splat_color.g) / 255.0f;
	res.b = static_cast<f32>(packed_splat_color.b) / 255.0f;
	res.a = static_cast<f32>(packed_splat_color.a) / 255.0f;

	return res;
}

SplatCov decode_splat_cov(const PackedSplatCov &packed_splat_cov, const Range<f32> &scale_range_x, const Range<f32> &scale_range_y, const Range<f32> &scale_range_z) {
	SplatCov res;

	constexpr f32 SCALE_NORM = static_cast<f32>(1.0 / 65535.0);
	res.s_x = scale_range_x.get(static_cast<f32>(packed_splat_cov.s_x) * SCALE_NORM);
	res.s_y = scale_range_y.get(static_cast<f32>(packed_splat_cov.s_y) * SCALE_NORM);
	res.s_z = scale_range_z.get(static_cast<f32>(packed_splat_cov.s_z) * SCALE_NORM);

	constexpr f32 ROT_NORM = static_cast<f32>(1.0 / 1023.0);
	constexpr f32 SQRT_2 = static_cast<f32>(std::numbers::sqrt2);
	constexpr f32 INV_SQRT_2 = static_cast<f32>(1.0 / std::numbers::sqrt2);
	constexpr std::array<std::array<u8, 4>, 4> QUAT_PERM{
		{ { 3, 0, 1, 2 },
		  { 0, 3, 1, 2 },
		  { 0, 1, 3, 2 },
		  { 0, 1, 2, 3 } }
	};

	const f32 quat_0 = static_cast<f32>(packed_splat_cov.rotation & 0x3FFu) * ROT_NORM;
	const f32 quat_1 = static_cast<f32>(packed_splat_cov.rotation >> 10 & 0x3FFu) * ROT_NORM;
	const f32 quat_2 = static_cast<f32>(packed_splat_cov.rotation >> 20 & 0x3FFu) * ROT_NORM;
	const u8 perm_type = packed_splat_cov.rotation >> 30;

	std::array<f32, 4> quat{
		(quat_0 * SQRT_2) - INV_SQRT_2,
		(quat_1 * SQRT_2) - INV_SQRT_2,
		(quat_2 * SQRT_2) - INV_SQRT_2,
		0.0f
	};
	quat[3] = std::sqrtf(std::max(0.0f, 1.0f - (quat[0] * quat[0]) - (quat[1] * quat[1]) - (quat[2] * quat[2])));

	assert(std::max({ quat[0], quat[1], quat[2] }) < quat[3] + 0.1f);

	res.quat_x = quat[QUAT_PERM[perm_type][0]];
	res.quat_y = quat[QUAT_PERM[perm_type][1]];
	res.quat_z = quat[QUAT_PERM[perm_type][2]];
	res.quat_w = quat[QUAT_PERM[perm_type][3]];

	return res;
}

}  // namespace lcc2ply
