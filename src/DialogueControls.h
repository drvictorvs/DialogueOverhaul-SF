#pragma once

namespace DO::DialogueControls
{
	// IDs this feature needs (BSInputEnableManager, its lock, UI::IsMenuOpen, BSFixedString).
	bool Available();

	// Runs once per frame on the game thread (an SFSE permanent task). While the dialogue menu is
	// open, force-enables the movement and look controls that the dialogue's input layers disable;
	// when it closes, takes back exactly what it added.
	void OnFrame();
}
