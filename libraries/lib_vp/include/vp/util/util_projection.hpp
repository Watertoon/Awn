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

    class Projection {
        protected:
            Matrix44f m_projection_matrix;
            bool      m_requires_update;
        public:
            VP_RTTI_BASE(Projection);
        public:
            constexpr Projection() : m_requires_update(true) {/*...*/}
            constexpr virtual ~Projection() {/*...*/}

            constexpr void UpdateProjectionMatrixSelf() {
                this->UpdateMatrix(std::addressof(m_projection_matrix));
                m_requires_update = false;
            }

            constexpr Matrix44f *GetProjectionMatrix() {
                if (m_requires_update == true) {
                    this->UpdateProjectionMatrixSelf();
                }
                return std::addressof(m_projection_matrix);
            }

            constexpr const Matrix44f *GetProjectionMatrix() const {
                return std::addressof(m_projection_matrix);
            }

            constexpr virtual void UpdateMatrix(Matrix44f *out_proj_matrix) const = 0;
    };
    
    class FrustumProjection : public Projection {
        public:
            friend class PerspectiveProjection;
        protected:
            float m_near;
            float m_far;
            float m_top;
            float m_bottom;
            float m_left;
            float m_right;
        public:
            VP_RTTI_DERIVED(FrustumProjection, Projection);
        public:
            constexpr FrustumProjection() {/*...*/}
            constexpr FrustumProjection(float near, float far, const BoundingBox2<float>& bound_box) : Projection(), m_near(near), m_far(far), m_top(bound_box.max_y), m_bottom(bound_box.min_y), m_left(bound_box.max_x), m_right(bound_box.min_x) {/*...*/}
            constexpr FrustumProjection(float near, float far, float top, float bottom, float left, float right) : Projection(), m_near(near), m_far(far), m_top(top), m_bottom(bottom), m_left(left), m_right(right) {/*...*/}
            constexpr virtual ~FrustumProjection() override {/*...*/}

            constexpr virtual void UpdateMatrix(Matrix44f *out_proj_matrix) const override {
                const float reciprocal_a = 1.0f / (m_right + m_left);
                const Vector4f row1 = {
                    (m_near + m_near) *  reciprocal_a,
                    0.0f,
                    (m_left + m_right) * reciprocal_a,
                    0.0f
                };
                out_proj_matrix->m_row1 = row1;

                const float reciprocal_b = 1.0f / (m_top - m_bottom);
                const Vector4f row2 = {
                    0.0f,
                    (m_near + m_near) * reciprocal_b,
                    (m_top + m_bottom) * reciprocal_b,
                    0.0f
                };
                out_proj_matrix->m_row2 = row2;

                const float reciprocal_c = 1.0f / (m_far - m_near);
                const Vector4f row3 = {
                    0.0f,
                    0.0f,
                    -((m_near + m_far) * reciprocal_c),
                    -((m_far + m_far) * m_near * reciprocal_c)
                };
                out_proj_matrix->m_row3 = row3;

                out_proj_matrix->m_row4 = { 0.0f, 0.0f, -1.0f, 0.0f };
            }
    };

    class PerspectiveProjection : public FrustumProjection {
        private:
            float m_fov_x;
        public:
            VP_RTTI_DERIVED(PerspectiveProjection, FrustumProjection);
        public:
            constexpr PerspectiveProjection() : FrustumProjection(1.0f, 10000.0f, TRadians<float, 45.0f>, ::sinf(TRadians<float, 45.0f> / 2), ::cosf(TRadians<float, 45.0f> / 2), ::tanf(TRadians<float, 45.0f> / 2)), m_fov_x(4.0 / 3.0f) {/*...*/}
            constexpr PerspectiveProjection(float near, float far, float fovy, float aspect) : FrustumProjection(near, far, fovy, ::sinf(fovy / 2), ::cosf(fovy / 2), ::tanf(fovy / 2)), m_fov_x(aspect) {/*...*/}
            constexpr virtual ~PerspectiveProjection() override {/*...*/}

            constexpr virtual void UpdateMatrix(Matrix44f *out_proj_matrix) const override {
                const Vector4f row1 = {
                    1.0f / (m_right * m_fov_x),
                    0.0f,
                    0.0f,
                    0.0f
                };
                out_proj_matrix->m_row1 = row1;

                const Vector4f row2 = {
                    0.0f,
                    1.0f / m_right,
                    0.0f,
                    0.0f
                };
                out_proj_matrix->m_row2 = row2;

                const float reciprocal_c = 1.0f / (m_far - m_near);
                const Vector4f row3 = {
                    0.0f,
                    0.0f,
                    -((m_near + m_far) * reciprocal_c),
                    -((m_far + m_far) * m_near * reciprocal_c)
                };
                out_proj_matrix->m_row3 = row3;

                out_proj_matrix->m_row4 = { 0.0f, 0.0f, -1.0f, 0.0f };
            }

            void SetAspect(float new_aspect) {
                m_fov_x = new_aspect;
                m_requires_update = true;
            }

            void SetFovX(float new_fov_x) {
                m_fov_x = ::tanf(new_fov_x * 0.5f) / m_right;
                m_requires_update = true;
            }
    };
}
