#pragma once

/* Reverse engineered nnsdk util formats */
#include <vp/res/res_nintendowaredictionary.hpp>
#include <vp/res/res_nintendowarerelocationtable.h>
#include <vp/res/res_nintendowarestringpool.hpp>
#include <vp/res/res_nintendowarefileheader.hpp>

/* Reverse engineered NintendoWare gfx file formats */
#include <vp/res/res_gfxcommon.hpp>
#include <vp/res/res_gfxtostring.hpp>

#ifdef VP_TARGET_GRAPHICS_API_vk
    #include <vp/res/res_gfxtovk.vk.hpp>
#endif

#include <vp/res/res_bntx.hpp>

/* Reverse engineered NintendoWare g3d formats */
#include <vp/res/res_bfresanimcurve.hpp>
#include <vp/res/res_bfresskeleton.hpp>
#include <vp/res/res_bfresmaterial.hpp>
#include <vp/res/res_bfresshape.hpp>
#include <vp/res/res_bfresmodel.hpp>
#include <vp/res/res_bfresskeletalanim.hpp>
#include <vp/res/res_bfresmaterialanim.hpp>
#include <vp/res/res_bfresbonevisibilityanim.hpp>
#include <vp/res/res_bfresshapeanim.hpp>
#include <vp/res/res_bfressceneanim.hpp>
#include <vp/res/res_bfres.hpp>

/* Reverse engineered NintendoWare font formats */
#include <vp/res/res_ui2dcommon.hpp>
#include <vp/res/res_bffnt.hpp>

/* Reverse engineered NintendoWare ui2d formats */
#include <vp/res/res_sarc.hpp>

/* Reverse engineered NintendoWare bezel formats*/
#include <vp/res/res_bea.hpp>

/* Reverse Engineered Nintendo EPD byaml formats */
#include <vp/res/res_byaml.hpp>
#include <vp/res/res_byamlstringtableiterator.hpp>
#include <vp/res/res_byamldictionaryiterator.hpp>
#include <vp/res/res_byamliterator.hpp>

/* Reverse engineered Nintendo EPD resource size table format */
#include <vp/res/res_rsizetable.hpp>
