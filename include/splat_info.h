#pragma once

#include <type.h>

namespace lcc2ply {

struct PackedSplatData {
	u32 x;
	u32 y;
	u32 z;
	u32 color;
	u32 cov_1;
	u32 cov_2;
	u32 cov_3;
	u32 unused;
};

struct PackedSplatColor {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};

#pragma pack(push)
#pragma pack(2)
struct PackedSplatCov {
	u16 s_x;
	u16 s_y;
	u16 s_z;
	u32 rotation;
	u16 unused;
};
#pragma pack(pop)

struct SplatPosition {
	f32 x;
	f32 y;
	f32 z;
};

struct SplatColor {
	f32 r;
	f32 g;
	f32 b;
	f32 a;
};

struct SplatCov {
	f32 s_x;
	f32 s_y;
	f32 s_z;
	f32 quat_x;
	f32 quat_y;
	f32 quat_z;
	f32 quat_w;
};

}  // namespace lcc2ply
