#pragma once

// Address Library IDs for Starfield 1.16.244. CommonLibSF's PlayerCamera IDs (166029, 166031,
// 166078, 166081) and its CameraState enum are out of date for this version, so this mod does not
// use them.
namespace DO::GameID
{
	// void (PlayerCamera*, std::uint32_t state). Swaps the active camera state for
	// cameraStates[state] (array at PlayerCamera + 0x188; active state at + 0x10).
	inline constexpr REL::ID SetCameraState{ 113375 };

	// The function that starts the dialogue camera calls SetCameraState(camera, 17).
	// Recorded for reference; not hooked.
	inline constexpr REL::ID StartDialogueCamera{ 113405 };

	// PlayerCamera* (global pointer).
	inline constexpr REL::ID PlayerCameraSingleton{ 937788 };

	// bool: the game's own Settings > "Dialogue Camera" option (added in 1.11.36). Read by the
	// dialogue menu (ID 90371): when it is off, the menu itself disables walking, jumping and more
	// (user-event flags 0xD61) through its "DialogueMenu Input Layer"; when it is on, that is left to
	// the dialogue camera state's own "DialogueCamera Input Layer" (its Begin, ID 74555).
	inline constexpr REL::ID DialogueCameraOption{ 923937 };

	// Camera state index of DialogueCameraState in 1.16.244. CommonLibSF's kDialogue is 16;
	// the game's states from there on are one higher than CommonLibSF's list.
	inline constexpr std::uint32_t kDialogueState = 17;

	// First 32 bytes of SetCameraState in 1.16.244. The hook is installed only if they match,
	// so a game update that changes the function leaves the game untouched.
	// The first 5 bytes (mov [rsp+8], rbx) are copied into the stub that calls the original.
	inline constexpr std::array<std::uint8_t, 32> kSetCameraStateBytes{
		0x48, 0x89, 0x5C, 0x24, 0x08,        // mov [rsp+8], rbx
		0x48, 0x89, 0x6C, 0x24, 0x10,        // mov [rsp+10h], rbp
		0x48, 0x89, 0x74, 0x24, 0x18,        // mov [rsp+18h], rsi
		0x57,                                // push rdi
		0x48, 0x83, 0xEC, 0x30,              // sub rsp, 30h
		0x8B, 0xEA,                          // mov ebp, edx
		0x48, 0x8B, 0xF9,                    // mov rdi, rcx
		0x48, 0x8B, 0x9C, 0xE9, 0x88, 0x01, 0x00  // mov rbx, [rcx+rbp*8+188h]
	};
	inline constexpr std::size_t kPrologueSize = 5;
}
