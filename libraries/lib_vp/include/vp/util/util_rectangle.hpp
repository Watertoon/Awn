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
	struct Rectangle {
        union {
            struct {                        
                T min_x;
                T min_y;
                T max_x;
                T max_y;
            };
            struct {
                T left;
                T top;
                T right;
                T bottom;
            };
        };

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr V GetPosition() const {
            using Type = typename V::UnderlyingType;
            const Type x = static_cast<Type>(this->left + this->right) / static_cast<Type>(2);
            const Type y = static_cast<Type>(this->top + this->bottom) / static_cast<Type>(2);
            return V{ x, y, };
        }

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr V GetSize() const {
            using Type = typename V::UnderlyingType;
            const Type w = static_cast<T>(right - left);
            const Type h = static_cast<T>(top - bottom);
            return V{ w, h, };
        }

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr void SetPosition(const V &position) {
            using Type = typename V::UnderlyingType;
            const V size = this->GetSize<V>();
            const Type next_l = position[0] - (size[0] / static_cast<Type>(2));
            const Type next_t = position[1] - (size[1] / static_cast<Type>(2));
            const Type next_r = position[0] + (size[0] / static_cast<Type>(2));
            const Type next_b = position[1] + (size[1] / static_cast<Type>(2));
            this->left   = static_cast<T>(next_l);
            this->top    = static_cast<T>(next_t);
            this->right  = static_cast<T>(next_r);
            this->bottom = static_cast<T>(next_b);
        }

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr void SetSize(const V &size) {
            using Type = typename V::UnderlyingType;
            const V position = this->GetPosition<V>();
            const Type next_l = position[0] - (size[0] / static_cast<Type>(2));
            const Type next_t = position[1] - (size[1] / static_cast<Type>(2));
            const Type next_r = position[0] + (size[0] / static_cast<Type>(2));
            const Type next_b = position[1] + (size[1] / static_cast<Type>(2));
            this->left   = static_cast<T>(next_l);
            this->top    = static_cast<T>(next_t);
            this->right  = static_cast<T>(next_r);
            this->bottom = static_cast<T>(next_b);
        }

        constexpr bool IsOverlapping(const Rectangle *overlapping_rect) const {
            const T left_l   = this->left;
            const T top_l    = this->top;
            const T right_l  = this->right;
            const T bottom_l = this->bottom;
            const T left_r   = overlapping_rect->left;
            const T top_r    = overlapping_rect->top;
            const T right_r  = overlapping_rect->right;
            const T bottom_r = overlapping_rect->bottom;

            /* Check dimensions are in bounds */
            const bool is_hit_w = (left_l < right_r) && (left_r < right_l);
            const bool is_hit_h = (bottom_r < top_l) && (bottom_l < top_r);
            const bool result   = is_hit_w && is_hit_h;

            return result;
        }

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr bool IsOverlapping(const V &position) const {
            const T left_l   = this->left;
            const T top_l    = this->top;
            const T right_l  = this->right;
            const T bottom_l = this->bottom;

            const T x = static_cast<T>(position[0]);
            const T y = static_cast<T>(position[1]);

            /* Check X and Y is in bounds */
            const bool is_hit_x = (x < right_l) & (left_l < x);
            const bool is_hit_y = (y < top_l) & (bottom_l < y);
            const bool result   = is_hit_x & is_hit_y;

            return result;
        }

        constexpr bool IsTouching(const Rectangle *touching_rect) const {
            const T left_l   = this->left;
            const T top_l    = this->top;
            const T right_l  = this->right;
            const T bottom_l = this->bottom;
            const T left_r   = touching_rect->left;
            const T top_r    = touching_rect->top;
            const T right_r  = touching_rect->right;
            const T bottom_r = touching_rect->bottom;

            /* Check all edges for overlap */
            const bool hit_tl = (left_l < right_r) && (bottom_r < top_l);
            const bool hit_br = (left_r < right_l) && (bottom_l < top_r);
            const bool hit_tr = (bottom_r < top_l) && (left_r < right_l);
            const bool hit_bl = (bottom_l < top_r) && (left_l < right_r);

            const bool result = hit_br || hit_tl || hit_tr || hit_bl;

            return result;
        }

        constexpr bool TryReject(Rectangle *out_reject_rect, const Rectangle *overlapping_rect) {
            const T left_l   = this->left;
            const T top_l    = this->top;
            const T right_l  = this->right;
            const T bottom_l = this->bottom;
            const T left_r   = overlapping_rect->left;
            const T top_r    = overlapping_rect->top;
            const T right_r  = overlapping_rect->right;
            const T bottom_r = overlapping_rect->bottom;

            /* Compute overlap */
            const bool hit_tl = (left_l < right_r) && (bottom_r < top_l);
            const bool hit_br = (left_r < right_l) && (bottom_l < top_r);

            /* Set reject on exit */
            T reject_x = 0;
            T reject_y = 0;
            const auto reject_guard = SCOPE_GUARD {
                out_reject_rect->left   = left_r   + reject_x;
                out_reject_rect->top    = top_r    + reject_y;
                out_reject_rect->right  = right_r  + reject_x;
                out_reject_rect->bottom = bottom_r + reject_y;
            };

            /* Full overlap */
            if (hit_tl && hit_br) {

                /* Reject along nearest edge */
                const T reject_left  = right_l - left_r;
                const T reject_up    = top_l - bottom_r;
                const T reject_right = left_l - right_r;
                const T reject_down  = bottom_l - top_r;

                /* Select minimum reject, defaulting to top left */
                const T abs_left  = vp::util::Abs(reject_left);
                const T abs_up    = vp::util::Abs(reject_up);
                const T abs_right = vp::util::Abs(reject_right);
                const T abs_down  = vp::util::Abs(reject_down);
                T       reject_x  = 0;
                T       reject_y  = 0;
                if (abs_up < abs_left && abs_up < abs_right) {
                    reject_y = (abs_up <= abs_down) ? reject_up : reject_down;
                } else if (abs_down < abs_left && abs_down < abs_right) {
                    reject_y = reject_down;
                } else if (abs_left < abs_down && abs_left < abs_up) {
                    reject_x = (abs_left <= abs_right) ? reject_left : reject_right;
                } else if (abs_right < abs_down && abs_right < abs_up) {
                    reject_x = reject_right;
                } else {
                    reject_x = reject_left;
                    reject_x = reject_up;
                }

                return true;
            }
            
            /* Compute corner overlap */
            const bool hit_tr = (bottom_r < top_l) && (left_r < right_l);
            const bool hit_bl = (bottom_l < top_r) && (left_l < right_r);

            /* Half overlap */
            if (hit_tl && hit_tr) {

                /* Reject up */
                reject_x = 0;
                reject_y = top_l - bottom_r;

                return true;
            } else if (hit_tl && hit_bl) {

                /* Reject right */
                reject_x = left_l - right_r;
                reject_y = 0;
    
                return true;
            } else if (hit_bl && hit_br) {

                /* Reject down */
                reject_x = 0;
                reject_y = bottom_l - top_r;

                return true;
            } else if (hit_br && hit_tr) {

                /* Reject left */
                reject_x = right_l - left_r;
                reject_y = 0;

                return true;
            }

            /* Corner overlap */
            if (hit_tl) {

                /* Reject along nearest edge (diagonally if equidistant) */
                const T reject_left = right_l - left_r;
                const T reject_up   = top_l - bottom_r;

                reject_x = (reject_left <= reject_up) ? reject_left : 0;
                reject_y = (reject_up <= reject_left) ? reject_up : 0;

                return true;
            } else if (hit_br) {

                /* Reject along nearest edge (diagonally if equidistant) */
                const T reject_right = left_l - right_r;
                const T reject_down = bottom_l - top_r;

                reject_x = (reject_right <= reject_down) ? reject_right : 0;
                reject_y = (reject_down <= reject_right) ? reject_down : 0;

                return true;
            } else if (hit_tr) {

                /* Reject along nearest edge (diagonally if equidistant) */
                const T reject_right = left_l - right_r;
                const T reject_up    = top_l - bottom_r;

                reject_x = (reject_right <= reject_up) ? reject_right : 0;
                reject_y = (reject_up <= reject_right) ? reject_up : 0;

                return true;
            } else if (hit_bl) {

                /* Reject along nearest edge (diagonally if equidistant) */
                const T reject_left   = right_l - left_r;
                const T reject_bottom = bottom_l - top_r;

                reject_x = (reject_left <= reject_bottom) ? reject_left : 0;
                reject_y = (reject_bottom <= reject_left) ? reject_bottom : 0;

                return true;
            }

            /* No overlap */
            reject_guard.Cancel();
            return false;
        }

        constexpr bool TryIntersect(Rectangle *out_intersect_rect, const Rectangle *overlapping_rect) const {
            const T left_l   = left;
            const T top_l    = top;
            const T right_l  = right;
            const T bottom_l = bottom;
            const T left_r   = overlapping_rect->left;
            const T top_r    = overlapping_rect->top;
            const T right_r  = overlapping_rect->right;
            const T bottom_r = overlapping_rect->bottom;

            /* Compute overlap */
            const bool hit_tl = (left_l < right_r) && (bottom_r < top_l);
            const bool hit_br = (left_r < right_l) && (bottom_l < top_r);

            /* Full overlap */
            if (hit_tl && hit_br) {
                out_intersect_rect = *overlapping_rect;
                return true;
            }

            /* Compute corner overlap */
            const bool hit_tr = (bottom_r < top_l) && (left_r < right_l);
            const bool hit_bl = (bottom_l < top_r) && (left_l < right_r);

            /* Half overlap */
            if (hit_tl && hit_tr) {
                out_intersect_rect->left   = left_r;
                out_intersect_rect->top    = top_l;
                out_intersect_rect->right  = right_r;
                out_intersect_rect->bottom = bottom_r;
                return true;
            } else if (hit_tl && hit_bl) {
                out_intersect_rect->left   = left_l;
                out_intersect_rect->top    = top_r;
                out_intersect_rect->right  = right_r;
                out_intersect_rect->bottom = bottom_r;
                return true;
            } else if (hit_bl && hit_br) {
                out_intersect_rect->left   = left_r;
                out_intersect_rect->top    = top_r;
                out_intersect_rect->right  = right_r;
                out_intersect_rect->bottom = bottom_l;
                return true;
            } else if (hit_br && hit_tr) {
                out_intersect_rect->left   = left_r;
                out_intersect_rect->top    = top_r;
                out_intersect_rect->right  = right_l;
                out_intersect_rect->bottom = bottom_r;
                return true;
            }

            /* Corner overlap */
            if (hit_tl) {
                out_intersect_rect->left   = left_l;
                out_intersect_rect->top    = top_l;
                out_intersect_rect->right  = right_r;
                out_intersect_rect->bottom = bottom_r;
                return true;
            } else if (hit_br) {
                out_intersect_rect->left   = left_r;
                out_intersect_rect->top    = top_r;
                out_intersect_rect->right  = right_l;
                out_intersect_rect->bottom = bottom_l;
                return true;
            } else if (hit_tr) {
                out_intersect_rect->left   = left_r;
                out_intersect_rect->top    = top_l;
                out_intersect_rect->right  = right_l;
                out_intersect_rect->bottom = bottom_r;
                return true;
            } else if (hit_bl) {
                out_intersect_rect->left   = left_l;
                out_intersect_rect->top    = top_r;
                out_intersect_rect->right  = right_r;
                out_intersect_rect->bottom = bottom_l;
                return true;
            }

            /* No overlap */
            return false;
        }

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr V ClampPoint(const V &position) const {

            /* Clamp position to rect */
            using Type = typename V::UnderlyingType;
            const T pos_x   = static_cast<T>(position[0]);
            const T pos_y   = static_cast<T>(position[1]);
            const T min_x   = vp::util::Max(left, pos_x);
            const T min_y   = vp::util::Max(top, pos_y);
            const T clamp_x = vp::util::Min(right, min_x);
            const T clamp_y = vp::util::Min(bottom, min_y);

            return V{ static_cast<Type>(clamp_x), static_cast<Type>(clamp_y), };
        }

        template <typename V>
            requires (V::cIsVectorType && 2 <= V::cVectorCount)
        constexpr vp::util::Float2 NormalizePointFloat(const V &position) const {

            /* Convert to normalized floating point */
            V point = this->ClampPoint(position);
            V size = this->GetSize();
            const float logical_x = static_cast<float>(point[0]) / static_cast<float>(size[0]);
            const float logical_y = static_cast<float>(point[1]) / static_cast<float>(size[1]);

            return { logical_x, logical_y, };
        }
	};

    using Rectanglef = Rectangle<float>;
    using Rectanglei = Rectangle<s32>;

    using BoundingBox2f = Rectangle<float>;
    using BoundingBox2i = Rectangle<s32>;
}
