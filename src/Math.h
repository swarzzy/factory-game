#pragma once
#include <stdlib.h>
#include "Intrinsics.h"

struct RandomSeries {
    // TODO: Random
};

f32 RandomUnilateral(RandomSeries* series) {
    f32 result = (f32)rand() / (f32)RAND_MAX;
    return result;
}

template <typename T, u32 _Size>
union Vector {};

template<typename T>
union Vector<T, 2> {
    static constexpr u32 Size = 2;
    struct {
        T x, y;
    };
    struct {
        T r, g;
    };
    struct {
        T u, v;
    };
    T data[Size];
};

template<typename T>
union Vector<T, 3> {
    static constexpr u32 Size = 3;
    struct {
        T x, y, z;
    };
    struct {
        T r, g, b;
    };
    struct {
        Vector<T, 2> xy;
        T _unused;
    };
    T data[Size];
};

template<typename T>
union Vector<T, 4> {
    static constexpr u32 Size = 4;
    struct {
        T x, y, z, w;
    };
    struct {
        T r, g, b, a;
    };
    struct {
        Vector<T, 2> xy;
        T _unused1[2];
    };
    struct {
        Vector<T, 3> xyz;
        T _unused2;
    };
    T data[Size];
};

typedef Vector<f32, 2> v2;
typedef Vector<f32, 3> v3;
typedef Vector<f32, 4> v4;

typedef Vector<u32, 2> uv2;
typedef Vector<u32, 3> uv3;
typedef Vector<u32, 4> uv4;

typedef Vector<i32, 2> iv2;
typedef Vector<i32, 3> iv3;
typedef Vector<i32, 4> iv4;

template<u32 Size>
union SquareMatrix {};

template<>
union SquareMatrix<3> {
    static const u32 Size = 3;
    v3 columns[Size];
    f32 data[Size * Size];
    struct {
        f32 _11, _21, _31;
        f32 _12, _22, _32;
        f32 _13, _23, _33;
    };

    f32& At(u32 i, u32 j) {
        return this->data[i + Size * j];
    }
};

template<>
union SquareMatrix<4> {
    static const u32 Size = 4;
    v4 columns[Size];
    f32 data[Size * Size];
    struct
    {
        f32 _11, _21, _31, _41;
        f32 _12, _22, _32, _42;
        f32 _13, _23, _33, _43;
        f32 _14, _24, _34, _44;
    };

    f32& At(u32 i, u32 j) {
        return this->data[i + Size * j];
    }
};

typedef SquareMatrix<4> m4x4;
typedef SquareMatrix<3> m3x3;

constexpr v2 V2(f32 x, f32 y) { return v2{x, y}; }
constexpr v2 V2(f32 val) { return v2{val, val}; }
constexpr v2 V2(v3 v) { return v2{v.x, v.y}; }
constexpr v2 V2(v4 v) { return v2{v.x, v.y}; }

constexpr v3 V3(f32 x, f32 y, f32 z) { return v3{x, y, z}; }
constexpr v3 V3(f32 val) { return v3{val, val, val}; }
constexpr v3 V3(v2 v, f32 z) { return v3{v.x, v.y, z}; }
constexpr v3 V3(v4 v) { return v3{v.x, v.y, v.z}; }
constexpr v3 V3(u32 x, u32 y, u32 z) { return v3{(f32)x, (f32)y, (f32)z}; }
constexpr v3 V3(iv3 v) { return v3{(f32)v.x, (f32)v.y, (f32)v.z}; }

constexpr v4 V4(f32 x, f32 y, f32 z, f32 w) { return v4{x, y ,z, w}; }
constexpr v4 V4(f32 val) { return v4{val, val , val, val}; }
constexpr v4 V4(v2 v, f32 z, f32 w) { return v4{v.x, v.y ,z, w}; }
constexpr v4 V4(v3 v, f32 w) { return v4{v.x, v.y ,v.z, w}; }

constexpr uv2 UV2(u32 x, u32 y) { return uv2{x, y }; }
constexpr uv2 UV2(u32 val) { return uv2{val, val}; }
constexpr uv2 UV2(uv3 v) { return uv2{v.x, v.y}; }
constexpr uv2 UV2(uv4 v) { return uv2{v.x, v.y}; }

