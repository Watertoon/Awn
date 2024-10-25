#include <vp.hpp>

namespace vp::util {

    namespace {
        template <typename T>
            requires (T::cIsVectorType)
        ALWAYS_INLINE bool ParseByamlVectorArrayImpl(T *vector, vp::res::ByamlIterator iter) {
            T vec;
            const bool result0 = iter.TryGetValueByIndex(std::addressof(vec.x), 0);
            if (!result0) { return false; }
            if constexpr (1 < T::cVectorCount) {
                const bool result1 = iter.TryGetValueByIndex(std::addressof(vec.y), 1);
                if (!result1) { return false; }
            }
            if constexpr (2 < T::cVectorCount) {
                const bool result2 = iter.TryGetValueByIndex(std::addressof(vec.z), 2);
                if (!result2) { return false; }
            }
            if constexpr (3 < T::cVectorCount) {
                const bool result3 = iter.TryGetValueByIndex(std::addressof(vec.w), 3);
                if (!result3) { return false; }
            }
            *vector = vec;
            return true;
        }
        template <typename T>
            requires (T::cIsVectorType)
        ALWAYS_INLINE bool ParseByamlVectorDictionaryImpl(T *vector, vp::res::ByamlIterator iter) {
            T vec;
            const bool result0 = iter.TryGetValueByKey(std::addressof(vec.x), "X");
            if (!result0 && !iter.TryGetValueByKey(std::addressof(vec.x), "x")) { return false; }
            if constexpr (1 < T::cVectorCount) {
                const bool result1 = iter.TryGetValueByKey(std::addressof(vec.y), "Y");
                if (!result1 && !iter.TryGetValueByKey(std::addressof(vec.y), "y")) { return false; }
            }
            if constexpr (2 < T::cVectorCount) {
                const bool result2 = iter.TryGetValueByKey(std::addressof(vec.z), "Z");
                if (!result2 && !iter.TryGetValueByKey(std::addressof(vec.z), "z")) { return false; }
            }
            if constexpr (3 < T::cVectorCount) {
                const bool result3 = iter.TryGetValueByKey(std::addressof(vec.w), "W");
                if (!result3 && !iter.TryGetValueByKey(std::addressof(vec.w), "w")) { return false; }
            }
            *vector = vec;
            return true;
        }
        template <typename T>
            requires (T::cIsVectorType)
        ALWAYS_INLINE bool ParseByamlVectorDictionaryUpperCaseImpl(T *vector, vp::res::ByamlIterator iter) {
            T vec;
            const bool result0 = iter.TryGetValueByKey(std::addressof(vec.x), "X");
            if (!result0) { return false; }
            if constexpr (1 < T::cVectorCount) {
                const bool result1 = iter.TryGetValueByKey(std::addressof(vec.y), "Y");
                if (!result1) { return false; }
            }
            if constexpr (2 < T::cVectorCount) {
                const bool result2 = iter.TryGetValueByKey(std::addressof(vec.z), "Z");
                if (!result2) { return false; }
            }
            if constexpr (3 < T::cVectorCount) {
                const bool result3 = iter.TryGetValueByKey(std::addressof(vec.w), "W");
                if (!result3) { return false; }
            }
            *vector = vec;
            return true;
        }
        template <typename T>
            requires (T::cIsVectorType)
        ALWAYS_INLINE bool ParseByamlVectorDictionaryLowerCaseImpl(T *vector, vp::res::ByamlIterator iter) {
            T vec;
            const bool result0 = iter.TryGetValueByKey(std::addressof(vec.x), "x");
            if (!result0) { return false; }
            if constexpr (1 < T::cVectorCount) {
                const bool result1 = iter.TryGetValueByKey(std::addressof(vec.y), "y");
                if (!result1) { return false; }
            }
            if constexpr (2 < T::cVectorCount) {
                const bool result2 = iter.TryGetValueByKey(std::addressof(vec.z), "z");
                if (!result2) { return false; }
            }
            if constexpr (3 < T::cVectorCount) {
                const bool result3 = iter.TryGetValueByKey(std::addressof(vec.w), "w");
                if (!result3) { return false; }
            }
            *vector = vec;
            return true;
        }
    }

	bool Float2::ApplyByamlArray(vp::res::ByamlIterator iter) { return ParseByamlVectorArrayImpl(this, iter); }
	bool Float2::ApplyByamlDictionary(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryImpl(this, iter); }
	bool Float2::ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryUpperCaseImpl(this, iter); }
	bool Float2::ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryLowerCaseImpl(this, iter); }

	bool Float3::ApplyByamlArray(vp::res::ByamlIterator iter) { return ParseByamlVectorArrayImpl(this, iter); }
	bool Float3::ApplyByamlDictionary(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryImpl(this, iter); }
	bool Float3::ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryUpperCaseImpl(this, iter); }
	bool Float3::ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryLowerCaseImpl(this, iter); }

	bool Float4::ApplyByamlArray(vp::res::ByamlIterator iter) { return ParseByamlVectorArrayImpl(this, iter); }
	bool Float4::ApplyByamlDictionary(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryImpl(this, iter); }
	bool Float4::ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryUpperCaseImpl(this, iter); }
	bool Float4::ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryLowerCaseImpl(this, iter); }

	bool Vector2f::ApplyByamlArray(vp::res::ByamlIterator iter) { return ParseByamlVectorArrayImpl(this, iter); }
	bool Vector2f::ApplyByamlDictionary(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryImpl(this, iter); }
	bool Vector2f::ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryUpperCaseImpl(this, iter); }
	bool Vector2f::ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryLowerCaseImpl(this, iter); }

	bool Vector3f::ApplyByamlArray(vp::res::ByamlIterator iter) { return ParseByamlVectorArrayImpl(this, iter); }
	bool Vector3f::ApplyByamlDictionary(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryImpl(this, iter); }
	bool Vector3f::ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryUpperCaseImpl(this, iter); }
	bool Vector3f::ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryLowerCaseImpl(this, iter); }

	bool Vector4f::ApplyByamlArray(vp::res::ByamlIterator iter) { return ParseByamlVectorArrayImpl(this, iter); }
	bool Vector4f::ApplyByamlDictionary(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryImpl(this, iter); }
	bool Vector4f::ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryUpperCaseImpl(this, iter); }
	bool Vector4f::ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter) { return ParseByamlVectorDictionaryLowerCaseImpl(this, iter); }
}
