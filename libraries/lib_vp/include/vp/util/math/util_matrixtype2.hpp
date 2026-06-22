#pragma once

namespace vp::util {

    #define VP_MATRIX_TRAITS(type, row_count, column_count, is_row_major, is_vectorized) \
        using UnderlyingType = type ; \
        static constexpr bool cIsMatrixType = true; \
        static constexpr bool cIsVectorized = is_vectorized ; \
        static constexpr bool cIsRowMajor = is_row_major ; \
        static constexpr bool cIsColumnMajor = ! is_row_major ; \
        static constexpr u32 cRowCount = row_count ; \
        static constexpr u32 cColumnCount = column_count ; \
        static constexpr u32 cVectorizedRowCount = (is_vectorized && cIsColumnMajor) ? vp::util::AlignPow2(cRowCount) : cRowCount; \
        static constexpr u32 cVectorizedColumnCount = (is_vectorized && cIsRowMajor) ? vp::util::AlignPow2(cColumnCount) : cColumnCount; \
        template <class Self> \
            requires (cIsRowMajor) \
        constexpr auto &&operator[](this Self&& self, u32 row, u32 column) { \
            const u32 index = column + row * cVectorizedColumnCount; \
            return std::forward<Self>(self).m[index]; \
        } \
        template <class Self> \
            requires (cIsColumnMajor && !(cIsVectorized && cRowCount == 4 && cColumnCount == 3)) \
        constexpr auto &&operator[](this Self&& self, u32 row, u32 column) { \
            const u32 index = row + column * cVectorizedRowCount; \
            return std::forward<Self>(self).m[index]; \
        } \
        template <class Self> \
            requires (cIsColumnMajor && (cIsVectorized && cRowCount == 4 && cColumnCount == 3)) \
        constexpr auto &&operator[](this Self&& self, u32 row, u32 column) { \
            return std::forward<Self>(self).m[column][row]; \
        }

    template <typename T, u32 RowCount, u32 ColCount, bool IsRowMajor>
    struct MatrixPlain {
        T m[RowCount*ColCount];

        VP_MATRIX_TRAITS(T, RowCount, ColCount, IsRowMajor, false);
    };
    template <typename T, u32 RowCount, u32 ColCount, bool IsRowMajor>
    struct MatrixVector {
        typedef T VectorizedType __attribute__((vector_size(sizeof(T)*vp::util::AlignPow2(RowCount*ColCount))));
        VectorizedType m;

        VP_MATRIX_TRAITS(T, RowCount, ColCount, IsRowMajor, true);
    };
    template <typename T>
    struct MatrixVector<T,4,3,false> {
        typedef T VectorizedType __attribute__((vector_size(sizeof(T)*4)));
        T m[3] __attribute__((vector_size(sizeof(T)*4)));

        VP_MATRIX_TRAITS(T, 4, 3, false, true);
    };

    namespace impl {

        template <typename T, u32 RowCount, u32 ColumnCount, bool IsRowMajor, bool IsVectorized>
        struct MatrixHelper;

        template <typename T, u32 RowCount, u32 ColumnCount, bool IsRowMajor> 
        struct MatrixHelper<T,RowCount,ColumnCount,IsRowMajor, true> {
            using Type = MatrixVector<T,RowCount,ColumnCount,IsRowMajor>;
        };
        template <typename T, u32 RowCount, u32 ColumnCount, bool IsRowMajor> 
        struct MatrixHelper<T,RowCount,ColumnCount,IsRowMajor, false> {
            using Type = MatrixPlain<T,RowCount,ColumnCount,IsRowMajor>;
        };
    }

    template <typename T, u32 RowCount, u32 ColumnCount, bool IsRowMajor, bool IsVectorized> 
    using TMatrix = impl::MatrixHelper<T,RowCount,ColumnCount,IsRowMajor,IsVectorized>::Type;

    namespace impl {

        template <typename T>
            requires (!T::cIsVectorized)
        consteval T MakeIdentity() {
            T ret = {};
            for (u32 i = 0; i < T::cRowCount; ++i) {
                for (u32 j = 0; j < T::cColumnCount; ++j) {
                    ret[i,j] = (i == j) ? static_cast<typename T::UnderlyingType>(1) : static_cast<typename T::UnderlyingType>(0);
                }
            }
            return ret;
        }
        template <typename T>
            requires (T::cIsVectorized && !(T::cIsColumnMajor && T::cRowCount == 4 && T::cColumnCount == 3))
        consteval T MakeIdentity() {
            constexpr T ret = {
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1,
            };
            return ret;
        }
        template <typename T>
            requires (T::cIsColumnMajor && (T::cIsVectorized && T::cRowCount == 4 && T::cColumnCount == 3))
        consteval T MakeIdentity() {
            constexpr T ret = {
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
            };
            return ret;
        }
    }

