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
#include <awn.hpp>

namespace awn::para {

    Result DocumentBinderAsync::LoadDocumentAsync(const DocumentBinderAsyncInfo *info) {
        impl::DocumentQueue *queue = impl::DocumentQueue::GetInstance();
        RESULT_RETURN_IF(queue == nullptr, ResultNoInstance);
        if (info->path_type == DocumentPathType::Path) {
            res::MaxPathString str;
            str.Append(info->path);
            str.Append(".");
            str.Append(info->class_name);
            str.Append(".bgyml");
            return queue->LoadDocumentAsync(this, str.GetString(), info->is_allow_dummy);
        } else if (info->path_type == DocumentPathType::NameAndClass) {
            return queue->LoadDocumentAsync(this, info->path, info->is_allow_dummy);
        }
        RESULT_RETURN(ResultInvalidPathType);
    }

    void DocumentBinderAsync::WaitForCompletion() {
        if (m_watcher.HasTask() && m_watcher.IsPending()) {
            m_watcher.WaitForCompletion();
        }
        return;
    }
}
