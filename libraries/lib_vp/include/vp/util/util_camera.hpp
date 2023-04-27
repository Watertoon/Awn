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
            Matrix34f m_camera_mtx;
        public:
            VP_RTTI_BASE(Camera);
        public:
            constexpr Camera() {}

            void UpdateCameraMatrixSelf() {
                this->UpdateCameraMatrix(std::addressof(m_camera_mtx));
            }

            constexpr Matrix34f *GetCameraMatrix() {
                return std::addressof(m_camera_mtx);
            }

            constexpr const Matrix34f *GetCameraMatrix() const {
                return std::addressof(m_camera_mtx);
            }

            constexpr inline virtual void UpdateCameraMatrix(Matrix34f *out_view_matrix) const = 0;
    };

    class LookAtCamera : public Camera {
        private:
            Vector3d  m_pos;
            Vector3d  m_at;
            Vector3f  m_up;
        public:
            VP_RTTI_DERIVED(LookAtCamera, Camera);
        public:
            constexpr LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up) : m_pos(pos), m_at(at), m_up(up) {/*...*/}

            constexpr virtual void UpdateCameraMatrix(Matrix34f *out_view_matrix) const override {

                /* Calculate normalized direction */
                Vector3f dir = m_pos - m_at;
                const float dir_mag = dir.Magnitude();
                if (0.0 < dir_mag) {
                    const float dir_norm = 1.0 / dir_mag;
                    dir = dir * dir_norm;
                }

                /* Calculate normalized right */
                Vector3f right = m_up.Cross(dir);
                const float right_mag = right.Magnitude();
                if (0.0 < right_mag) {
                    const float right_norm = 1.0 / right_mag;
                    right = right * right_norm;
                    
                }

                /* Calculate normalized up */
                const Vector3f up = dir.Cross(right);

                /* Set matrix */
                out_view_matrix->m_arr2d[0][0] = right.x;
                out_view_matrix->m_arr2d[0][1] = right.y;
                out_view_matrix->m_arr2d[0][2] = right.z;
                out_view_matrix->m_arr2d[1][0] = up.x;
                out_view_matrix->m_arr2d[1][1] = up.y;
                out_view_matrix->m_arr2d[1][2] = up.z;
                out_view_matrix->m_arr2d[2][0] = dir.x;
                out_view_matrix->m_arr2d[2][1] = dir.y;
                out_view_matrix->m_arr2d[2][2] = dir.z;

                /* Calculate position inline */
                out_view_matrix->m_arr2d[0][3] = -(right.x * m_pos.x + right.y * m_pos.y + right.z * m_pos.z);
                out_view_matrix->m_arr2d[1][3] = -(up.x    * m_pos.x + up.y    * m_pos.y + up.z    * m_pos.z);
                out_view_matrix->m_arr2d[2][3] = -(dir.x   * m_pos.x + dir.y   * m_pos.y + dir.z   * m_pos.z);
            }

            constexpr ALWAYS_INLINE void SetPos(const Vector3f& new_pos) {
                m_pos = new_pos;
            }

            constexpr ALWAYS_INLINE void SetAt(const Vector3f& new_at) {
                m_at = new_at;
            }

            constexpr ALWAYS_INLINE void SetUp(const Vector3f& new_up) {
                m_up = new_up;
            }

            constexpr ALWAYS_INLINE void GetPos(Vector3f *out_pos) const {
                *out_pos = m_pos;
            }

            constexpr ALWAYS_INLINE void GetAt(Vector3f *out_at) const {
                *out_at = m_at;
            }

            constexpr ALWAYS_INLINE void GetUp(Vector3f *out_up) const {
                *out_up = m_up;
            }

            constexpr void GetRightVectorByMatrix(Vector3f *out_right_vec) const {
                const Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_right_vec->x = camera_mtx->m_arr2d[0][0];
                out_right_vec->y = camera_mtx->m_arr2d[0][1];
                out_right_vec->z = camera_mtx->m_arr2d[0][2];
            }

            constexpr void GetLookDirVectorByMatrix(Vector3f *out_look_dir_vec) const {
                const Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_look_dir_vec->x = camera_mtx->m_arr2d[1][0];
                out_look_dir_vec->y = camera_mtx->m_arr2d[1][1];
                out_look_dir_vec->z = camera_mtx->m_arr2d[1][2];
            }

            constexpr void GetUpVectorByMatrix(Vector3f *out_up_vec) const {
                const Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_up_vec->x = camera_mtx->m_arr2d[2][0];
                out_up_vec->y = camera_mtx->m_arr2d[2][1];
                out_up_vec->z = camera_mtx->m_arr2d[2][2];
            }

            constexpr void GetWorldPosByMatrix(Vector3f *out_world_pos_vec) const {
                const Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_world_pos_vec[0] = (-(camera_mtx->m_arr2d[0][0] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][0] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][0] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[1] = (-(camera_mtx->m_arr2d[0][1] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][1] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][1] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[2] = (-(camera_mtx->m_arr2d[0][2] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][2] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][2] * camera_mtx->m_arr2d[2][3]);
            }

            constexpr void CameraPositionToWorldPositionByMatrix(Vector3f *out_world_pos_vec, const Vector3f& camera_pos) const {
                const Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_world_pos_vec[0] = camera_pos.x + camera_pos.y + camera_pos.z + (-(camera_mtx->m_arr2d[0][0] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][0] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][0] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[1] = camera_pos.x + camera_pos.y + camera_pos.z + (-(camera_mtx->m_arr2d[0][1] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][1] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][1] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[2] = camera_pos.x + camera_pos.y + camera_pos.z + (-(camera_mtx->m_arr2d[0][2] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][2] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][2] * camera_mtx->m_arr2d[2][3]);
            }
    };
}