    template <typename T>
        requires (T::cIsMatrixType)
    struct MatrixConstant {
        static constexpr const T cIdentity = impl::MakeIdentity<T>();
    };

    using FloatRowMajor3x3 = MatrixPlain<float,3,3,true>;
    using FloatRowMajor4x3 = MatrixPlain<float,4,3,true>;
    using FloatRowMajor4x4 = MatrixPlain<float,4,4,true>;
    using MatrixRowMajor4x3f = MatrixVector<float,4,3,true>;
    using MatrixRowMajor4x4f = MatrixVector<float,4,4,true>;

    using FloatColumnMajor3x3 = MatrixPlain<float,3,3,false>;
    using FloatColumnMajor4x3 = MatrixPlain<float,4,3,false>;
    using FloatColumnMajor4x4 = MatrixPlain<float,4,4,false>;
    using MatrixColumnMajor4x3f = MatrixVector<float,4,3,false>;
    using MatrixColumnMajor4x4f = MatrixVector<float,4,4,false>;

    using DoubleRowMajor3x3 = MatrixPlain<double,3,3,true>;
    using DoubleRowMajor4x3 = MatrixPlain<double,4,3,true>;
    using DoubleRowMajor4x4 = MatrixPlain<double,4,4,true>;
    using MatrixRowMajor4x3d = MatrixVector<double,4,3,true>;
    using MatrixRowMajor4x4d = MatrixVector<double,4,4,true>;