constexpr uv3 UV3(u32 x, u32 y, u32 z) { return uv3{x, y, z}; }
constexpr uv3 UV3(u32 val) { return uv3{val, val, val}; }
constexpr uv3 UV3(uv2 v, u32 z) { return uv3{v.x, v.y, z}; }
constexpr uv3 UV3(uv4 v) { return uv3{v.x, v.y, v.z}; }

constexpr uv4 UV4(u32 x, u32 y, u32 z, u32 w) { return uv4{x, y, z, w}; }
constexpr uv4 UV4(u32 val) { return uv4{val, val , val, val}; }
constexpr uv4 UV4(uv2 v, u32 z, u32 w) { return uv4{v.x, v.y ,z, w}; }
constexpr uv4 UV4(uv3 v, u32 w) { return uv4{v.x, v.y ,v.z, w}; }

constexpr iv2 IV2(i32 x, i32 y) { return iv2{x, y}; }
constexpr iv2 IV2(i32 val) { return iv2{val, val}; }
constexpr iv2 IV2(iv3 v) { return iv2{v.x, v.y}; }
constexpr iv2 IV2(iv4 v) { return iv2{v.x, v.y}; }

constexpr iv3 IV3(i32 x, i32 y, i32 z) { return iv3{x, y, z}; }
constexpr iv3 IV3(i32 val) { return iv3{val, val, val}; }
constexpr iv3 IV3(iv2 v, i32 z) { return iv3{v.x, v.y, z}; }
constexpr iv3 IV3(iv4 v) { return iv3{v.x, v.y, v.z}; }

constexpr iv4 IV4(i32 x, i32 y, i32 z, i32 w) { return iv4{x, y, z, w}; }
constexpr iv4 IV4(i32 val) { return iv4{val, val, val, val}; }
constexpr iv4 IV4(iv2 v, i32 z, i32 w) { return iv4{v.x, v.y ,z, w}; }
constexpr iv4 IV4(iv3 v, i32 w) { return iv4{v.x, v.y ,v.z, w}; }

constexpr m3x3 M3x3(f32 diag) { m3x3 result = {}; result._11 = diag; result._22 = diag; result._33 = diag; return result; }
constexpr m4x4 M4x4(f32 diag) { m4x4 result = {};  result._11 = diag; result._22 = diag; result._33 = diag; result._44 = diag; return result; }
m3x3 M3x3(m4x4 m) { m3x3 result; result._11 = m._11; result._12 = m._12; result._13 = m._13; result._21 = m._21; result._22 = m._22; result._23 = m._23; result._31 = m._31; result._32 = m._32; result._33 = m._33; return result; }
m4x4 M4x4(m3x3 m) { m4x4 result = {}; result._11 = m._11; result._12 = m._12; result._13 = m._13; result._21 = m._21; result._22 = m._22; result._23 = m._23; result._31 = m._31; result._32 = m._32; result._33 = m._33; result._44 = 1.0f; return result; }

template <typename T, u32 Size>
bool operator==(Vector<T, Size> l, Vector<T, Size> r) {
    bool result = true;
    for (u32x i = 0; i < Size; i++) {
        if (l.data[i] != r.data[i]) {
            result = false;
            break;
        }
    }
    return result;
}

template <typename T, u32 Size>
bool operator!=(Vector<T, Size> l, Vector<T, Size> r) {
    bool result = !(l == r);
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator+(Vector<T, Size> l, Vector<T, Size> r) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = l.data[i] + r.data[i];
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator+(Vector<T, Size> v, T s) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] + s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator+(f32 s, Vector<T, Size> v) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] + s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size>& operator+=(Vector<T, Size>& l, Vector<T, Size> r) {
    for (u32x i = 0; i < Size; i++) {
        l.data[i] += r.data[i];
    }
    return l;
}

template <typename T, u32 Size>
Vector<T, Size>& operator+=(Vector<T, Size>& v, T s) {
    for (u32x i = 0; i < Size; i++) {
        v.data[i] += s;
    }
    return l;
}

