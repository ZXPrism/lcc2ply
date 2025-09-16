#include <splat_info.h>
#include <util.h>

namespace lcc2ply {

SplatPosition decode_splat_position(const PackedSplatData &packed_splat_data);
SplatColor decode_splat_color(const PackedSplatColor &packed_splat_color);
SplatCov decode_splat_cov(const PackedSplatCov &packed_splat_cov, const Range<f32> &scale_range_x, const Range<f32> &scale_range_y, const Range<f32> &scale_range_z);

}  // namespace lcc2ply
