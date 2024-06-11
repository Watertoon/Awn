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

namespace awn::para {

    struct ComponentListBase_Layout {
        const char               *Category   = "";
        PropertyMap<const char*>  Components = {};
        const char               *Label      = "";

        AWN_PARA_LAYOUT_TYPE_TRAITS(ComponentListBase_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__ComponentListBase, ComponentListBase,
        awn::para::TypedParam, ComponentListBase_Layout,
        Label,
        Category,
        Components
    );
}