template <typename T, u32 Size>
Vector<T, Size> operator-(Vector<T, Size> vec) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = -vec.data[i];
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator-(Vector<T, Size> l, Vector<T, Size> r) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = l.data[i] - r.data[i];
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator-(Vector<T, Size> v, T s) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] - s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator-(f32 s, Vector<T, Size> v) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] - s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size>& operator-=(Vector<T, Size>& l, Vector<T, Size> r) {
    for (u32x i = 0; i < Size; i++) {
        l.data[i] -= r.data[i];
    }
    return l;
}

template <typename T, u32 Size>
Vector<T, Size>& operator-=(Vector<T, Size>& v, T s) {
    for (u32x i = 0; i < Size; i++) {
        v.data[i] -= s;
    }
    return v;
}

template <typename T, u32 Size>
Vector<T, Size> operator*(Vector<T, Size> v, T s) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] * s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator*(T s, Vector<T, Size> v) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] * s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size>& operator*=(Vector<T, Size>& v, T s) {
    for (u32x i = 0; i < Size; i++) {
        v.data[i] *= s;
    }
    return v;
}

template <typename T, u32 Size>
Vector<T, Size> operator/(Vector<T, Size> v, T s) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] / s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> operator/(f32 s, Vector<T, Size> v) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = v.data[i] / s;
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size>& operator/=(Vector<T, Size>& v, T s) {
    for (u32x i = 0; i < Size; i++) {
        v.data[i] /= s;
    }
    return l;
}

template <typename T, u32 Size>
T LengthSq(Vector<T, Size> v) {
    T result = static_cast<T>(0);
    for (u32x i = 0; i < Size; i++) {
        result += v.data[i] * v.data[i];
    }
    return result;
}

template <u32 Size>
f32 Length(Vector<f32, Size> v) {
    auto result = Sqrt(LengthSq(v));
    return result;
}

template <u32 Size>
f32 DistSq(Vector<f32, Size> v1, Vector<f32, Size> v2) {
    f32 result = 0.0f;
    for (u32x i = 0; i < Size; i++) {
        result += v2.data[i] - v1.data[i];
    }
    return result;
}

template <u32 Size>
f32 Dist(Vector<f32, Size> v1, Vector<f32, Size> v2) {
    auto result = Sqrt(DistSq(v1, v2));
    return result;
}

template <u32 Size>
Vector<f32, Size> Normalize(Vector<f32, Size> v) {
    Vector<f32, Size> result = {};
    auto len = Length(v);
    if (len > F32::Eps) {
        for (u32x i = 0; i < Size; i++) {
            result.data[i] = v.data[i] / len;
        }
    }
    return result;
}

template <u32 Size>
Vector<f32, Size> Lerp(Vector<f32, Size> a, Vector<f32, Size> b, f32 t) {
    Vector<f32, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = Lerp(a.data[i], b.data[i], t);
    }
    return result;
}

template <typename T, u32 Size>
Vector<T, Size> Hadamard(Vector<T, Size> a, Vector<T, Size> b) {
    Vector<T, Size> result;
    for (u32x i = 0; i < Size; i++) {
        result.data[i] = a.data[i] * b.data[i];
    }
    return result;
}

template <u32 Size>
f32 Dot(Vector<f32, Size> a, Vector<f32, Size> b) {
    f32 result = 0.0f;
    for (u32x i = 0; i < Size; i++) {
        result += a.data[i] * b.data[i];
    }
    return result;
}

v3 Cross(v3 l, v3 r) {
    auto result = v3 { l.y * r.z - l.z * r.y,
                       l.z * r.x - l.x * r.z,
                       l.x * r.y - l.y * r.x };
    return result;
}

template<u32 Size>
SquareMatrix<Size> operator*(SquareMatrix<Size> left, SquareMatrix<Size> right) {
    SquareMatrix<Size> result = {};
    for (u32x i = 0; i < Size; i++) {
        for (u32x j = 0; j < Size; j++) {
            for (u32x k = 0; k < Size; k++) {
                result.At(i, j) += left.At(i, k) * right.At(k, j);
            }
        }
    }
    return result;
}

template<u32 Size>
Vector<f32, Size> operator*(SquareMatrix<Size> left, Vector<f32, Size> right) {
    Vector<f32, Size> result = {};
    for (u32x i = 0; i < Size; i++) {
        for (u32x j = 0; j < Size; j++) {
            result.data[i] += left.At(i, j) * right.data[j];
        }
    }
    return result;
}

