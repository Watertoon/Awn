#pragma once

namespace vp::res {

    static constexpr u32 cNintendoWareRandomConstant = 0x6c078965;

    constexpr ALWAYS_INLINE u32 CalculateNintendoWareRandomInitialState(u32 seed) {
        return (seed ^ (seed >> 0x1e)) * cNintendoWareRandomConstant;
    }

	struct NintendoWareRandom {
		u32 state0;
		u32 state1;
		u32 state2;
		u32 state3;

		constexpr NintendoWareRandom(u32 seed) : state0(CalculateNintendoWareRandomInitialState(seed) + 1), state1(CalculateNintendoWareRandomInitialState(state0) + 2), state2(CalculateNintendoWareRandomInitialState(state1) + 3), state3(CalculateNintendoWareRandomInitialState(state2) + 4) {/*...*/}
        constexpr ~NintendoWareRandom() {/*...*/}

		constexpr u32 GetU32() {

            /* Calculate value */
			const u32 hash0 = state0 ^ (state0 << 0xb);
			const u32 hash1 = state3;
			const u32 value = hash0 ^ (hash0 >> 8) ^ hash1 ^ (hash1 >> 0x13);

            /* Shift state */
            state0 = state1;
            state1 = state2;
            state2 = state3;
            state3 = value;

			return value;
		}
	};
}
