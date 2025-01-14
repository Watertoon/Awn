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
            bool m_trigger_is_update;
            MatrixRowMajor4x4f m_projection_matrix;
        public:
            VP_RTTI_BASE(Projection);
        public:
            constexpr Projection() : m_trigger_is_update(true) {/*...*/}
            constexpr virtual ~Projection() {/*...*/}

            constexpr void UpdateMatrix() {
                if (m_trigger_is_update) {        
                    this->CalculateProjectionMatrix(std::addressof(m_projection_matrix));
                }
                m_trigger_is_update = false;
            }

            constexpr MatrixRowMajor4x4f *GetProjectionMatrix() {
                return std::addressof(m_projection_matrix);
            }

            constexpr const MatrixRowMajor4x4f *GetProjectionMatrix() const {
                return std::addressof(m_projection_matrix);
            }

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const = 0;
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
            constexpr FrustumProjection() : Projection(), m_near(), m_far(), m_top(), m_bottom(), m_left(), m_right() {/*...*/}
            constexpr FrustumProjection(float near, float far, const BoundingBox2f& bound_box) : Projection(), m_near(near), m_far(far), m_top(bound_box.max_y), m_bottom(bound_box.min_y), m_left(bound_box.max_x), m_right(bound_box.min_x) {/*...*/}
            constexpr FrustumProjection(float near, float far, float top, float bottom, float left, float right) : Projection(), m_near(near), m_far(far), m_top(top), m_bottom(bottom), m_left(left), m_right(right) {/*...*/}
            constexpr virtual ~FrustumProjection() override {/*...*/}

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const override {

                const float reciprocal_a = 1.0f / (m_right - m_left);
                const float reciprocal_b = 1.0f / (m_top - m_bottom);
                const float reciprocal_c = 1.0f / (m_far - m_near);
                const float near2 = (m_near + m_near);

                const v4f row0 = { near2 *  reciprocal_a,             0.0f,                               0.0f,                                       0.0f, };
                const v4f row1 = { 0.0f,                              near2 * reciprocal_b,               0.0f,                                       0.0f, };
                const v4f row2 = { (m_right + m_left) * reciprocal_a, (m_top + m_bottom) * reciprocal_b, -(m_near + m_far) * reciprocal_c,           -1.0f, };
                const v4f row3 = { 0.0f,                              0.0f,                              -((m_far + m_far) * m_near) * reciprocal_c,  0.0f, };

                StoreColumn(out_proj_matrix, row0, 0);
                StoreColumn(out_proj_matrix, row1, 1);
                StoreColumn(out_proj_matrix, row2, 2);
                StoreColumn(out_proj_matrix, row3, 3);
            }
    };

    class PerspectiveProjection : public Projection {
        public:
            static constexpr float cDefaultFovy = TRadians<float, 45.0f>;
            static constexpr float cDefaultFovySin = vp::util::Sin(cDefaultFovy / 2);
            static constexpr float cDefaultFovyCos = vp::util::Cos(cDefaultFovy / 2);
            static constexpr float cDefaultFovyTan = vp::util::Tan(cDefaultFovy / 2);
        private:
            float m_near;
            float m_far;
            float m_fov_y;
            float m_top_div_hypotenuse;
            float m_near_div_hypotenuse;
            float m_top_div_near;
            float m_aspect;
            vp::util::Float2 m_offset;
        public:
            VP_RTTI_DERIVED(PerspectiveProjection, Projection);
        public:
            constexpr PerspectiveProjection() : Projection(), m_near(1.0f), m_far(10000.0f), m_fov_y(cDefaultFovy), m_top_div_hypotenuse(cDefaultFovySin), m_near_div_hypotenuse(cDefaultFovyCos), m_top_div_near(cDefaultFovyTan), m_aspect(4.0 / 3.0f), m_offset() {/*...*/}
            constexpr PerspectiveProjection(float near, float far, float fovy, float aspect) : Projection(), m_near(near), m_far(far), m_fov_y(fovy), m_top_div_hypotenuse(vp::util::Sin(fovy / 2)), m_near_div_hypotenuse(vp::util::Cos(fovy / 2)), m_top_div_near(vp::util::Tan(fovy / 2)), m_aspect(aspect), m_offset() {/*...*/}
            constexpr virtual ~PerspectiveProjection() override {/*...*/}

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const override {

                const float near2 = (m_near + m_near);
                const float t_n = near2 * m_top_div_near;
                const float t_n_div2 = t_n / 2;
                const float t_n_a = t_n * m_aspect;

                const float t_n_a_ox = t_n_a * m_offset[0];
                const float t_n_a_div2 = t_n_a / 2;
                const float r = t_n_a_ox + t_n_a_div2;
                const float l = t_n_a_ox - t_n_a_div2;
                const float r_add_l = r + l;
                const float r_sub_l = r - l;

                const float t_n_oy =  t_n * m_offset[1];
                const float t_add_b = (t_n_oy + t_n_div2) + (t_n_oy - t_n_div2);
                const float t_sub_b = (t_n_oy + t_n_div2) - (t_n_oy - t_n_div2);

                const float reciprocal_a = 1.0f / r_sub_l;
                const float reciprocal_b = 1.0f / t_sub_b;
                const float reciprocal_c = 1.0f / (m_far - m_near);

                /* Row major style projection matrix */
                /* | 2n/(r-l),    0,            0,              0, | */
                /* | 0,           2n/(t-b),     0,              0, | */
                /* | (r+l)/(r-l), (t+b)/(t-b), -((f+n)/(f-n)), -1, | */
                /* | 0,           0,           -(2fn/(f-n)),    0, | */
                const v4f row0 = { near2 * reciprocal_a,   0.0f,                    0.0f,                                  0.0f, };
                const v4f row1 = { 0.0f,                   near2 * reciprocal_b,    0.0f,                                  0.0f, };
                const v4f row2 = { r_add_l * reciprocal_a, t_add_b * reciprocal_b, -(m_near + m_far) * reciprocal_c,      -1.0f, };
                const v4f row3 = { 0.0f,                   0.0f,                   -2.0f * m_far * m_near * reciprocal_c,  0.0f, };

                StoreColumn(out_proj_matrix, row0, 0);
                StoreColumn(out_proj_matrix, row1, 1);
                StoreColumn(out_proj_matrix, row2, 2);
                StoreColumn(out_proj_matrix, row3, 3);
            }

            constexpr void SetAspect(float new_aspect) {
                m_aspect = new_aspect;
                m_trigger_is_update = true;
            }

            void SetFovX(float new_fov_x) {
                m_aspect = vp::util::Tan(new_fov_x * 0.5f) / m_top_div_near;
                m_trigger_is_update = true;
            }
    };

    class OrthographicProjection : public Projection {
        private:
            float m_near;
            float m_far;
            float m_top;
            float m_bottom;
            float m_left;
            float m_right;
        public:
            VP_RTTI_DERIVED(OrthographicProjection, Projection);
        public:
            constexpr OrthographicProjection() : Projection(), m_near(0.0f), m_far(1.0f), m_top(0.5f), m_bottom(-0.5f), m_left(-0.5f), m_right(0.5f) {/*...*/}
            constexpr virtual ~OrthographicProjection() override {/*...*/}

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const override {

                const float r_add_l = (m_right + m_left) / 2.0f;
                const float r_sub_l = (m_right - m_left) / 2.0f;
                const float t_add_b = (m_top + m_bottom) / 2.0f;
                const float t_sub_b = (m_top - m_bottom) / 2.0f;

                const float rec_fn = 1.0f / (m_far - m_near);
                const float neg_fn = -(m_near + m_far);

                const v4f row0 = { 1.0f / r_sub_l,    0.0f,               0.0f,            0.0f, };
                const v4f row1 = { 0.0f,              1.0f / t_sub_b,     0.0f,            0.0f, };
                const v4f row2 = { 0.0f,              0.0f,              -2.0f * rec_fn,   0.0f, };
                const v4f row3 = { r_add_l / r_sub_l, t_add_b / t_sub_b,  neg_fn * rec_fn, 1.0f, };

                StoreColumn(out_proj_matrix, row0, 0);
                StoreColumn(out_proj_matrix, row1, 1);
                StoreColumn(out_proj_matrix, row2, 2);
                StoreColumn(out_proj_matrix, row3, 3);
            }
    };
}