template<u32 Size>
SquareMatrix<Size> Transpose(SquareMatrix<Size> m) {
    SquareMatrix<Size> result;
    for (u32x i = 0; i < Size; i++) {
        for (u32x j = 0; j < Size; j++) {
            result.data[j + Size * i] = m.data[i + Size * j];
        }
    }
    return result;
}

m4x4 Translate(v3 offset) {
    m4x4 result = {};
    result._11 = 1.0f;
    result._22 = 1.0f;
    result._33 = 1.0f;
    result._44 = 1.0f;

    result._14 = offset.x;
    result._24 = offset.y;
    result._34 = offset.z;

    return result;
}

v3 ExtractTranslation(m4x4 m) {
    return V3(m._14, m._24, m._34);
}

m4x4 Scale(v3 scalar) {
    m4x4 result = {};
    result._11 = scalar.x;
    result._22 = scalar.y;
    result._33 = scalar.z;
    result._44 = 1.0f;

    return result;
}
#undef far
#undef near
m4x4 OrthoGLRH(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
    m4x4 m = {};
    m._11 = 2.0f / (right - left);
    m._22 = 2.0f / (top - bottom);
    m._33 = -2.0f / (far - near);
    m._14 = -(right + left) / (right - left);
    m._24 = -(top + bottom) / (top - bottom);
    m._34 = -(far + near) / (far - near);
    m._44 = 1.0f;
    return m;
}

m4x4 PerspectiveGLRH(f32 near, f32 far, f32 fov, f32 aRatio) {
    m4x4 m = {};
    f32 c = 1.0f / Tan(ToRad(fov) / 2.0f);
    m._11 = c / aRatio;
    m._22 = c;
    m._33 = -(far + near) / (far - near);
    m._34 = -(2.0f * far * near) / (far - near);
    m._43 = -1.0f;
    return m;
}

m4x4 LookAtGLRH(v3 eye, v3 front, v3 up) {
    m4x4 m = {};

    auto z = Normalize(front);
    auto x = Normalize(Cross(up, z));
    auto y = Normalize(Cross(z, x));

    m._11 = x.x;
    m._12 = x.y;
    m._13 = x.z;
    m._14 = -Dot(x, eye);

    m._21 = y.x;
    m._22 = y.y;
    m._23 = y.z;
    m._24 = -Dot(y, eye);

    m._31 = z.x;
    m._32 = z.y;
    m._33 = z.z;
    m._34 = -Dot(z, eye);

    m._44 = 1.0f;

    return m;
}

m3x3 RotateX(f32 angleDeg) {
    m3x3 m = {};
    f32 c = Cos(ToRad(angleDeg));
    f32 s = Sin(ToRad(angleDeg));

    m._11 = 1.0f;
    m._22 = c;
    m._33 = c;
    m._32 = -s;
    m._23 = s;

    return m;
}

m3x3 RotateY(f32 angleDeg) {
    m3x3 m = {};
    f32 c = Cos(ToRad(angleDeg));
    f32 s = Sin(ToRad(angleDeg));

    m._11 = c;
    m._22 = 1.0f;
    m._33 = c;
    m._31 = -s;
    m._13 = s;

    return m;
}

m3x3 RotateZ(f32 angleDeg) {
    m3x3 m = {};
    f32 c = Cos(ToRad(angleDeg));
    f32 s = Sin(ToRad(angleDeg));

    m._11 = c;
    m._22 = c;
    m._21 = -s;
    m._12 = s;
    m._33 = 1.0f;

    return m;
}

// TODO: Crappy and slow. Use quaternions
m4x4 Rotate(f32 xAngle, f32 yAngle, f32 zAngle) {
    return M4x4(RotateZ(zAngle) * RotateY(yAngle) * RotateX(xAngle));
}

f32 Determinant(m3x3 m) {
    auto result = m._11 * m._22 * m._33 - m._11 * m._23 * m._32
        - m._12 * m._21 * m._33 + m._12 * m._23 * m._31
        + m._13 * m._21 * m._32 - m._13 * m._22 * m._31;
    return result;
}

