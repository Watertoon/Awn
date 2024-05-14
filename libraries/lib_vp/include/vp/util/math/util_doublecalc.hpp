#pragma once

namespace vp::util {

    constexpr Vector3d operator+(const Vector3d &left, const Vector3d &right) {
        const Vector3d out = { .vector = left.vector + right.vector };
        return out;
    }
    constexpr Vector3d operator-(const Vector3d &left, const Vector3d &right) {
        const Vector3d out = { .vector = left.vector - right.vector };
        return out;
    }

	constexpr Vector2d CastVector(const Vector2f &vector) {
		const Vector2d out = { static_cast<double>(vector.x), static_cast<double>(vector.y) };
        return out;
	}
	constexpr Vector3d CastVector(const Vector3f &vector) {
		const Vector3d out = { static_cast<double>(vector.x), static_cast<double>(vector.y), static_cast<double>(vector.z) };
        return out;
	}
	constexpr Vector4d CastVector(const Vector4f &vector) {
		const Vector4d out = { static_cast<double>(vector.x), static_cast<double>(vector.y), static_cast<double>(vector.z), static_cast<double>(vector.w) };
        return out;
	}
}
