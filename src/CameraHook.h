#pragma once

namespace DO::CameraHook
{
	// Puts a jump at the start of PlayerCamera::SetCameraState that drops requests for the dialogue
	// camera. False (and logged) if the ID is missing or the function isn't the expected code.
	bool Install();

	// Turns the dropping on or off while the game runs (OSF Settings); the hook stays in place and
	// passes every request through while off. Safe from any thread.
	void SetEnabled(bool a_enabled);

	// Dialogue camera requests dropped since the game started. The game repeats the request every
	// frame while a conversation is on, so this grows by about 60 a second during one.
	std::uint64_t SkippedCount();
}