f32 Determinant(m4x4 m) {
        f32 minor1 = m._11 * (m._22 * m._33 * m._44 - m._22 * m._34 * m._43
                              - m._23 * m._32 * m._44 + m._23 * m._34 * m._42
                              + m._24 * m._32 * m._43 - m._24 * m._33 * m._42);

        f32 minor2 = m._12 * (m._21 * m._33 * m._44 - m._21 * m._34 * m._43
                              - m._23 * m._31 * m._44 + m._23 * m._41 * m._34
                              +m._24 * m._31 * m._43 - m._24 * m._33 * m._41);

        f32 minor3 = m._13 * (m._21 * m._32 * m._44 - m._21 * m._34 * m._42
                              - m._22 * m._31 * m._44 + m._22 * m._34 * m._41
                              + m._24 * m._31 * m._42 - m._24 * m._32 * m._41);

        f32 minor4 = m._14 * (m._21 * m._32 * m._43 - m._21 * m._33 * m._42
                              - m._22 * m._31 * m._43 + m._22 * m._41 * m._33
                              + m._23 * m._31 * m._42 - m._23 * m._32 * m._41);

    auto result = minor1 - minor2 + minor3 - minor4;
    return result;
}

m3x3 Inverse(m3x3 m) {
    f32 a11 = m._22 * m._33 - m._23 * m._32;
    f32 a12 = - (m._21 * m._33 - m._23 * m._31);
    f32 a13 = m._21 * m._32 - m._22 * m._31;

    f32 det = (m._11 * a11 + m._12 * a12 + m._13 * a13);

    if (Abs(det) > F32::Eps) {
        f32 a21 = - (m._12 * m._33 - m._32 * m._13);
        f32 a22 = m._11 * m._33 - m._13 * m._31;
        f32 a23 = - (m._11 * m._32 - m._12 * m._31);

        f32 a31 = m._12 * m._23 - m._22 * m._13;
        f32 a32 = - (m._11 * m._23 - m._21 * m._13);
        f32 a33 = m._11 * m._22 - m._21 * m._12;

        f32 oneOverDet = 1.0f / det;

        m3x3 inv;
        inv._11 = a11 * oneOverDet;
        inv._12 = a21 * oneOverDet;
        inv._13 = a31 * oneOverDet;
        inv._21 = a12 * oneOverDet;
        inv._22 = a22 * oneOverDet;
        inv._23 = a32 * oneOverDet;
        inv._31 = a13 * oneOverDet;
        inv._32 = a23 * oneOverDet;
        inv._33 = a33 * oneOverDet;

        return inv;
    }
    assert(false);
    return M3x3(0.0f);
}

