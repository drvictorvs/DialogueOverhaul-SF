# Dialogue Overhaul SF

An SFSE plugin for Starfield that makes conversations less confining:

- **No dialogue close-up:** the camera stays in first or third person, as it was, with no zoom or FOV change when a conversation starts.
- **Move and look while talking:** walk, run, sprint, jump, crouch and turn the camera during a conversation.

Each can be turned off on its [OSF Settings](https://github.com/ozooma10/osf-settings-slim) page or in `Data/SFSE/Plugins/DialogueOverhaulSF.ini`.

Status: 0.3.0. The camera part is confirmed in game (0.1.0); moving while talking (0.2.0) and the OSF Settings page are not yet tested. Planned: dialogue options chosen with the number keys, with W/A/S/D left to movement (a patch for the dialogue menu).

Requires [SFSE](https://www.nexusmods.com/starfield/mods/106) and [Address Library for SFSE Plugins](https://www.nexusmods.com/starfield/mods/3256). Built for Starfield 1.16.244. Optional: [OSF Settings](https://github.com/ozooma10/osf-settings-slim) 1.0.0, for an in-game settings page.

## Settings

| OSF Settings | INI (`[General]`) | Default | Does |
|---|---|---|---|
| Keep the normal camera | `bDisableDialogueCamera` | on | No dialogue close-up or FOV change when a conversation starts |
| Move while talking | `bAllowMovement` | on | Walk, run, sprint, jump and crouch during conversations |
| Look around while talking | `bAllowLooking` | on | Turn the camera with the mouse during conversations |

With OSF Settings installed, its page (Dialogue Overhaul) supplies the values and the INI is only a fallback. Changes there apply while the game runs: the camera setting at once, the controls at the next conversation. If part of the plugin can't run on your game version, it says so in OSF Settings' Mod Issues tab, as well as in the log. Without OSF Settings, the INI is read when the game starts.

## How it works

**Camera.** The game switches to the dialogue camera by asking the player camera to change to camera state 17 (`DialogueCameraState`). The plugin puts a jump at the start of the engine's camera-state switch (Address Library ID 113375): requests for state 17 are dropped, and every other request goes to the original function unchanged. During a conversation the game repeats the request every frame, so the log notes only the first of each run.

Before patching, the plugin checks that the Address Library has the ID and that the function's first 32 bytes are the expected ones. If either check fails (another game version, or another plugin already hooking the function), it logs the reason and leaves the camera alone.

**Controls.** During a conversation, the game disables controls through input-enable layers: the dialogue menu's own layer ("DialogueMenu Input Layer"), and the dialogue camera's ("DialogueCamera Input Layer"). Which one does the work depends on the game's own Settings > Dialogue Camera option. Besides its layers, the game's input-enable manager keeps a mask of force-enabled controls that applies over them; the game's `DumpInputEnableLayers` console command (`diel`) lists both. While the dialogue menu is open, the plugin adds walking, looking, jumping, crouching, running and sprinting to that mask, and when the menu closes it removes exactly what it added.

The log is at `Documents\My Games\Starfield\SFSE\Logs\DialogueOverhaulSF.log`. It records the controls' state when each conversation starts and half a second in.

Note for other plugin authors: in 1.16.244 the dialogue state is 17, while CommonLibSF's `CameraState::kDialogue` is 16 (the game's states are one higher than CommonLibSF's list from there on). CommonLibSF's `PlayerCamera` function IDs are also out of date for this version.

## Build

Requires Visual Studio 2022 Build Tools (C++), XMake 3.0+ and Git. CommonLibSF (OSF's fork) is expected next to this project, and `xmake.lua` includes it as `../commonlibsf`:

```
...\Projects\commonlibsf
...\Projects\DialogueOverhaul-SF   <- this folder
```

In this folder:

```powershell
xmake f -c -m releasedbg
xmake build -v
```

`DialogueOverhaulSF.dll` and `.pdb` land in `build\windows\x64\releasedbg\`. With `XSE_SF_MODS_PATH` (or `XSE_SF_GAME_PATH`) set, `xmake install` copies them, the INI and the OSF Settings schema into place. Otherwise, make a mod with:

```
Data/SFSE/Plugins/DialogueOverhaulSF.dll
Data/SFSE/Plugins/DialogueOverhaulSF.pdb
Data/SFSE/Plugins/DialogueOverhaulSF.ini
Data/SFSE/Plugins/OSF/Settings/schemas/dialogueoverhaul-sf.json
```

`src/vendor/` holds the OSF Settings SDK headers (see its README); the plugin builds and runs without OSF Settings installed.

## Credits

- Jonathan Feenstra, [DisableDialogueCameraSFSE](https://github.com/JonathanFeenstra/DisableDialogueCameraSFSE) (GPL-3.0): the idea of filtering the dialogue camera state out of the camera-state switch.
- AntoniX35, Disable Dialogue Camera, which showed it could be done.
- ozooma10, [OSF Settings](https://github.com/ozooma10/osf-settings-slim) (GPL-3.0): the settings page and Mod Issues, through its SDK in `src/vendor/`.

## License

GPL-3.0.
