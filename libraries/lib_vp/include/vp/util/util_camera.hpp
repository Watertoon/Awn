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

    class Camera {
        private:
            MatrixRowMajor4x3f m_view_matrix;
        public:
            VP_RTTI_BASE(Camera);
        public:
            constexpr Camera() : m_view_matrix(MatrixConstant<MatrixRowMajor4x3f>::cIdentity) {/*...*/}
            constexpr virtual ~Camera() {/*...*/}

            void UpdateMatrix() {
                this->CalculateViewMatrix(std::addressof(m_view_matrix));
            }

            constexpr MatrixRowMajor4x3f *GetViewMatrix();
            constexpr const MatrixRowMajor4x3f *GetViewMatrix() const;

            constexpr Vector3f GetRightByMatrix() const;
            constexpr Vector3f GetAtByMatrix() const;
            constexpr Vector3f GetUpByMatrix() const;

            constexpr Vector3f GetWorldPositionByMatrix() const;
            constexpr Vector3f ConvertViewPositionToWorldPositionByMatrix(const Vector3f& view_pos) const;
        public:
            constexpr virtual void CalculateViewMatrix(MatrixRowMajor4x3f *out_view_matrix) const = 0;
    };

    class LookAtCamera : public Camera {
        private:
            Vector3d  m_pos;
            Vector3d  m_at;
            Vector3f  m_up;
        public:
            VP_RTTI_DERIVED(LookAtCamera, Camera);
        public:
            constexpr LookAtCamera() : Camera(), m_pos(Vector3d{0.0,0.0,10.0}), m_at(Vector3d{0.0,0.0,0.0}), m_up(Vector3f{0.0f,1.0f,0.0f}) {/*...*/}
            constexpr LookAtCamera(const Vector3d& pos, const Vector3d& at, const Vector3f& up) : Camera(), m_pos(pos), m_at(at), m_up(up) {/*...*/}
            constexpr LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up) : Camera(), m_pos(Convert<Vector3d>(pos)), m_at(Convert<Vector3d>(at)), m_up(up) {/*...*/}
            constexpr virtual ~LookAtCamera() override {/*...*/}

            template <typename V>
                requires (V::cIsVectorType) && (V::cCount >= 3)
            constexpr void SetPosition(const V &new_pos);

            template <typename V>
                requires (V::cIsVectorType) && (V::cCount >= 3)
            constexpr void SetAt(const V &new_at);

            constexpr void SetUp(const Vector3f &new_up);

            constexpr Vector3d GetPosition() const;
            constexpr Vector3d GetAt() const;
            constexpr Vector3f GetUp() const;

            constexpr Vector3d GetDirection() const;
            constexpr Vector3d GetDirectionAndMagnitude(double *out_dir_mag) const;
        public:
            constexpr Vector3d GetZAxis() const;
            constexpr Vector3f GetZAxisf() const;
        public:
            constexpr virtual void CalculateViewMatrix(MatrixRowMajor4x3f *out_view_matrix) const override;
    };

    class OrthographicCamera : public LookAtCamera {
        public:
            VP_RTTI_DERIVED(OrthographicCamera, LookAtCamera);
        public:
            constexpr OrthographicCamera() : LookAtCamera(Vector3d{0.0,0.0,0.0},Vector3d{0.0,0.0,-1.0},Vector3f{0.0f,1.0f,0.0f}) {/*...*/}
            constexpr OrthographicCamera(const OrthographicProjection &ortho_proj) : LookAtCamera(ortho_proj.GetOrthoCameraPosition(), ortho_proj.GetOrthoCameraAt(), VectorConstant<Vector3f>::cUnit[1]) {/*...*/}
            constexpr virtual ~OrthographicCamera() override {/*...*/}
    };

    /* Constexpr impl */
    constexpr MatrixRowMajor4x3f *Camera::GetViewMatrix() {
        return std::addressof(m_view_matrix);
    }

    constexpr const MatrixRowMajor4x3f *Camera::GetViewMatrix() const {
        return std::addressof(m_view_matrix);
    }

    constexpr Vector3f Camera::GetRightByMatrix() const {
        const MatrixRowMajor4x3f *view_mtx = this->GetViewMatrix();
        return LoadRow(*view_mtx, 0);
    }

    constexpr Vector3f Camera::GetUpByMatrix() const {
        const MatrixRowMajor4x3f *view_mtx = this->GetViewMatrix();
        return LoadRow(*view_mtx, 1);
    }

    constexpr Vector3f Camera::GetAtByMatrix() const {
        const MatrixRowMajor4x3f *view_mtx = this->GetViewMatrix();
        return LoadRow(*view_mtx, 2);
    }

    constexpr Vector3f Camera::GetWorldPositionByMatrix() const {
        const MatrixRowMajor4x3f *view_mtx = this->GetViewMatrix();
        const Vector3f world_pos = {
            (-((*view_mtx)[0,0] * (*view_mtx)[3,0]) - ((*view_mtx)[0,1] * (*view_mtx)[3,1])) - ((*view_mtx)[0,2] * (*view_mtx)[3,2]),
            (-((*view_mtx)[1,0] * (*view_mtx)[3,0]) - ((*view_mtx)[1,1] * (*view_mtx)[3,1])) - ((*view_mtx)[1,2] * (*view_mtx)[3,2]),
            (-((*view_mtx)[2,0] * (*view_mtx)[3,0]) - ((*view_mtx)[2,1] * (*view_mtx)[3,1])) - ((*view_mtx)[2,2] * (*view_mtx)[3,2]),
        };
        return world_pos;
    }

    constexpr Vector3f Camera::ConvertViewPositionToWorldPositionByMatrix(const Vector3f& view_pos) const {
        const MatrixRowMajor4x3f *view_mtx = this->GetViewMatrix();
        const Vector3f world_pos = {
            view_pos[0] + view_pos[1] + view_pos[2] + (-((*view_mtx)[0,0] * (*view_mtx)[3,0]) - ((*view_mtx)[0,1] * (*view_mtx)[3,1])) - ((*view_mtx)[0,2] * (*view_mtx)[3,2]),
            view_pos[0] + view_pos[1] + view_pos[2] + (-((*view_mtx)[1,0] * (*view_mtx)[3,0]) - ((*view_mtx)[1,1] * (*view_mtx)[3,1])) - ((*view_mtx)[1,2] * (*view_mtx)[3,2]),
            view_pos[0] + view_pos[1] + view_pos[2] + (-((*view_mtx)[2,0] * (*view_mtx)[3,0]) - ((*view_mtx)[2,1] * (*view_mtx)[3,1])) - ((*view_mtx)[2,2] * (*view_mtx)[3,2]),
        };
        return world_pos;
    }
    
    constexpr void LookAtCamera::CalculateViewMatrix(MatrixRowMajor4x3f *out_view_matrix) const {

        /* Ensure non-overlapping camera pos */
        const Vector3d u_pos = m_pos;
        const Vector3d u_at = m_at;
        if (u_pos == u_at) { return; }

        /* Calculate normalized z axis */
        const Vector3f z_axis = this->GetZAxisf();

        /* Calculate normalized x axis */
        const Vector3f right = Cross(m_up, z_axis);
        const Vector3f x_axis = Normalize(right);

        /* Calculate normalized y axis */
        const Vector3f y_axis = Cross(z_axis, x_axis);

        /* Calculate position */
        const Vector3d d_r = Convert<Vector3d>(x_axis);
        const Vector3d d_u = Convert<Vector3d>(y_axis);
        const Vector3d d_f = Convert<Vector3d>(z_axis);
        const Vector3d d_p = m_pos;
        const Vector3d rp = d_r * d_p;
        const Vector3d up = d_u * d_p;
        const Vector3d dp = d_f * d_p;
        const double pos_x = Sum(rp);
        const double pos_y = Sum(up);
        const double pos_z = Sum(dp);
        const Vector3f pos = {
            -static_cast<float>(pos_x),
            -static_cast<float>(pos_y),
            -static_cast<float>(pos_z),
            0.0f,
        };

        /* Set matrix */
        StoreRow(out_view_matrix, x_axis, 0);
        StoreRow(out_view_matrix, y_axis, 1);
        StoreRow(out_view_matrix, z_axis, 2);
        StoreRow(out_view_matrix, pos, 3);

        return;
    }

    template <typename V>
        requires (V::cIsVectorType) && (V::cCount >= 3)
    constexpr void LookAtCamera::SetPosition(const V &new_pos) {
        if constexpr (!std::is_same_v<typename V::UnderlyingType, double>) {
            m_pos = Convert<Vector3d>(new_pos);
        } else {
            m_pos = new_pos;
        }
    }

    template <typename V>
        requires (V::cIsVectorType) && (V::cCount >= 3)
    constexpr void LookAtCamera::SetAt(const V &new_at) {
        if constexpr (!std::is_same_v<typename V::UnderlyingType, double>) {
            m_at = Convert<Vector3d>(new_at);
        } else {
            m_at = new_at;
        }
    }

    constexpr void LookAtCamera::SetUp(const Vector3f &new_up) {
        const Vector3f next = Normalize(new_up);
        m_up = next;
    }

    constexpr Vector3d LookAtCamera::GetZAxis() const {
        const Vector3d d_inv_dir = m_pos - m_at;
        const Vector3d ret = Normalize(d_inv_dir);
        return ret;
    }
    constexpr Vector3d LookAtCamera::GetDirection() const {
        const Vector3d d_dir = m_at - m_pos;
        const Vector3d ret = Normalize(d_dir);
        return ret;
    }
    constexpr Vector3d LookAtCamera::GetDirectionAndMagnitude(double *out_dir_mag) const {
        const Vector3d d_dir = m_at - m_pos;
        const double mag = Magnitude(d_dir);
        const Vector3d ret = NormalizeByMagnitude(d_dir, mag);
        *out_dir_mag = mag;
        return ret;
    }
    constexpr Vector3d LookAtCamera::GetPosition() const { return m_pos; }
    constexpr Vector3d LookAtCamera::GetAt() const { return m_at; }
    constexpr Vector3f LookAtCamera::GetUp() const { return m_up; }

    constexpr Vector3f LookAtCamera::GetZAxisf() const {
        const Vector3d d_inv_dir = m_pos - m_at;
        const Vector3f inv_dir = Convert<Vector3f>(d_inv_dir);
        const Vector3f ret = Normalize(inv_dir);
        return ret;
    }
}
