#pragma once

namespace DO
{
	// Values come from Data/SFSE/Plugins/DialogueOverhaulSF.ini.
	struct Settings
	{
		bool disableDialogueCamera{ true };  // keep the normal camera when a conversation starts
		bool allowMovement{ true };          // walk, run, sprint, jump and crouch while talking
		bool allowLooking{ true };           // turn the camera with the mouse while talking

		static Settings& Get()
		{
			static Settings settings;
			return settings;
		}

		void Load();
	};
}
