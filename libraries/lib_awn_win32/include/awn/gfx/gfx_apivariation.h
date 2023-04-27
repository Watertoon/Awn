#pragma once

namespace awn::gfx {

	enum class ApiVariation {
		Vulkan,
		Pica200,
        Flipper,
        Hollywood,
        Latte,
        Maxwell
	};

    constexpr inline ApiVariation cApiVariation = static_cast<ApiVariation>(VP_TARGET_GRAPHICS_API);
}