m4x4 Inverse(m4x4 m) {
    f32 a11_22 = m._33 * m._44 - m._34 * m._43;
    f32 a11_23 = m._32 * m._44 - m._34 * m._42;
    f32 a11_24 = m._32 * m._43 - m._33 * m._42;

    f32 A11 = m._22 * a11_22 - m._23 * a11_23 + m._24 * a11_24;

    f32 a12_21 = m._33 * m._44 - m._34 * m._43;
    f32 a12_23 = m._31 * m._44 - m._34 * m._41;
    f32 a12_24 = m._31 * m._43 - m._33 * m._41;

    f32 A12 = -(m._21 * a12_21 - m._23 * a12_23 + m._24 * a12_24);

    f32 a13_21 = m._32 * m._44 - m._34 * m._42;
    f32 a13_22 = m._31 * m._44 - m._34 * m._41;
    f32 a13_24 = m._31 * m._42 - m._32 * m._41;

    f32 A13 = m._21 * a13_21 - m._22 * a13_22 + m._24 * a13_24;

    f32 a14_21 = m._32 * m._43 - m._33 * m._42;
    f32 a14_22 = m._31 * m._43 - m._33 * m._41;
    f32 a14_23 = m._31 * m._42 - m._32 * m._41;

    f32 A14 = -(m._21 * a14_21 - m._22 * a14_22 + m._23 * a14_23);

    f32 a21_12 = m._33 * m._44 - m._34 * m._43;
    f32 a21_13 = m._32 * m._44 - m._34 * m._42;
    f32 a21_14 = m._32 * m._43 - m._33 * m._42;

    f32 A21 = -(m._12 * a21_12 - m._13 * a21_13 + m._14 * a21_14);

    f32 a22_11 = m._33 * m._44 - m._34 * m._43;
    f32 a22_13 = m._31 * m._44 - m._34 * m._41;
    f32 a22_14 = m._31 * m._43 - m._33 * m._41;

    f32 A22 = m._11 * a22_11 - m._13 * a22_13 + m._14 * a22_14;

    f32 a23_11 = m._32 * m._44 - m._34 * m._42;
    f32 a23_12 = m._31 * m._44 - m._34 * m._41;
    f32 a23_14 = m._31 * m._42 - m._32 * m._41;

    f32 A23 = -(m._11 * a23_11 - m._12 * a23_12 + m._14 * a23_14);

    f32 a24_11 = m._32 * m._43 - m._33 * m._42;
    f32 a24_12 = m._31 * m._43 - m._33 * m._41;
    f32 a24_13 = m._31 * m._42 - m._32 * m._41;

    f32 A24 = m._11 * a24_11 - m._12 * a24_12 + m._13 * a24_13;

    f32 a31_12 = m._23 * m._44 - m._24 * m._43;
    f32 a31_13 = m._22 * m._44 - m._24 * m._42;
    f32 a31_14 = m._22 * m._43 - m._23 * m._42;

    f32 A31 = m._12 * a31_12 - m._13 * a31_13 + m._14 * a31_14;

    f32 a32_11 = m._23 * m._44 - m._24 * m._43;
    f32 a32_13 = m._21 * m._44 - m._24 * m._41;
    f32 a32_14 = m._21 * m._43 - m._23 * m._41;

    f32 A32 = -(m._11 * a32_11 - m._13 * a32_13 + m._14 * a32_14);

    f32 a33_11 = m._22 * m._44 - m._24 * m._42;
    f32 a33_12 = m._21 * m._44 - m._24 * m._41;
    f32 a33_14 = m._21 * m._42 - m._22 * m._41;

    f32 A33 = m._11 * a33_11 - m._12 * a33_12 + m._14 * a33_14;

    f32 a34_11 = m._22 * m._43 - m._23 * m._42;
    f32 a34_12 = m._21 * m._43 - m._23 * m._41;
    f32 a34_13 = m._21 * m._42 - m._22 * m._41;

    f32 A34 = -(m._11 * a34_11 - m._12 * a34_12 + m._13 * a34_13);

    f32 a41_12 = m._23 * m._34 - m._24 * m._33;
    f32 a41_13 = m._22 * m._34 - m._24 * m._32;
    f32 a41_14 = m._22 * m._33 - m._23 * m._32;

    f32 A41 = -(m._12 * a41_12 - m._13 * a41_13 + m._14 * a41_14);

    f32 a42_11 = m._23 * m._34 - m._24 * m._33;
    f32 a42_13 = m._21 * m._34 - m._24 * m._31;
    f32 a42_14 = m._21 * m._33 - m._23 * m._31;

    f32 A42 = m._11 * a42_11 - m._13 * a42_13 + m._14 * a42_14;

    f32 a43_11 = m._22 * m._34 - m._24 * m._32;
    f32 a43_12 = m._21 * m._34 - m._24 * m._31;
    f32 a43_14 = m._21 * m._32 - m._22 * m._31;

    f32 A43 = -(m._11 * a43_11 - m._12 * a43_12 + m._14 * a43_14);

    f32 a44_11 = m._22 * m._33 - m._23 * m._32;
    f32 a44_12 = m._21 * m._33 - m._23 * m._31;
    f32 a44_13 = m._21 * m._32 - m._22 * m._31;

    f32 A44 = m._11 * a44_11 - m._12 * a44_12 + m._13 * a44_13;

    f32 det = m._11 * A11 + m._12 * A12 + m._13 * A13 + m._14 * A14;

    if (det == 0) {
        assert(false);
        return M4x4(0.0f);
    } else {
        m4x4 result;
        f32 oneOverDet = 1.0f / det;
        result._11 = A11 * oneOverDet;
        result._12 = A21 * oneOverDet;
        result._13 = A31 * oneOverDet;
        result._14 = A41 * oneOverDet;
        result._21 = A12 * oneOverDet;
        result._22 = A22 * oneOverDet;
        result._23 = A32 * oneOverDet;
        result._24 = A42 * oneOverDet;
        result._31 = A13 * oneOverDet;
        result._32 = A23 * oneOverDet;
        result._33 = A33 * oneOverDet;
        result._34 = A43 * oneOverDet;
        result._41 = A14 * oneOverDet;
        result._42 = A24 * oneOverDet;
        result._43 = A34 * oneOverDet;
        result._44 = A44 * oneOverDet;

        return result;
    }
}

