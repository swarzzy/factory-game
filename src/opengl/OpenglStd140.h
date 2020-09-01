#pragma once

#define layout_std140 alignas(16)

#define std140_vec2 alignas(8) v2
#define std140_vec3 alignas(16) v3
#define std140_int alignas(4) i32
#define std140_float alignas(4) f32
#define std140_mat4 alignas(16) m4x4
#define std140_mat4_array alignas(16) m4x4
#define std140_mat3 alignas(16) std140::mat3

namespace std140
{
#pragma pack(push, 1)
    union mat3 {
        v4 columns[3];
        float data[12];
        inline mat3() {}
        inline mat3(m3x3 m) {
            this->columns[0] = V4(m.columns[0], 0.0f);
            this->columns[1] = V4(m.columns[1], 0.0f);
            this->columns[2] = V4(m.columns[2], 0.0f);
        }
    };
#pragma pack(pop)
}
