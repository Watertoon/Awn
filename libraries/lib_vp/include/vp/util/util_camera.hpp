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
            MatrixRowMajor4x3f m_camera_mtx;
        public:
            VP_RTTI_BASE(Camera);
        public:
            constexpr Camera() : m_camera_mtx(vp::util::MatrixRowMajor4x3f::cIdentity) {/*...*/}
            constexpr virtual ~Camera() {/*...*/}

            void UpdateMatrix() {
                this->CalculateCameraMatrix(std::addressof(m_camera_mtx));
            }

            constexpr MatrixRowMajor4x3f *GetCameraMatrix() {
                return std::addressof(m_camera_mtx);
            }

            constexpr const MatrixRowMajor4x3f *GetCameraMatrix() const {
                return std::addressof(m_camera_mtx);
            }

            constexpr virtual void CalculateCameraMatrix(MatrixRowMajor4x3f *out_view_matrix) const = 0;
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
            constexpr LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up) : Camera(), m_pos(CastVector3d(pos)), m_at(CastVector3d(at)), m_up(up) {/*...*/}
            constexpr virtual ~LookAtCamera() override {/*...*/}

            constexpr virtual void CalculateCameraMatrix(MatrixRowMajor4x3f *out_view_matrix) const override {

                /* Calculate normalized direction */
                const Vector3f inv_dir = this->GetInverseDirection();

                /* Calculate normalized right */
                const Vector3f right = Cross(m_up, inv_dir);
                const Vector3f norm_right = Normalize(right);

                /* Calculate normalized up */
                const Vector3f up = Cross(inv_dir, norm_right);

                /* Calculate position */
                const Vector3d d_right = CastVector3d(norm_right);
                const Vector3d d_up = CastVector3d(up);
                const Vector3d d_dir = CastVector3d(inv_dir);
                const double pos_x = m_pos[0];
                const double pos_y = m_pos[1];
                const double pos_z = m_pos[2];
                const Vector3f pos = {
                    -static_cast<float>(d_right[0] * pos_x + d_right[1] * pos_y + d_right[2] * pos_z),
                    -static_cast<float>(d_up[0]    * pos_x + d_up[1]    * pos_y + d_up[2]    * pos_z),
                    -static_cast<float>(d_dir[0]   * pos_x + d_dir[1]   * pos_y + d_dir[2]   * pos_z),
                    0.0f,
                };

                /* Set matrix */
                StoreColumn(out_view_matrix, norm_right.v, 0);
                StoreColumn(out_view_matrix, up.v, 1);
                StoreColumn(out_view_matrix, inv_dir.v, 2);
                StoreColumn(out_view_matrix, pos.v, 3);

                return;
            }

            template <typename V>
                requires (V::cIsVectorType && 3 <= V::cVectorCount)
            constexpr void SetPosition(const V &new_pos) {
                const Vector3d next = CastVector3d(new_pos);
                m_pos = next;
            }

            template <typename V>
                requires (V::cIsVectorType && 3 <= V::cVectorCount)
            constexpr void SetAt(const V &new_at) {
                const Vector3d next = CastVector3d(new_at);
                m_at = next;
            }

            template <typename V>
                requires (V::cIsVectorType && 3 <= V::cVectorCount)
            constexpr void SetUp(const V &new_up) {
                const Vector3f cast = CastVector3f(new_up);
                const Vector3f next = Normalize(cast);
                m_up = next;
            }

            constexpr Vector3f GetPosition() const { return CastVector3f(m_pos); }
            constexpr Vector3f GetInverseDirection() const {
                Vector3f dir = CastVector3f(m_pos - m_at);
                dir = Normalize(dir);
                return dir;
            }
            constexpr Vector3f GetDirection() const {
                Vector3f dir = CastVector3f(m_at - m_pos);
                dir = Normalize(dir);
                return dir;
            }
            constexpr Vector3f GetAt() const {  return CastVector3f(m_at); }
            constexpr Vector3f GetUp() const {  return m_up; }

            constexpr Vector3f GetRightByMatrix() const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                return Vector3f { (*camera_mtx)[1,0], (*camera_mtx)[2,0], (*camera_mtx)[3,0], };
            }

            constexpr Vector3f GetAtByMatrix() const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                return Vector3f { (*camera_mtx)[1,1], (*camera_mtx)[2,1], (*camera_mtx)[3,1], };
            }

            constexpr Vector3f GetUpByMatrix() const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                return Vector3f { (*camera_mtx)[1,2], (*camera_mtx)[2,2], (*camera_mtx)[3,2], };
            }

            constexpr Vector3f GetWorldPositionByMatrix() const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                const Vector3f world_pos = {
                    (-((*camera_mtx)[0,0] * (*camera_mtx)[3,0]) - ((*camera_mtx)[0,1] * (*camera_mtx)[3,1])) - ((*camera_mtx)[0,2] * (*camera_mtx)[3,2]),
                    (-((*camera_mtx)[1,0] * (*camera_mtx)[3,0]) - ((*camera_mtx)[1,1] * (*camera_mtx)[3,1])) - ((*camera_mtx)[1,2] * (*camera_mtx)[3,2]),
                    (-((*camera_mtx)[2,0] * (*camera_mtx)[3,0]) - ((*camera_mtx)[2,1] * (*camera_mtx)[3,1])) - ((*camera_mtx)[2,2] * (*camera_mtx)[3,2]),
                };
                return world_pos;
            }

            constexpr Vector3f GetWorldPositionFromCameraPositionByMatrix(const Vector3f& camera_pos) const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                const Vector3f world_pos = {
                    camera_pos[0] + camera_pos[1] + camera_pos[2] + (-((*camera_mtx)[0,0] * (*camera_mtx)[3,0]) - ((*camera_mtx)[0,1] * (*camera_mtx)[3,1])) - ((*camera_mtx)[0,2] * (*camera_mtx)[3,2]),
                    camera_pos[0] + camera_pos[1] + camera_pos[2] + (-((*camera_mtx)[1,0] * (*camera_mtx)[3,0]) - ((*camera_mtx)[1,1] * (*camera_mtx)[3,1])) - ((*camera_mtx)[1,2] * (*camera_mtx)[3,2]),
                    camera_pos[0] + camera_pos[1] + camera_pos[2] + (-((*camera_mtx)[2,0] * (*camera_mtx)[3,0]) - ((*camera_mtx)[2,1] * (*camera_mtx)[3,1])) - ((*camera_mtx)[2,2] * (*camera_mtx)[3,2]),
                };
                return world_pos;
            }
    };
}