// TODO: These should be reworked
struct Basis {
    // NOTE: Assumed to be normalized
    v3 p;
    v3 xAxis;
    v3 yAxis;
    v3 zAxis;
};

union FrustumCorners {
    struct  {
        v3 nlt; // near left top
        v3 nrt;
        v3 nlb; // near left botom
        v3 nrb;
        v3 flt; // far left top
        v3 frt;
        v3 flb;
        v3 frb;
    };
    v3 corners[8];
};

FrustumCorners GetFrustumCorners(Basis basis, f32 fovDeg, f32 aspectRatio, f32 n, f32 f) {
    FrustumCorners result;
    f32 tanHalfFov2 = Tan(ToRad(fovDeg / 2.0f)) * 2.0f;

    f32 halfNearHeight = tanHalfFov2 * n * 0.5f;
    f32 halfNearWidth = halfNearHeight * aspectRatio;

    f32 halfFarHeight = tanHalfFov2 * f * 0.5f;
    f32 halfFarWidth = halfFarHeight * aspectRatio;

    v3 nearCenter = basis.p + basis.zAxis * n;
    v3 farCenter = basis.p + basis.zAxis * f;

    result.nlt = nearCenter + basis.yAxis * halfNearHeight - basis.xAxis * halfNearWidth;
    result.nrt = nearCenter + basis.yAxis * halfNearHeight + basis.xAxis * halfNearWidth;
    result.nlb = nearCenter - basis.yAxis * halfNearHeight - basis.xAxis * halfNearWidth;
    result.nrb = nearCenter - basis.yAxis * halfNearHeight + basis.xAxis * halfNearWidth;
    result.flt = farCenter + basis.yAxis * halfFarHeight - basis.xAxis * halfFarWidth;
    result.frt = farCenter + basis.yAxis * halfFarHeight + basis.xAxis * halfFarWidth;
    result.flb = farCenter - basis.yAxis * halfFarHeight - basis.xAxis * halfFarWidth;
    result.frb = farCenter - basis.yAxis * halfFarHeight + basis.xAxis * halfFarWidth;

    return result;
}

//
// Bounding boxes
//

struct Mesh;

union BBoxAligned {
    struct {
        v3 min;
        v3 max;
    };
    f32 data[6];

    static BBoxAligned From(Mesh* mesh);
    static inline BBoxAligned Realign(BBoxAligned box) {
        BBoxAligned result = {};
        result.min.x = Min(box.min.x, box.max.x);
        result.min.y = Min(box.min.y, box.max.y);
        result.min.z = Min(box.min.z, box.max.z);
        result.max.x = Max(box.min.x, box.max.x);
        result.max.y = Max(box.min.y, box.max.y);
        result.max.z = Max(box.min.z, box.max.z);
        return result;
    }
};

v3 GetBarycentric(v3 boxMin, v3 boxMax, v3 p) {
    v3 result;
    result.x = SafeRatio0(p.x - boxMin.x, boxMax.x - boxMin.x);
    result.y = SafeRatio0(p.y - boxMin.y, boxMax.y - boxMin.y);
    result.z = SafeRatio0(p.z - boxMin.z, boxMax.z - boxMin.z);
    return result;
}

bool IntersectFast(BBoxAligned box, v3 ro, v3 rd, f32 tMin, f32 tMax) {
    // Andrew Kensler's algorithm
    for (u32x i = 0; i < 3; i++) {
        f32 invD = 1.0f / rd.data[i];
        f32 t0 = (box.min.data[i] - ro.data[i]) * invD;
        f32 t1 = (box.max.data[i] - ro.data[i]) * invD;
        if (invD < 0.0f) {
            f32 tmp = t1;
            t1 = t0;
            t0 = tmp;
        }

        tMin = t0 > tMin ? t0 : tMin;
        tMax = t1 < tMax ? t1 : tMax;

        if (tMax <= tMin) {
            return false;
        }
    }
    return true;
}

