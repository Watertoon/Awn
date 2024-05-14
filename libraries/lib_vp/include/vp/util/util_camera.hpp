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
            constexpr Camera() : m_camera_mtx() {/*...*/}
            constexpr virtual ~Camera() {/*...*/}

            void UpdateCameraMatrix() {
                this->CalculateCameraMatrix(std::addressof(m_camera_mtx));
            }

            constexpr MatrixRowMajor4x3f *GetCameraMatrix() {
                return std::addressof(m_camera_mtx);
            }

            constexpr const MatrixRowMajor4x3f *GetCameraMatrix() const {
                return std::addressof(m_camera_mtx);
            }

            constexpr inline virtual void CalculateCameraMatrix(MatrixRowMajor4x3f *out_view_matrix) const = 0;
    };

    class LookAtCamera : public Camera {
        private:
            Vector3d  m_pos;
            Vector3d  m_at;
            Vector3f  m_up;
        public:
            VP_RTTI_DERIVED(LookAtCamera, Camera);
        public:
            constexpr LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up) : Camera(), m_pos(CastVector(pos)), m_at(CastVector(at)), m_up(up) {/*...*/}
            constexpr virtual ~LookAtCamera() override {/*...*/}

            constexpr virtual void CalculateCameraMatrix(MatrixRowMajor4x3f *out_view_matrix) const override {

                /* Calculate normalized direction */
                Vector3f dir = CastVector(m_pos - m_at);
                dir          = Normalize(dir);

                /* Calculate normalized right */
                Vector3f right = Cross(m_up, dir);
                right          = Normalize(right);

                /* Calculate normalized up */
                const Vector3f up = Cross(dir, right);

                /* Set matrix */
                out_view_matrix->array_2d[0][0] = right.x;
                out_view_matrix->array_2d[1][0] = right.y;
                out_view_matrix->array_2d[2][0] = right.z;
                out_view_matrix->array_2d[0][1] = up.x;
                out_view_matrix->array_2d[1][1] = up.y;
                out_view_matrix->array_2d[2][1] = up.z;
                out_view_matrix->array_2d[0][2] = dir.x;
                out_view_matrix->array_2d[1][2] = dir.y;
                out_view_matrix->array_2d[2][2] = dir.z;

                /* Calculate position inline */
                out_view_matrix->array_2d[3][0] = -(right.x * m_pos.x + right.y * m_pos.y + right.z * m_pos.z);
                out_view_matrix->array_2d[3][1] = -(up.x    * m_pos.x + up.y    * m_pos.y + up.z    * m_pos.z);
                out_view_matrix->array_2d[3][2] = -(dir.x   * m_pos.x + dir.y   * m_pos.y + dir.z   * m_pos.z);

                return;
            }

            constexpr ALWAYS_INLINE void SetPosition(const Vector3f &new_pos) { m_pos = CastVector(new_pos); }
            constexpr ALWAYS_INLINE void SetAt(const Vector3f &new_at)        { m_at  = CastVector(new_at); }

            constexpr ALWAYS_INLINE void SetUp(const Vector3f &new_up) {
                m_up = Normalize(new_up);
            }

            constexpr ALWAYS_INLINE void GetPosition(Vector3f *out_pos) const { *out_pos = CastVector(m_pos); }
            constexpr ALWAYS_INLINE void GetAt(Vector3f *out_at)        const { *out_at  = CastVector(m_at); }
            constexpr ALWAYS_INLINE void GetUp(Vector3f *out_up)        const { *out_up  = m_up; }

            constexpr void GetRightByMatrix(Vector3f *out_right_vec) const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                out_right_vec->x = camera_mtx->array_2d[0][0];
                out_right_vec->y = camera_mtx->array_2d[1][0];
                out_right_vec->z = camera_mtx->array_2d[2][0];
            }

            constexpr void GetDirectionByMatrix(Vector3f *out_look_dir_vec) const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                out_look_dir_vec->x = camera_mtx->array_2d[0][1];
                out_look_dir_vec->y = camera_mtx->array_2d[1][1];
                out_look_dir_vec->z = camera_mtx->array_2d[2][1];
            }

            constexpr void GetUpByMatrix(Vector3f *out_up_vec) const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                out_up_vec->x = camera_mtx->array_2d[0][2];
                out_up_vec->y = camera_mtx->array_2d[1][2];
                out_up_vec->z = camera_mtx->array_2d[2][2];
            }

            constexpr void GetWorldPositionByMatrix(Vector3f *out_world_pos_vec) const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                out_world_pos_vec->x = (-(camera_mtx->array_2d[0][0] * camera_mtx->array_2d[3][0]) - (camera_mtx->array_2d[0][1] * camera_mtx->array_2d[3][1])) - (camera_mtx->array_2d[0][2] * camera_mtx->array_2d[3][2]);
                out_world_pos_vec->y = (-(camera_mtx->array_2d[1][0] * camera_mtx->array_2d[3][0]) - (camera_mtx->array_2d[1][1] * camera_mtx->array_2d[3][1])) - (camera_mtx->array_2d[1][2] * camera_mtx->array_2d[3][2]);
                out_world_pos_vec->z = (-(camera_mtx->array_2d[2][0] * camera_mtx->array_2d[3][0]) - (camera_mtx->array_2d[2][1] * camera_mtx->array_2d[3][1])) - (camera_mtx->array_2d[2][2] * camera_mtx->array_2d[3][2]);
            }

            constexpr void GetWorldPositionFromCameraPositionByMatrix(Vector3f *out_world_pos_vec, const Vector3f& camera_pos) const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                out_world_pos_vec->x = camera_pos.x + camera_pos.y + camera_pos.z + (-(camera_mtx->array_2d[0][0] * camera_mtx->array_2d[3][0]) - (camera_mtx->array_2d[0][1] * camera_mtx->array_2d[3][1])) - (camera_mtx->array_2d[0][2] * camera_mtx->array_2d[3][2]);
                out_world_pos_vec->y = camera_pos.x + camera_pos.y + camera_pos.z + (-(camera_mtx->array_2d[1][0] * camera_mtx->array_2d[3][0]) - (camera_mtx->array_2d[1][1] * camera_mtx->array_2d[3][1])) - (camera_mtx->array_2d[1][2] * camera_mtx->array_2d[3][2]);
                out_world_pos_vec->z = camera_pos.x + camera_pos.y + camera_pos.z + (-(camera_mtx->array_2d[2][0] * camera_mtx->array_2d[3][0]) - (camera_mtx->array_2d[2][1] * camera_mtx->array_2d[3][1])) - (camera_mtx->array_2d[2][2] * camera_mtx->array_2d[3][2]);
            }
    };
}
