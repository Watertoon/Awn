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
    constexpr TVectorType<typename T::UnderlyingType, 4> LoadRow(const T &matrix, u32 col_index) {
        /* Use a vector load if rows are contiguous in memory */
        if constexpr (T::cIsColumnMajor) {
            /* Load vectorized matrices by aligned load, for non-vector 4 rows use unaligned load, 3 rows use mask load */
            if constexpr (T::cIsVectorized) {
                return Load4Aligned(std::addressof(matrix[0,col_index]));
            } else if constexpr (T::cRowCount == 4) {
                return Load4(std::addressof(matrix[0,col_index]));
            } else if constexpr (sizeof(typename T::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetLoad4Mask128(true, true, true, false);
                return Load4Mask128(std::addressof(matrix[0,col_index]), mask);
            } else if constexpr (sizeof(typename T::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetLoad4Mask256(true, true, true, false);
                return Load4Mask256(std::addressof(matrix[0,col_index]), mask);
            }
        }
        /* Row major fallback */
        using v4t = TVectorType<typename T::UnderlyingType, 4>;
        v4t ret = {};
        for (u32 i = 0; i < T::cRowCount; ++i) {
            ret[i] = matrix[i,col_index];
        }
        return ret;
    }

    template <typename T>
        requires (T::cIsMatrixType && (T::cColumnCount == 3 || T::cColumnCount == 4))
    constexpr TVectorType<typename T::UnderlyingType, 4> LoadColumn(const T &matrix, u32 row_index) {
        /* Use a vector load is columns are contiguous in memory */
        if constexpr (T::cIsRowMajor) {
            /* Load vectorized matrices by aligned load, for non-vector 4 rows use unaligned load, 3 rows use mask load */
            if constexpr (T::cIsVectorized) {
                return Load4Aligned(std::addressof(matrix[row_index,0]));
            } else if constexpr (T::cColumnCount == 4) {
                return Load4(std::addressof(matrix[row_index,0]));
            } else if constexpr (T::cColumnCount == 3 && sizeof(typename T::UnderlyingType) == sizeof(float)) {
                const v4si mask = GetLoad4Mask128(true, true, true, false);
                return Load4Mask128(std::addressof(matrix[row_index,0]), mask);
            } else if constexpr (T::cColumnCount == 3 && sizeof(typename T::UnderlyingType) == sizeof(double)) {
                const v4sll mask = GetLoad4Mask256(true, true, true, false);
                return Load4Mask256(std::addressof(matrix[row_index,0]), mask);
            }
        }
        /* Column major fallback */
        using v4t = TVectorType<typename T::UnderlyingType, 4>;
        v4t ret = {};
        for (u32 i = 0; i < T::cColumnCount; ++i) {
            ret[i] = matrix[row_index,i];
        }
        return ret;
    }

    template <typename Ret, typename T>
        requires (Ret::cIsMatrixType && (Ret::cRowCount == 3 || Ret::cRowCount == 4)) && (T::cIsVectorizedType && Ret::cRowCount <= T::cCount)
    constexpr void StoreRow(Ret *matrix, const T &row, u32 col_index) {
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
    constexpr void StoreColumn(Ret *matrix, const T &col, u32 row_index) {
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
            using v4t = TVectorType<Type, 4>;
            v4t l_row_array[Ret::cRowCount];
            for (u32 i = 0; i < T0::cRowCount; ++i) {
                l_row_array[i] = LoadColumn(lhs_matrix, i);
            }
            for (u32 i = T0::cRowCount; i < Ret::cRowCount; ++i) {
                l_row_array[i] = v4t{};
                l_row_array[i][i] = static_cast<Type>(1);
            }
            v4t r_row_array[Ret::cRowCount];
            for (u32 i = 0; i < T1::cRowCount; ++i) {
                r_row_array[i] = LoadColumn(rhs_matrix, i);
            }
            for (u32 i = T1::cRowCount; i < Ret::cRowCount; ++i) {
                r_row_array[i] = v4t{};
                r_row_array[i][i] = static_cast<Type>(1);
            }
            if constexpr (T0::cColumnCount == 3) {
                l_row_array[3][3] = static_cast<Type>(1);
            }
            if constexpr (T1::cColumnCount == 3) {
                r_row_array[3][3] = static_cast<Type>(1);
            }

            /* For each result row + column, multiply all elements of left column with elements with all elements of right column */
            const auto row_lambda = [=](const v4t &r_row) {
                /* Broadcast column values of left matrix, multiply, and add */
                const v4t row0_brod0 = Broadcast4(std::addressof(r_row[0]));
                v4t rowi_mul = row0_brod0 * l_row_array[0];
                for (u32 i = 1; i < Ret::cRowCount; ++i) {
                    const v4t rowi_brod = Broadcast4(std::addressof(r_row[i]));
                    rowi_mul = MultiplyAdd(rowi_brod, l_row_array[i], rowi_mul);
                }
                return rowi_mul;
            };
            if constexpr (Ret::cIsColumnMajor) {
                /* Transpose result for opposite major result matrix */
                TMatrix<typename Ret::UnderlyingType, Ret::cRowCount, Ret::cColumnCount, !Ret::cIsRowMajor> matrix;
                for (u32 i = 0; i < Ret::cRowCount; ++i) {
                    const v4t result = row_lambda(r_row_array[i]);
                    StoreColumn(std::addressof(matrix), result, i);
                }
                const Ret ret = ConvertMajor<Ret>(matrix);
                return ret;
            }
            Ret ret;
            for (u32 i = 0; i < Ret::cRowCount; ++i) {
                const v4t result = row_lambda(r_row_array[i]);
                StoreColumn(std::addressof(ret), result, i);
            }
            return ret;
        }
        if constexpr (T0::cIsColumnMajor && (T0::cColumnCount == 4 || T0::cColumnCount == 3) && T0::cRowCount == 4
        && T1::cIsColumnMajor && ((T1::cColumnCount == 3  || T1::cColumnCount == 4) && T1::cRowCount == 4)) {

            /* Load all columns */
            using Type = typename T0::UnderlyingType;
            using v4t = TVectorType<Type, 4>;
            v4t l_col_array[Ret::cRowCount];
            for (u32 i = 0; i < T0::cColumnCount; ++i) {
                l_col_array[i] = LoadRow(lhs_matrix, i);
            }
            for (u32 i = T0::cColumnCount; i < Ret::cRowCount; ++i) {
                l_col_array[i] = v4t{};
                l_col_array[i][i] = static_cast<Type>(1);
            }
            v4t r_col_array[Ret::cRowCount];
            for (u32 i = 0; i < T1::cColumnCount; ++i) {
                r_col_array[i] = LoadRow(rhs_matrix, i);
            }
            for (u32 i = T1::cColumnCount; i < Ret::cRowCount; ++i) {
                r_col_array[i] = v4t{};
                r_col_array[i][i] = static_cast<Type>(1);
            }

            /* For each result row + column, multiply all elements of left matrix column with elements with all elements of right matrix row */
            const auto col_lambda = [=](const v4t &l_col) {
                /* Broadcast row values of right matrix, multiply, and add */
                const v4t col0_brod0 = Broadcast4(std::addressof(l_col[0]));
                v4t coli_mul  = col0_brod0 * r_col_array[0];
                for (u32 i = 1; i < T1::cRowCount; ++i) {
                    const v4t coli_brod = Broadcast4(std::addressof(l_col[i]));
                    coli_mul = MultiplyAdd(coli_brod, r_col_array[i], coli_mul);
                }
                return coli_mul;
            };
            if constexpr (Ret::cIsRowMajor) {
                /* Transpose result for opposite major result matrix */
                TMatrix<typename Ret::UnderlyingType, Ret::cRowCount, Ret::cColumnCount, !Ret::cIsRowMajor> matrix;
                for (u32 i = 0; i < T0::cRowCount; ++i) {
                    const v4t result = col_lambda(l_col_array[i]);
                    StoreRow(std::addressof(matrix), result, i);
                }
                const Ret ret = ConvertMajor<Ret>(matrix);
                return ret;
            }
            Ret ret = {};
            for (u32 i = 0; i < T0::cRowCount; ++i) {
                const v4t result = col_lambda(l_col_array[i]);
                StoreRow(std::addressof(ret), result, i);
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