struct IntersectionResult {
    b32 hit;
    f32 t;
    v3 normal;
    iv3 iNormal;
};

f32 TestAABBSideVsRay(BBoxAligned box, v3 ro, v3 rd, f32 tMinBound, f32 tMaxBound, int xi, int yi, int zi, bool boxi) {
    int boff = boxi ? 3 : 0;
    f32 tMin = F32::Max;
    if (Abs(rd.data[xi]) > F32::Eps) {
        f32 t = (box.data[boff + xi] - ro.data[xi]) / rd.data[xi];
        if (t > tMinBound && t < tMaxBound) {
            f32 y = ro.data[yi] + rd.data[yi] * t;
            f32 z = ro.data[zi] + rd.data[zi] * t;
            if (y >= box.data[yi] && y <= box.data[3 + yi] && z >= box.data[zi] && z <= box.data[3 + zi]) {
                tMin = t;
            }
        }
    }
    return tMin;
}

IntersectionResult Intersect(BBoxAligned box, v3 ro, v3 rd, f32 tMinBound, f32 tMaxBound) {
    f32 tMin = F32::Max;
    v3 n = {};
    iv3 in = {};

    f32 t = F32::Max;
    t = TestAABBSideVsRay(box, ro, rd, tMinBound, tMaxBound, 0, 1, 2, false);
    if (t < tMin) {
        tMin = t;
        n = V3(-1.0f, 0.0f, 0.0f);
        in = IV3(-1, 0, 0);
    }
    t = TestAABBSideVsRay(box, ro, rd, tMinBound, tMaxBound, 1, 0, 2, false);
    if (t < tMin) {
        tMin = t;
        n = V3(0.0f, -1.0f, 0.0f);
        in = IV3(0, -1, 0);
    }
    t = TestAABBSideVsRay(box, ro, rd, tMinBound, tMaxBound, 2, 0, 1, false);
    if (t < tMin) {
        tMin = t;
        n = V3(0.0f, 0.0f, -1.0f);
        in = IV3(0, 0, -1);
    }
    t = TestAABBSideVsRay(box, ro, rd, tMinBound, tMaxBound, 0, 1, 2, true);
    if (t < tMin) {
        tMin = t;
        n = V3(1.0f, 0.0f, 0.0f);
        in = IV3(1, 0, 0);
    }
    t = TestAABBSideVsRay(box, ro, rd, tMinBound, tMaxBound, 1, 0, 2, true);
    if (t < tMin) {
        tMin = t;
        n = V3(0.0f, 1.0f, 0.0f);
        in = IV3(0, 1, 0);
    }
    t = TestAABBSideVsRay(box, ro, rd, tMinBound, tMaxBound, 2, 0, 1, true);
    if (t < tMin) {
        tMin = t;
        n = V3(0.0f, 0.0f, 1.0f);
        in = IV3(0, 0, 1);
    }

    return { tMin < F32::Max, tMin, n, in };
}

struct TriangleIntersectionResult {
    b32 hit;
    f32 t;
};

TriangleIntersectionResult IntersectRayTriangle(v3 ro, v3 rd, v3 vt0, v3 vt1, v3 vt2) {
    // TODO: This placeholder code taken from https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%9C%D0%BE%D0%BB%D0%BB%D0%B5%D1%80%D0%B0_%E2%80%94_%D0%A2%D1%80%D1%83%D0%BC%D0%B1%D0%BE%D1%80%D0%B0
    // It should be replaced with actual intersection code
    v3 e1 = vt1 - vt0;
    v3 e2 = vt2 - vt0;
    v3 pvec = Cross(rd, e2);
    f32 det = Dot(e1, pvec);
    if (Abs(det) < F32::Eps) {
        return {false, 0};
    }
    f32 invDet = 1.0f / det;
    v3 tvec = ro - vt0;
    f32 u = Dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f) {
        return {false, 0};
    }
    v3 qvec = Cross(tvec, e1);
    f32 v = Dot(rd, qvec) * invDet;
    if (v < 0.0f || (u + v) > 1.0f) {
        return {false, 0};
    }
    return { true, Dot(e2, qvec) * invDet };
}
