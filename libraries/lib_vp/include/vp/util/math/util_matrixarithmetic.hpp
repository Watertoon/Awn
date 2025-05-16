/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace vp::util {

    template <typename T>
        requires (T::cIsMatrixType && (T::cRowCount == 3 || T::cRowCount == 4))
    constexpr TVector<typename T::UnderlyingType, T::cRowCount> LoadColumn(const T &matrix, u32 col_index) {
        using VectorType = TVector<typename T::UnderlyingType, T::cRowCount>;
        /* Use a vector load if rows are contiguous in memory */
        if constexpr (T::cIsColumnMajor) {
            /* Load vectorized matrices by aligned load, for non-vector 4 rows use unaligned load, 3 rows use mask load */
            if constexpr (T::cIsVectorized) {
                return VectorType { .v = Load4Aligned(std::addressof(matrix[0,col_index])), };
            } else if constexpr (T::cRowCount == 4) {
                return VectorType { .v = Load4(std::addressof(matrix[0,col_index])), };
            } else if constexpr (sizeof(typename T::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetLoad4Mask128(true, true, true, false);
                return VectorType { .v = Load4Mask128(std::addressof(matrix[0,col_index]), mask), };
            } else if constexpr (sizeof(typename T::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetLoad4Mask256(true, true, true, false);
                return VectorType { .v = Load4Mask256(std::addressof(matrix[0,col_index]), mask), };
            }
        }
        /* Row major fallback */
        using v4t = TVectorType<typename T::UnderlyingType, 4>;
        v4t ret = {};
        for (u32 i = 0; i < T::cRowCount; ++i) {
            ret[i] = matrix[i,col_index];
        }
        return VectorType { .v = ret, };
    }

    template <typename T>
        requires (T::cIsMatrixType && (T::cColumnCount == 3 || T::cColumnCount == 4))
    constexpr TVector<typename T::UnderlyingType, T::cColumnCount> LoadRow(const T &matrix, u32 row_index) {
        using VectorType = TVector<typename T::UnderlyingType, T::cColumnCount>;
        /* Use a vector load if columns are contiguous in memory */
        if constexpr (T::cIsRowMajor) {
            /* Load vectorized matrices by aligned load, for non-vector 4 rows use unaligned load, 3 rows use mask load */
            if constexpr (T::cIsVectorized) {
                return VectorType{ .v = Load4Aligned(std::addressof(matrix[row_index,0])), };
            } else if constexpr (T::cColumnCount == 4) {
                return VectorType{ .v = Load4(std::addressof(matrix[row_index,0])), };
            } else if constexpr (T::cColumnCount == 3 && sizeof(typename T::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetLoad4Mask128(true, true, true, false);
                return VectorType{ .v = Load4Mask128(std::addressof(matrix[row_index,0]), mask), };
            } else if constexpr (T::cColumnCount == 3 && sizeof(typename T::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetLoad4Mask256(true, true, true, false);
                return VectorType{ .v = Load4Mask256(std::addressof(matrix[row_index,0]), mask), };
            }
        }
        /* Column major fallback */
        using v4t = TVectorType<typename T::UnderlyingType, 4>;
        v4t ret = {};
        for (u32 i = 0; i < T::cColumnCount; ++i) {
            ret[i] = matrix[row_index,i];
        }
        return VectorType { .v = ret, };
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cRowCount == 3 || Ret::cRowCount == 4)) && (T::cIsVectorizedType && Ret::cRowCount <= T::cCount)
    constexpr void StoreColumn(Ret *matrix, const T &row, u32 col_index) {
        /* Use a vector store if row are contiguous in memory */
        if constexpr (Ret::cIsColumnMajor) {
            /* Write column to vectorized matrices by aligned store, for non-vector 4 cols use unaligned store, 3 cols use mask store */
            if constexpr (Ret::cIsVectorized) {
                Store4Aligned(std::addressof((*matrix)[0,col_index]), std::addressof(row));
                return;
            } else if constexpr (Ret::cRowCount == 4) {
                Store4(std::addressof((*matrix)[0,col_index]), std::addressof(row));
                return;
            }  else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetStore4Mask128(true, true, true, false);
                Store4Mask128(std::addressof((*matrix)[0,col_index]), std::addressof(row), mask);
                return;
            } else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetStore4Mask256(true, true, true, false);
                Store4Mask256(std::addressof((*matrix)[0,col_index]), std::addressof(row), mask);
                return;
            }
        }
        /* Row major fallback */
        for (u32 i = 0; i < Ret::cRowCount; ++i) {
            (*matrix)[i,col_index] = row[i];
        }
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cColumnCount == 3 || Ret::cColumnCount == 4)) && (T::cIsVectorizedType && Ret::cColumnCount <= T::cCount)
    constexpr void StoreRow(Ret *matrix, const T &col, u32 row_index) {
        /* Use a vector store if columns are contiguous in memory */
        if constexpr (Ret::cIsRowMajor) {
            /* Write column to vectorized matrices by aligned store, for non-vector 4 cols use unaligned store, 3 cols use mask store */
            if constexpr (Ret::cIsVectorized) {
                Store4Aligned(std::addressof((*matrix)[row_index,0]), std::addressof(col));
                return;
            } else if constexpr (Ret::cColumnCount == 4) {
                Store4(std::addressof((*matrix)[row_index,0]), std::addressof(col));
                return;
            }  else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetStore4Mask128(true, true, true, false);
                Store4Mask128(std::addressof((*matrix)[row_index,0]), std::addressof(col), mask);
                return;
            } else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetStore4Mask256(true, true, true, false);
                Store4Mask256(std::addressof((*matrix)[row_index,0]), std::addressof(col), mask);
                return;
            }
        }
        /* Column major fallback */
        const u32 cRealColCount = (Ret::cIsVectorized) ? 4 : Ret::cColumnCount;
        for (u32 i = 0; i < cRealColCount; ++i) {
            (*matrix)[row_index,i] = col[i];
        }
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cRowCount == 3 || Ret::cRowCount == 4)) && (T::cIsVectorType && Ret::cRowCount <= T::cVectorCount)
    constexpr void StoreColumn(Ret *matrix, const T &row, u32 col_index) {
        /* Use a vector store if row are contiguous in memory */
        if constexpr (Ret::cIsColumnMajor && T::cIsVectorized) {
            /* Write column to vectorized matrices by aligned store, for non-vector 4 cols use unaligned store, 3 cols use mask store */
            if constexpr (Ret::cIsVectorized) {
                Store4Aligned(std::addressof((*matrix)[0,col_index]), std::addressof(row.v));
                return;
            } else if constexpr (Ret::cRowCount == 4) {
                Store4(std::addressof((*matrix)[0,col_index]), std::addressof(row.v));
                return;
            }  else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetStore4Mask128(true, true, true, false);
                Store4Mask128(std::addressof((*matrix)[0,col_index]), std::addressof(row.v), mask);
                return;
            } else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetStore4Mask256(true, true, true, false);
                Store4Mask256(std::addressof((*matrix)[0,col_index]), std::addressof(row.v), mask);
                return;
            }
        }
        /* Row major fallback */
        for (u32 i = 0; i < Ret::cRowCount; ++i) {
            (*matrix)[i,col_index] = row[i];
        }
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cColumnCount == 3 || Ret::cColumnCount == 4)) && (T::cIsVectorType && Ret::cColumnCount <= T::cVectorCount)
    constexpr void StoreRow(Ret *matrix, const T &col, u32 row_index) {
        /* Use a vector store if columns are contiguous in memory */
        if constexpr (Ret::cIsRowMajor && T::cIsVectorized) {
            /* Write column to vectorized matrices by aligned store, for non-vector 4 cols use unaligned store, 3 cols use mask store */
            if constexpr (Ret::cIsVectorized) {
                Store4Aligned(std::addressof((*matrix)[row_index,0]), std::addressof(col.v));
                return;
            } else if constexpr (Ret::cColumnCount == 4) {
                Store4(std::addressof((*matrix)[row_index,0]), std::addressof(col.v));
                return;
            }  else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetStore4Mask128(true, true, true, false);
                Store4Mask128(std::addressof((*matrix)[row_index,0]), std::addressof(col.v), mask);
                return;
            } else if constexpr (sizeof(typename Ret::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetStore4Mask256(true, true, true, false);
                Store4Mask256(std::addressof((*matrix)[row_index,0]), std::addressof(col.v), mask);
                return;
            }
        }
        /* Column major fallback */
        const u32 cRealColCount = Ret::cColumnCount;
        for (u32 i = 0; i < cRealColCount; ++i) {
            (*matrix)[row_index,i] = col[i];
        }
        return;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && T::cIsMatrixType)
    constexpr Ret Transpose(const T &matrix) {
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
#endif
        }

        /* Same major transpose */
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
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
#endif
        }
        constexpr u32 cRealRowCount = vp::util::Min(T::cRowCount, Ret::cRowCount);
        constexpr u32 cRealColumnCount = vp::util::Min(T::cColumnCount, Ret::cColumnCount);
        Ret ret = {};
        for (u32 i = 0 ; i < cRealRowCount; ++i) {
            for (u32 j = 0 ; j < cRealColumnCount; ++j) {
                ret[i,j] = matrix[i,j];
            }
        }
        return ret;
    }

    /* We support result matrices larger than necessary, as in a Matrix44 * Matrix43 would make a Matrix43, but we can use a Matrix44 result */
    /* All underlying types must be identical */
    template <typename Ret, typename T0, typename T1>
        requires (
            Ret::cIsMatrixType && ((T0::cRowCount <= Ret::cColumnCount && T1::cColumnCount <= Ret::cRowCount) || (T0::cIsVectorized))
            && std::is_same_v<typename Ret::UnderlyingType, typename T0::UnderlyingType> && std::is_same_v<typename T0::UnderlyingType, typename T1::UnderlyingType>
        )
    constexpr Ret MatrixMultiply(const T0 &lhs_matrix, const T1 &rhs_matrix) {

#ifdef VP_TARGET_ARCHITECTURE_x86
        if constexpr (T0::cIsRowMajor && (T0::cColumnCount == 4 || T0::cColumnCount == 3) && T0::cRowCount == 4
        && T1::cIsRowMajor && ((T1::cColumnCount == 3  || T1::cColumnCount == 4) && T1::cRowCount == 4)) {

            /* Load all rows */
            using Type = typename T0::UnderlyingType;
            using v4t0 = TVector<Type, T0::cColumnCount>;
            using v4t1 = TVector<Type, T1::cColumnCount>;
            v4t0 l_row_array[Ret::cRowCount];
            for (u32 i = 0; i < T0::cRowCount; ++i) {
                l_row_array[i] = LoadRow(lhs_matrix, i);
            }
            for (u32 i = T0::cRowCount; i < Ret::cRowCount; ++i) {
                l_row_array[i] = v4t0{};
                l_row_array[i][i] = static_cast<Type>(1);
            }
            v4t1 r_row_array[Ret::cRowCount];
            for (u32 i = 0; i < T1::cRowCount; ++i) {
                r_row_array[i] = LoadRow(rhs_matrix, i);
            }
            for (u32 i = T1::cRowCount; i < Ret::cRowCount; ++i) {
                r_row_array[i] = v4t1{};
                r_row_array[i][i] = static_cast<Type>(1);
            }
            if constexpr (T0::cColumnCount == 3) {
                l_row_array[3][3] = static_cast<Type>(1);
            }
            if constexpr (T1::cColumnCount == 3) {
                r_row_array[3][3] = static_cast<Type>(1);
            }

            /* For each result row + column, multiply all elements of left column with elements with all elements of right column */
            const auto row_lambda = [=](const v4t1 &r_row) {
                /* Broadcast column values of left matrix, multiply, and add */
                const v4t0 row0_brod0 = BroadcastVector<Type, Ret::cColumnCount>(r_row[0]);
                v4t0 rowi_mul = row0_brod0 * l_row_array[0];
                for (u32 i = 1; i < Ret::cRowCount; ++i) {
                    const v4t0 rowi_brod = BroadcastVector<Type, Ret::cColumnCount>(r_row[i]);
                    rowi_mul = MultiplyAdd(rowi_brod, l_row_array[i], rowi_mul);
                }
                return rowi_mul;
            };
            if constexpr (Ret::cIsColumnMajor) {
                /* Transpose result for opposite major result matrix */
                TMatrix<typename Ret::UnderlyingType, Ret::cRowCount, Ret::cColumnCount, !Ret::cIsRowMajor> matrix;
                for (u32 i = 0; i < Ret::cRowCount; ++i) {
                    const v4t0 result = row_lambda(r_row_array[i]);
                    StoreRow(std::addressof(matrix), result, i);
                }
                const Ret ret = ConvertMajor<Ret>(matrix);
                return ret;
            }
            Ret ret;
            for (u32 i = 0; i < Ret::cRowCount; ++i) {
                const v4t0 result = row_lambda(r_row_array[i]);
                StoreRow(std::addressof(ret), result, i);
            }
            return ret;
        }
        if constexpr (T0::cIsColumnMajor && (T0::cColumnCount == 4 || T0::cColumnCount == 3) && T0::cRowCount == 4
        && T1::cIsColumnMajor && ((T1::cColumnCount == 3  || T1::cColumnCount == 4) && T1::cRowCount == 4)) {

            /* Load all columns */
            using Type = typename T0::UnderlyingType;
            using v4t0 = TVector<Type, T0::cRowCount>;
            using v4t1 = TVector<Type, T1::cRowCount>;
            v4t0 l_col_array[Ret::cRowCount];
            for (u32 i = 0; i < T0::cColumnCount; ++i) {
                l_col_array[i] = LoadColumn(lhs_matrix, i);
            }
            for (u32 i = T0::cColumnCount; i < Ret::cRowCount; ++i) {
                l_col_array[i] = v4t0{};
                l_col_array[i][i] = static_cast<Type>(1);
            }
            v4t1 r_col_array[Ret::cRowCount];
            for (u32 i = 0; i < T1::cColumnCount; ++i) {
                r_col_array[i] = LoadColumn(rhs_matrix, i);
            }
            for (u32 i = T1::cColumnCount; i < Ret::cRowCount; ++i) {
                r_col_array[i] = v4t1{};
                r_col_array[i][i] = static_cast<Type>(1);
            }

            /* For each result row + column, multiply all elements of left matrix column with elements with all elements of right matrix row */
            const auto col_lambda = [=](const v4t1 &l_col) {
                /* Broadcast row values of right matrix, multiply, and add */
                const v4t0 col0_brod0 = BroadcastVector<Type, Ret::cRowCount>(l_col[0]);
                v4t0 coli_mul  = col0_brod0 * r_col_array[0];
                for (u32 i = 1; i < T1::cRowCount; ++i) {
                    const v4t0 coli_brod = BroadcastVector<Type, Ret::cRowCount>(l_col[i]);
                    coli_mul = MultiplyAdd(coli_brod, r_col_array[i], coli_mul);
                }
                return coli_mul;
            };
            if constexpr (Ret::cIsRowMajor) {
                /* Transpose result for opposite major result matrix */
                TMatrix<typename Ret::UnderlyingType, Ret::cRowCount, Ret::cColumnCount, !Ret::cIsRowMajor> matrix;
                for (u32 i = 0; i < T0::cRowCount; ++i) {
                    const v4t0 result = col_lambda(l_col_array[i]);
                    StoreColumn(std::addressof(matrix), result, i);
                }
                const Ret ret = ConvertMajor<Ret>(matrix);
                return ret;
            }
            Ret ret = {};
            for (u32 i = 0; i < T0::cRowCount; ++i) {
                const v4t0 result = col_lambda(l_col_array[i]);
                StoreColumn(std::addressof(ret), result, i);
            }
            return ret;
        }
#endif
        Ret ret = {};
        for (u32 i = 0; i < T0::cColumnCount; ++i) {
            for (u32 j = 0; j < Ret::cRowCount; ++j) {
                typename T0::UnderlyingType product = 0;
                for (u32 k = 0; k < T1::cColumnCount; ++k) {
                    product += lhs_matrix[k, i] * rhs_matrix[j, k];
                }
                ret[j, i] = product;
            }
        }
        return ret;
    }

    template <typename M, typename V>
        requires (M::cIsMatrixType && std::is_same_v<typename M::UnderlyingType, typename V::UnderlyingType>)
    constexpr M MatrixMultiplyScalarRow(const M &matrix, const V &vector) {
        using Type = typename V::UnderlyingType;

        M ret = {};
        constexpr u32 cVectorMin = Min(V::cVectorCount, M::cRowCount);
        for (u32 i = 0; i < V::cVectorCount; ++i) {
            const V b = BroadcastVector<Type, M::cColumnCount>(vector[i]);
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

    template <typename M, typename V>
        requires (M::cIsMatrixType && std::is_same_v<typename M::UnderlyingType, typename V::UnderlyingType>)
    constexpr M MatrixMultiplyScalarColumn(const M &matrix, const V &vector) {
        using Type = typename V::UnderlyingType;

        M ret = {};
        constexpr u32 cVectorMin = Min(V::cVectorCount, M::cColumnCount);
        for (u32 i = 0; i < V::cVectorCount; ++i) {
            const V b = BroadcastVector<Type, M::cRowCount>(vector[i]);
            const V col = LoadColumn(matrix, i);
            const V mul = b * col;
            StoreColumn(std::addressof(ret), mul, i);
        }
        constexpr u32 cLeftover = M::cColumnCount - cVectorMin;
        for (u32 i = cVectorMin; i < cLeftover + cVectorMin; ++i) {
            const V col = LoadColumn(matrix, i);
            StoreColumn(std::addressof(ret), col, i);
        }
        return ret;
    }

    template <typename Ret, typename M, typename V>
        requires (M::cIsMatrixType && Ret::cIsVectorType && V::cIsVectorType && 
        std::is_same_v<typename M::UnderlyingType, typename V::UnderlyingType> && std::is_same_v<typename Ret::UnderlyingType, typename V::UnderlyingType> && 
        V::cVectorCount == 3 && Ret::cVectorCount == 3 && 4 <= M::cRowCount && 3 <= M::cColumnCount)
    constexpr Ret MatrixTransform(const M &matrix, const V &position) {
        using Type = typename V::UnderlyingType;
        using Vec = TVector<Type, M::cColumnCount, true>;

        Vec rot = {};
        for (u32 i = 0; i < V::cVectorCount; ++i) {
            const Vec b = BroadcastVector<Type, M::cColumnCount>(position[i]);
            const Vec row = LoadRow(matrix, i);
            const Vec mul = b * row;
            rot = rot + mul;
        }
        const Vec pos = LoadRow(matrix, 3);
        const Vec acc = pos + rot;
        Ret ret;
        if constexpr (!Ret::cIsVectorized) {
            ret = vp::util::CastVector<Ret>(acc);
        } else {
            ret = acc;
        }
        return ret;
    }

    template <typename M>
        requires (M::cIsMatrixType && 3 <= M::cRowCount && M::cRowCount <= 4 && 3 <= M::cColumnCount && M::cColumnCount <= 4)
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
        requires (M::cIsMatrixType && 3 <= M::cRowCount && M::cRowCount <= 4 && 3 <= M::cColumnCount && M::cColumnCount <= 4)
    constexpr M MatrixInverse(const M &mtx, bool *out_is_singular) {
        using Type = typename M::UnderlyingType;

        const Type determinant = MatrixDeterminant(mtx);
        const bool is_singular = (determinant == 0.0);
        if (out_is_singular != nullptr) { *out_is_singular = is_singular; }
        if (is_singular) { return mtx; }
        const Type inv_det = static_cast<Type>(1.0) / determinant;
        constexpr u32 min_square = Min(M::cRowCount, M::cColumnCount);
        M ret;
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
    constexpr T Absolute(const T &matrix) {
        using Type = typename T::UnderlyingType;
        T ret = {};
        if constexpr (T::cIsRowMajor) {
            using V = TVector<Type, T::cColumnCount, T::cIsVectorized>;
            for (u32 i = 0; i < T::cRowCount; ++i) {           
                const V row = LoadRow(matrix, i);
                const V abs = Absolute(row);
                StoreRow(std::addressof(ret), abs, i);
            }
        } else {
            using V = TVector<Type, T::cColumnCount, T::cIsVectorized>;
            for (u32 i = 0; i < T::cColumnCount; ++i) {           
                const V col = LoadColumn(matrix, i);
                const V abs = Absolute(col);
                StoreColumn(std::addressof(ret), abs, i);
            }
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsMatrixType)
    constexpr bool operator==(const T &left, const T &right) {
        bool result = true;
        for (u32 i = 0; i < T::cColumnCount; ++i) {
            for (u32 j = 0; j < T::cRowCount; ++j) {
                result &= (left[j, i] == right[j, i]);
            }
        }
        return result;
    }

    template <typename T>
        requires (T::cIsMatrixType)
    constexpr bool operator!=(const T &left, const T &right) {
        return !(left == right);
    }
}