    using DoubleColumnMajor3x3 = MatrixPlain<double,3,3,false>;
    using DoubleColumnMajor4x3 = MatrixPlain<double,4,3,false>;
    using DoubleColumnMajor4x4 = MatrixPlain<double,4,4,false>;
    using MatrixColumnMajor4x3d = MatrixVector<double,4,3,false>;
    using MatrixColumnMajor4x4d = MatrixVector<double,4,4,false>;

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && T::cIsMatrixType)
    constexpr Ret Transpose(const T &matrix) {
        constexpr u32 cRealRowCount = vp::util::Min(T::cColumnCount, Ret::cRowCount);
        constexpr u32 cRealColumnCount = vp::util::Min(T::cRowCount, Ret::cColumnCount);
        Ret ret = {};
        for (u32 i = 0 ; i < cRealRowCount; ++i) {
            for (u32 j = 0 ; j < cRealColumnCount; ++j) {
                ret[i,j] = matrix[j,i];
            }
        }
        return ret;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && T::cIsMatrixType)
    constexpr Ret ConvertMajor(const T &matrix) {
        constexpr u32 cRealRowCount = vp::util::Min(T::cVectorizedRowCount, Ret::cVectorizedRowCount);
        constexpr u32 cRealColumnCount = vp::util::Min(T::cVectorizedColumnCount, Ret::cVectorizedColumnCount);
        Ret ret = {};
        for (u32 i = 0 ; i < cRealRowCount; ++i) {
            for (u32 j = 0 ; j < cRealColumnCount; ++j) {
                ret[i,j] = matrix[i,j];
            }
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsMatrixType && (T::cRowCount == 3 || T::cRowCount == 4))
    constexpr TVector<typename T::UnderlyingType, T::cRowCount, T::cIsVectorized> LoadColumn(const T &matrix, u32 col_index) {
        using V = TVector<typename T::UnderlyingType, T::cRowCount, T::cIsVectorized>;
        V ret = {};
        for (u32 i = 0; i < T::cVectorizedRowCount; ++i) {
            ret[i] = matrix[i,col_index];
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsMatrixType && (T::cColumnCount == 3 || T::cColumnCount == 4))
    constexpr TVector<typename T::UnderlyingType, T::cColumnCount, T::cIsVectorized> LoadRow(const T &matrix, u32 row_index) {
        using V = TVector<typename T::UnderlyingType, T::cColumnCount, T::cIsVectorized>;
        V ret = {};
        for (u32 i = 0; i < T::cVectorizedColumnCount; ++i) {
            ret[i] = matrix[row_index,i];
        }
        return ret;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cRowCount == 3 || Ret::cRowCount == 4)) && (T::cIsVectorType)
    constexpr void StoreColumn(Ret *matrix, const T &row, u32 col_index) {
        /* Use a vector store if row are contiguous in memory */
        if constexpr (Ret::cIsColumnMajor && (T::cVectorizedCount <= Ret::cVectorizedRowCount)) {
            Store(std::addressof((*matrix)[0,col_index]), row);
            return;
        }
        /* Row major fallback */
        constexpr const u32 min = vp::util::Min(T::cVectorizedCount, Ret::cVectorizedRowCount);
        for (u32 i = 0; i < min; ++i) {
            (*matrix)[i,col_index] = row[i];
        }
        return;
    }
    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cColumnCount == 3 || Ret::cColumnCount == 4)) && (T::cIsVectorType)
    constexpr void StoreRow(Ret *matrix, const T &col, u32 row_index) {
        /* Use a vector store if columns are contiguous in memory */
        if constexpr (Ret::cIsRowMajor && (T::cVectorizedCount <= Ret::cVectorizedColumnCount)) {
            Store(std::addressof((*matrix)[row_index,0]), col);
            return;
        }
        /* Column major fallback */
        constexpr const u32 min = vp::util::Min(T::cVectorizedCount, Ret::cVectorizedColumnCount);
        for (u32 i = 0; i < min; ++i) {
            (*matrix)[row_index,i] = col[i];
        }
        return;
    }

    template <typename Ret, typename T0, typename T1>
    constexpr Ret MatrixMultiply(const T0 &lhs_matrix, const T1 &rhs_matrix) {

        //using Type = typename T0::UnderlyingType;
        if (!std::is_constant_evaluated()) {
        if constexpr (T0::cIsVectorized && T0::cIsRowMajor && T1::cIsRowMajor) {
            using Type = typename T0::UnderlyingType;
            using V0 = TVector<Type, T0::cColumnCount, T0::cIsVectorized>;
            using V1 = TVector<Type, T1::cColumnCount, T1::cIsVectorized>;
            V0 l_row_array[Ret::cVectorizedRowCount];
            V1 r_row_array[Ret::cVectorizedRowCount];
            for (u32 i = 0; i < T0::cRowCount; ++i) {
                l_row_array[i] = LoadRow(lhs_matrix, i);
            }
            for (u32 i = T0::cRowCount; i < Ret::cVectorizedRowCount; ++i) {
                l_row_array[i] = V0{};
                l_row_array[i][i] = static_cast<Type>(1);
            }
            for (u32 i = 0; i < T1::cRowCount; ++i) {
                r_row_array[i] = LoadRow(rhs_matrix, i);
            }
            for (u32 i = T1::cRowCount; i < Ret::cVectorizedRowCount; ++i) {
                r_row_array[i] = V1{};
                r_row_array[i][i] = static_cast<Type>(1);
            }
#ifdef VP_TARGET_ARCHITECTURE_aarch64
            const auto row_lambda = [=](const V1 &r_row) {
                V0 rowi_mul = V0 { reinterpret_cast<V0::VectorizedType>(vmulq_laneq_f32(reinterpret_cast<float32x4_t>(l_row_array[0].v), reinterpret_cast<float32x4_t>(r_row.v), 0)) };
                for (u32 i = 1; i < T1::cColumnCount; ++i) {
                    rowi_mul = V0 { reinterpret_cast<V0::VectorizedType>(vfmaq_laneq_f32(reinterpret_cast<float32x4_t>(rowi_mul.v), reinterpret_cast<float32x4_t>(l_row_array[i].v), reinterpret_cast<float32x4_t>(r_row.v), i)), };
                }
                return rowi_mul;
            };
#elifdef VP_TARGET_ARCHITECTURE_x86
            const auto row_lambda = [=](const V1 &r_row) {
                const V0 row0_brod0 = Broadcast<V0>(r_row[0]);
                V0 rowi_mul = row0_brod0 * l_row_array[0];
                for (u32 i = 1; i < T1::cColumnCount; ++i) {
                    const V0 rowi_brod = Broadcast<V0>(r_row[i]);
                    rowi_mul = rowi_mul + rowi_brod * l_row_array[i];
                }
                return rowi_mul;
            };
#endif
            V0 r[Ret::cVectorizedRowCount];
            for (u32 i = 0; i < Ret::cVectorizedRowCount; ++i) {
                r[i] = (i == 3 && T1::cColumnCount == 3 && T1::cVectorizedColumnCount == 4) ? row_lambda(r_row_array[i]) + l_row_array[3] : row_lambda(r_row_array[i]);
            }
            Ret ret = {};
            for (u32 i = 0; i < Ret::cVectorizedRowCount; ++i) {
                StoreRow(std::addressof(ret), r[i], i);
            }
            return ret;
        } else if constexpr (T0::cIsVectorized && T0::cIsColumnMajor && T1::cIsColumnMajor) {
            using Type = typename T0::UnderlyingType;
            using V0 = TVector<Type, T0::cRowCount, T0::cIsVectorized>;
            using V1 = TVector<Type, T1::cRowCount, T1::cIsVectorized>;
            V0 l_col_array[Ret::cVectorizedColumnCount];
            V1 r_col_array[Ret::cVectorizedColumnCount];
            for (u32 i = 0; i < T0::cColumnCount; ++i) {
                l_col_array[i] = LoadColumn(lhs_matrix, i);
            }
            for (u32 i = T0::cColumnCount; i < Ret::cVectorizedColumnCount; ++i) {
                l_col_array[i] = V0{};
                l_col_array[i][i] = static_cast<Type>(1);
            }
            for (u32 i = 0; i < T1::cColumnCount; ++i) {
                r_col_array[i] = LoadColumn(rhs_matrix, i);
            }
            for (u32 i = T1::cColumnCount; i < Ret::cVectorizedColumnCount; ++i) {
                r_col_array[i] = V1{};
                r_col_array[i][i] = static_cast<Type>(1);
            }

#ifdef VP_TARGET_ARCHITECTURE_aarch64
            const auto col_lambda = [=](const V1 &r_col) {
                V0 coli_mul = V0 { reinterpret_cast<V0::VectorizedType>(vmulq_laneq_f32(reinterpret_cast<float32x4_t>(l_col_array[0].v), reinterpret_cast<float32x4_t>(r_col.v), 0)) };
                for (u32 i = 1; i < T1::cColumnCount; ++i) {
                    coli_mul = V0 { reinterpret_cast<V0::VectorizedType>(vfmaq_laneq_f32(reinterpret_cast<float32x4_t>(coli_mul.v), reinterpret_cast<float32x4_t>(l_col_array[i].v), reinterpret_cast<float32x4_t>(r_col.v), i)), };
                }
                return coli_mul;
            };
#elifdef VP_TARGET_ARCHITECTURE_x86
            const auto col_lambda = [=](const V1 &r_col) {
                const V0 col0_brod0 = Broadcast<V0>(r_col[0]);
                V0 coli_mul  = col0_brod0 * r_col_array[0];
                for (u32 i = 1; i < T1::cColumnCount; ++i) {
                    const V0 coli_brod = Broadcast<V0>(r_col[i]);
                    coli_mul = coli_mul + coli_brod * r_col_array[i];
                }
                return coli_mul;
            };
#endif
            V0 r[Ret::cVectorizedRowCount];
            for (u32 i = 0; i < Ret::cVectorizedRowCount; ++i) {
                r[i] = col_lambda(r_col_array[i]);
            }
            Ret ret = {};
            for (u32 i = 0; i < Ret::cVectorizedColumnCount; ++i) {
                StoreColumn(std::addressof(ret), r[i], i);
            }
            return ret;
        }
        }
        Ret ret = {};
        for (u32 i = 0; i < Min(Ret::cVectorizedColumnCount,T0::cVectorizedColumnCount); ++i) {
            for (u32 j = 0; j < Ret::cVectorizedRowCount; ++j) {
                typename T0::UnderlyingType product = {};
                for (u32 k = 0; k < T1::cVectorizedColumnCount; ++k) {
                    const typename T0::UnderlyingType p = lhs_matrix[k, i] * rhs_matrix[j, k];
                    product += p;
                }
                ret[j, i] = product;
            }
        }
        return ret;
    }

    template <typename M>
        requires (M::cRowCount <= 4 && M::cColumnCount <= 4)
    constexpr typename M::UnderlyingType MatrixDeterminant(const M &mtx) {
        using Type = typename M::UnderlyingType;

        constexpr u32 min_square = Min(M::cRowCount, M::cColumnCount);
        if constexpr (min_square == 3) {
            const Type det = mtx[0,0] * ((mtx[1,1] * mtx[2,2]) - (mtx[1,2] * mtx[2,1]))
                           + mtx[1,0] * ((mtx[2,1] * mtx[0,2]) - (mtx[2,2] * mtx[0,1]))
                           + mtx[2,0] * ((mtx[1,2] * mtx[0,1]) - (mtx[1,1] * mtx[0,2]));
            return det;
        } else if constexpr (min_square == 4) {
            const Type det = (mtx[0,0] * ((mtx[3,1] * mtx[1,2] * mtx[2,3]) + (mtx[2,1] * mtx[3,2] * mtx[1,3]) + (mtx[1,1] * mtx[2,2] * mtx[3,3]))
                           +  mtx[1,0] * ((mtx[3,1] * mtx[2,2] * mtx[0,3]) + (mtx[0,1] * mtx[3,2] * mtx[2,3]) + (mtx[2,1] * mtx[0,2] * mtx[3,3]))
                           +  mtx[2,0] * ((mtx[3,1] * mtx[0,2] * mtx[1,3]) + (mtx[1,1] * mtx[3,2] * mtx[0,3]) + (mtx[0,1] * mtx[1,2] * mtx[3,3]))
                           +  mtx[3,0] * ((mtx[2,1] * mtx[1,2] * mtx[0,3]) + (mtx[0,1] * mtx[2,2] * mtx[1,3]) + (mtx[0,2] * mtx[1,1] * mtx[2,3])))

                           -  mtx[0,0] * ((mtx[3,1] * mtx[2,2] * mtx[1,3]) + (mtx[1,1] * mtx[3,2] * mtx[2,3]) + (mtx[2,1] * mtx[1,2] * mtx[3,3]))
                           -  mtx[1,0] * ((mtx[3,1] * mtx[0,2] * mtx[2,3]) + (mtx[2,1] * mtx[3,2] * mtx[0,3]) + (mtx[0,1] * mtx[2,2] * mtx[3,3]))
                           -  mtx[2,0] * ((mtx[3,1] * mtx[1,2] * mtx[0,3]) + (mtx[0,1] * mtx[3,2] * mtx[2,3]) + (mtx[0,2] * mtx[1,1] * mtx[3,3]))
                           -  mtx[3,0] * ((mtx[2,1] * mtx[0,2] * mtx[1,3]) + (mtx[1,1] * mtx[2,2] * mtx[0,3]) + (mtx[0,1] * mtx[1,2] * mtx[2,3]));
            return det;
        }
    }

    template <typename M>
        requires (M::cRowCount <= 4 && M::cColumnCount <= 4)
    constexpr M MatrixInverse(const M &mtx, bool *out_is_singular) {
        using Type = typename M::UnderlyingType;

        const Type determinant = MatrixDeterminant(mtx);
        const bool is_singular = (determinant == 0.0);
        if (out_is_singular != nullptr) { *out_is_singular = is_singular; }
        if (is_singular) { return mtx; }
        const Type inv_det = static_cast<Type>(1.0) / determinant;
        constexpr u32 min_square = Min(M::cRowCount, M::cColumnCount);
        M ret = {};
        if constexpr (min_square == 3) {

            const Type ret00 =  ((mtx[1,1] * mtx[2,2]) - (mtx[1,2] * mtx[2,1])) * inv_det;
            const Type ret10 = -((mtx[2,2] * mtx[1,0]) - (mtx[1,2] * mtx[2,0])) * inv_det;
            const Type ret20 =  ((mtx[2,1] * mtx[1,0]) - (mtx[1,1] * mtx[2,0])) * inv_det;

            const Type ret01 = -((mtx[0,1] * mtx[2,2]) - (mtx[0,2] * mtx[2,1])) * inv_det;
            const Type ret11 =  ((mtx[0,0] * mtx[2,2]) - (mtx[0,2] * mtx[2,0])) * inv_det;
            const Type ret21 = -((mtx[0,0] * mtx[2,1]) - (mtx[0,1] * mtx[2,0])) * inv_det;

            const Type ret02 =  ((mtx[1,2] * mtx[0,1]) - (mtx[1,1] * mtx[0,2])) * inv_det;
            const Type ret12 = -((mtx[1,2] * mtx[0,0]) - (mtx[1,0] * mtx[0,2])) * inv_det;
            const Type ret22 =  ((mtx[1,1] * mtx[0,0]) - (mtx[1,0] * mtx[0,1])) * inv_det;

            ret[0,0] = ret00;
            ret[1,0] = ret10;
            ret[2,0] = ret20;
            ret[0,1] = ret01;
            ret[1,1] = ret11;
            ret[2,1] = ret21;
            ret[0,2] = ret02;
            ret[1,2] = ret12;
            ret[2,2] = ret22;
            if constexpr (M::cRowCount == 4) {
                const Type ret30 = ((-ret00 * mtx[3,0]) - ret10 * mtx[3,1]) - ret20 * mtx[3,2];
                const Type ret31 = ((-ret01 * mtx[3,0]) - ret11 * mtx[3,1]) - ret21 * mtx[3,2];
                const Type ret32 = ((-ret02 * mtx[3,0]) - ret12 * mtx[3,1]) - ret22 * mtx[3,2];
                ret[3,0] = ret30;
                ret[3,1] = ret31;
                ret[3,2] = ret32;
            }
            if constexpr (M::cIsRowMajor && M::cIsVectorized) {
                ret[0,3] = 0.0f;
                ret[1,3] = 0.0f;
                ret[2,3] = 0.0f;
                ret[3,3] = 1.0f;
            }

        } else if constexpr (min_square == 4) {

            const Type ret00 = ((mtx[1,1] * ((mtx[2,2] * mtx[3,3]) - (mtx[3,2] * mtx[2,3])) - mtx[2,1] * ((mtx[1,2] * mtx[3,3]) - (mtx[3,2] * mtx[1,3]))) - mtx[3,1] * ((mtx[2,2] * mtx[1,3]) - (mtx[1,2] * mtx[2,3]))) * inv_det;
            const Type ret10 = ((mtx[3,0] * ((mtx[2,2] * mtx[1,3]) - (mtx[1,2] * mtx[2,3])) + mtx[2,0] * ((mtx[1,2] * mtx[3,3]) - (mtx[3,2] * mtx[1,3]))) - mtx[1,0] * ((mtx[2,2] * mtx[3,3]) - (mtx[3,2] * mtx[2,3]))) * inv_det;
            const Type ret20 = ((mtx[3,0] * ((mtx[2,3] * mtx[1,1]) - (mtx[1,3] * mtx[2,1])) + mtx[1,0] * ((mtx[2,1] * mtx[3,3]) - (mtx[3,1] * mtx[2,3]))) + mtx[2,0] * ((mtx[1,3] * mtx[3,1]) - (mtx[1,1] * mtx[3,3]))) * inv_det;
            const Type ret30 = ((mtx[3,0] * ((mtx[2,1] * mtx[1,2]) - (mtx[1,1] * mtx[2,2])) + mtx[1,0] * ((mtx[2,2] * mtx[3,1]) - (mtx[3,2] * mtx[2,1]))) + mtx[2,0] * ((mtx[1,1] * mtx[3,2]) - (mtx[1,2] * mtx[3,1]))) * inv_det;

            const Type ret01 = ((mtx[3,1] * ((mtx[2,2] * mtx[0,3]) - (mtx[0,2] * mtx[2,3])) + mtx[2,1] * ((mtx[0,2] * mtx[3,3]) - (mtx[3,2] * mtx[0,3]))) - mtx[0,1] * ((mtx[2,2] * mtx[3,3]) - (mtx[3,2] * mtx[2,3]))) * inv_det;
            const Type ret11 = ((mtx[0,0] * ((mtx[2,2] * mtx[3,3]) - (mtx[3,2] * mtx[2,3])) - mtx[2,0] * ((mtx[0,2] * mtx[3,3]) - (mtx[3,2] * mtx[0,3]))) - mtx[3,0] * ((mtx[2,2] * mtx[0,3]) - (mtx[0,2] * mtx[2,3]))) * inv_det;
            const Type ret21 = ((mtx[3,0] * ((mtx[2,1] * mtx[0,3]) - (mtx[0,1] * mtx[2,3])) + mtx[2,0] * ((mtx[0,1] * mtx[3,3]) - (mtx[3,1] * mtx[0,3]))) - mtx[0,0] * ((mtx[2,1] * mtx[3,3]) - (mtx[3,1] * mtx[2,3]))) * inv_det;
            const Type ret31 = ((mtx[3,0] * ((mtx[2,2] * mtx[0,1]) - (mtx[2,1] * mtx[0,2])) + mtx[2,0] * ((mtx[0,2] * mtx[3,1]) - (mtx[3,2] * mtx[0,1]))) - mtx[0,0] * ((mtx[2,2] * mtx[3,1]) - (mtx[3,2] * mtx[2,1]))) * inv_det;

            const Type ret02 = ((mtx[0,1] * ((mtx[3,3] * mtx[1,2]) - (mtx[3,2] * mtx[1,3])) - mtx[1,1] * ((mtx[0,2] * mtx[3,3]) - (mtx[3,2] * mtx[0,3]))) - mtx[3,1] * ((mtx[0,3] * mtx[1,2]) - (mtx[0,2] * mtx[1,3]))) * inv_det;
            const Type ret12 = ((mtx[3,0] * ((mtx[0,3] * mtx[1,2]) - (mtx[0,2] * mtx[1,3])) + mtx[1,0] * ((mtx[0,2] * mtx[3,3]) - (mtx[3,2] * mtx[0,3]))) - mtx[0,0] * ((mtx[1,2] * mtx[3,3]) - (mtx[1,3] * mtx[3,2]))) * inv_det;
            const Type ret22 = ((mtx[3,0] * ((mtx[0,1] * mtx[1,3]) - (mtx[0,3] * mtx[1,1])) - mtx[0,0] * ((mtx[1,3] * mtx[3,1]) - (mtx[3,3] * mtx[1,1]))) - mtx[1,0] * ((mtx[0,1] * mtx[3,3]) - (mtx[0,3] * mtx[3,1]))) * inv_det;
            const Type ret32 = ((mtx[3,0] * ((mtx[0,2] * mtx[1,1]) - (mtx[0,1] * mtx[1,2])) - mtx[0,0] * ((mtx[1,1] * mtx[3,2]) - (mtx[3,1] * mtx[1,2]))) - mtx[1,0] * ((mtx[0,2] * mtx[3,1]) - (mtx[0,1] * mtx[3,2]))) * inv_det;

            const Type ret03 = (  mtx[2,1] * ((mtx[0,3] * mtx[1,2]) - (mtx[0,2] * mtx[1,3])) + (mtx[0,1] * ((mtx[1,3] * mtx[2,2]) - (mtx[1,2] * mtx[2,3]))  - mtx[1,1] * ((mtx[0,3] * mtx[2,2]) - (mtx[0,2] * mtx[2,3])))) * inv_det;
            const Type ret13 = (( mtx[1,0] * ((mtx[0,3] * mtx[2,2]) - (mtx[0,2] * mtx[2,3])) -  mtx[2,0] * ((mtx[1,2] * mtx[0,3]) - (mtx[1,3] * mtx[0,2]))) - mtx[0,0] * ((mtx[1,3] * mtx[2,2]) - (mtx[1,2] * mtx[2,3]))) * inv_det;
            const Type ret23 = ((-mtx[2,0] * ((mtx[0,1] * mtx[1,3]) - (mtx[0,3] * mtx[1,1])) -  mtx[0,0] * ((mtx[1,1] * mtx[2,3]) - (mtx[1,3] * mtx[2,1]))) - mtx[1,0] * ((mtx[0,3] * mtx[2,1]) - (mtx[0,1] * mtx[2,3]))) * inv_det;
            const Type ret33 = ((-mtx[2,0] * ((mtx[0,2] * mtx[1,1]) - (mtx[0,1] * mtx[1,2])) -  mtx[0,0] * ((mtx[1,2] * mtx[2,1]) - (mtx[1,1] * mtx[2,2]))) - mtx[1,0] * ((mtx[0,1] * mtx[2,2]) - (mtx[0,2] * mtx[2,1]))) * inv_det;

            ret[0,0] = ret00;
            ret[1,0] = ret10;
            ret[2,0] = ret20;
            ret[3,0] = ret30;
            ret[0,1] = ret01;
            ret[1,1] = ret11;
            ret[2,1] = ret21;
            ret[3,1] = ret31;
            ret[0,2] = ret02;
            ret[1,2] = ret12;
            ret[2,2] = ret22;
            ret[3,2] = ret32;
            ret[0,3] = ret03;
            ret[1,3] = ret13;
            ret[2,3] = ret23;
            ret[3,3] = ret33;
        }

        return ret;
    }

    template <typename T>
        requires (T::cIsMatrixType)
    constexpr bool operator==(const T &left, const T &right) {
        bool result = true;
        if constexpr (T::cIsRowMajor) {
            for (u32 i = 0; i < T::cVectorizedColumnCount; ++i) {
                for (u32 j = 0; j < T::cVectorizedRowCount; ++j) {
                    result &= (left[j, i] == right[j, i]);
                }
            }
        } else {
            for (u32 i = 0; i < T::cVectorizedRowCount; ++i) {
                for (u32 j = 0; j < T::cVectorizedColumnCount; ++j) {
                    result &= (left[i, j] == right[i, j]);
                }
            }
        }
        return result;
    }

    template <typename Ret, typename M, typename V>
        requires (M::cIsMatrixType && M::cRowCount == 4 && M::cColumnCount == 3) && (V::cIsVectorType && 3 == V::cCount)
    constexpr Ret MatrixTransform(const M &matrix, const V &position) {
        using Type = typename V::UnderlyingType;
        using Vec = TVector<Type,M::cColumnCount,true>;

        Vec rot = {};
        for (u32 i = 0; i < V::cCount; ++i) {
            const Vec b = Broadcast<Vec>(position[i]);
            const Vec row = LoadRow(matrix, i);
            const Vec mul = b * row;
            rot = rot + mul;
        }
        const Vec pos = LoadRow(matrix, 3);
        const Vec acc = pos + rot;
        return Convert<Ret>(acc);
    }

    template <typename T>
        requires (T::cIsMatrixType)
    constexpr T Absolute(const T &m) {
        T ret;
        for (u32 i = 0; i < T::cVectorizedRowCount; ++i) {
            for (u32 j = 0; j < T::cVectorizedColumnCount; ++j) {
                ret[i,j] = Absolute(m[i,j]);
            }
        }
        return ret;
    }

    template <typename M, typename V>
        requires (M::cIsMatrixType && std::is_same_v<typename M::UnderlyingType, typename V::UnderlyingType>)
    constexpr M MatrixMultiplyScalarRow(const M &matrix, const V &vector) {
        //using Type = typename V::UnderlyingType;

        M ret = {};
        constexpr u32 cVectorMin = Min(V::cCount, M::cRowCount);
        for (u32 i = 0; i < V::cCount; ++i) {
            const V b = Broadcast<V>(vector[i]);
            const V row = LoadRow(matrix, i);
            const V mul = b * row;
            StoreRow(std::addressof(ret), mul, i);
        }
        constexpr u32 cLeftover = M::cRowCount - cVectorMin;
        for (u32 i = cVectorMin; i < cLeftover + cVectorMin; ++i) {
            const V row = LoadRow(matrix, i);
            StoreRow(std::addressof(ret), row, i);
        }
        return ret;
    }

    template <typename M>
        requires (M::cIsMatrixType)
    constexpr M MatrixMultiplyScalar(const M &matrix, const typename M::UnderlyingType &scalar) {
        using V = TVector<typename M::UnderlyingType, M::cColumnCount, M::cIsVectorized>;
        M ret = {};
        const V b = Broadcast<V>(scalar);
        for (u32 i = 0; i < M::cRowCount; ++i) {
            const V row = LoadRow(matrix, i);
            const V mul = b * row;
            StoreRow(std::addressof(ret), mul, i);
        }
        return ret;
    }

    template <typename V>
        requires (V::cIsVectorType)
    constexpr TMatrix<typename V::UnderlyingType, V::cCount, V::cCount, true, V::cIsVectorized> MatrixCovariance(const V *array, u32 count, V average) {
        using M = TMatrix<typename V::UnderlyingType, V::cCount, V::cCount, true, V::cIsVectorized>;
        using Type = typename V::UnderlyingType;
        const V mean = average;
        M cm = {};
        for (u32 i = 0; i < count; ++i) {
            const V temp = array[i] - mean;
            for (u32 j = 0; j < M::cRowCount; ++j) {
                const V b = Broadcast<V>(temp[j]);
                const V r0 = LoadRow(cm, j);
                const V r1 = r0 + temp * b;
                StoreRow(std::addressof(cm), r1, j);
            }
        }
        const Type recp = static_cast<Type>(1) / static_cast<Type>(count);
        const M ret = MatrixMultiplyScalar(cm, recp);
        return ret;
    }
    template <typename V>
        requires (V::cIsVectorType)
    constexpr TMatrix<typename V::UnderlyingType, V::cCount, V::cCount, true, V::cIsVectorized> MatrixCovariance(const V *array, u32 count) {
        const V mean = Average(array,count);
        return MatrixCovariance(array,count,mean);
    }
    //template <typename M>
    //    requires (M::cIsMatrixType)
    //constexpr TVector<typename M::UnderlyingType, M::cRowCount> PrincipleAxis(const M &covariance_matrix) {
    //    using V = TVector<typename M::UnderlyingType, M::cRowCount>;
    //    V iter = VectorConstant<V>::cUnit[1];
    //    for () {
    //        const V temp = MatrixMultiplyScalarRow(covariance_matrix, iter);
    //        if (MagnitudeSquared(temp) == 0) { break; }
    //        const V n_temp = Normalize(temp);
    //    }
    //    return iter;
    //}
}
