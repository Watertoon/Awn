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
            constexpr LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up) : Camera(), m_pos(CastVector3d(pos)), m_at(CastVector3d(at)), m_up(up) {/*...*/}
            constexpr virtual ~LookAtCamera() override {/*...*/}

            constexpr virtual void CalculateCameraMatrix(MatrixRowMajor4x3f *out_view_matrix) const override {

                /* Calculate normalized direction */
                Vector3f dir = CastVector3f(m_pos - m_at);
                dir = Normalize(dir);

                /* Calculate normalized right */
                Vector3f right = Cross(m_up, dir);
                right = Normalize(right);

                /* Calculate normalized up */
                const Vector3f up = Cross(dir, right);

                /* Set matrix */
                (*out_view_matrix)[0,0] = right[0];
                (*out_view_matrix)[1,0] = right[1];
                (*out_view_matrix)[2,0] = right[2];
                (*out_view_matrix)[0,1] = up[0];
                (*out_view_matrix)[1,1] = up[1];
                (*out_view_matrix)[2,1] = up[2];
                (*out_view_matrix)[0,2] = dir[0];
                (*out_view_matrix)[1,2] = dir[1];
                (*out_view_matrix)[2,2] = dir[2];

                /* Calculate position inline */
                (*out_view_matrix)[3,0] = -(right[0] * m_pos[0] + right[1] * m_pos[1] + right[2] * m_pos[2]);
                (*out_view_matrix)[3,1] = -(up[0]    * m_pos[0] + up[1]    * m_pos[1] + up[2]    * m_pos[2]);
                (*out_view_matrix)[3,2] = -(dir[0]   * m_pos[0] + dir[1]   * m_pos[1] + dir[2]   * m_pos[2]);

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
            constexpr void SetAt(const Vector3f &new_at) {
                const Vector3d next = CastVector3d(new_at);
                m_pos = next;
            }

            template <typename V>
                requires (V::cIsVectorType && 3 <= V::cVectorCount)
            constexpr void SetUp(const V &new_up) {
                const Vector3f cast = CastVector3f(new_up);
                const Vector3f next = Normalize(cast);
                m_up = next;
            }

            constexpr Vector3f GetPosition() const { return CastVector3f(m_pos); }
            constexpr Vector3f GetAt() const {  return CastVector3f(m_at); }
            constexpr Vector3f GetUp() const {  return m_up; }

            constexpr Vector3f GetRightByMatrix() const {
                const MatrixRowMajor4x3f *camera_mtx = this->GetCameraMatrix();
                return Vector3f { (*camera_mtx)[1,0], (*camera_mtx)[2,0], (*camera_mtx)[3,0], };
            }

            constexpr Vector3f GetDirectionByMatrix() const {
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
