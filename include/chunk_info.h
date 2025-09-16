#pragma once

#include <type.h>

namespace lcc2ply {

struct ChunkPos {
	u16 x;
	u16 y;
};

#pragma pack(push)
#pragma pack(4)
struct ChunkInfo {
	u32 splat_cnt;
	u64 offset_bytes;
	u32 n_bytes;
};
#pragma pack(pop)

}  // namespace lcc2ply
