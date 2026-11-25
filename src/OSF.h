#pragma once

// Optional OSF Settings integration (https://github.com/ozooma10/osf-settings-slim).
//
// With OSFSettings.dll loaded, the settings page from
// Data/SFSE/Plugins/OSF/Settings/schemas/dialogueoverhaul-sf.json supplies the values (the INI is
// then only a fallback for any value OSF can't give), changes apply while the game runs, and
// problems are listed in its Mod Issues tab. Without it, the INI is used as before.
namespace DO::OSF
{
	inline constexpr const char* kModID = "dialogueoverhaul-sf";

	// Records a problem for the Mod Issues tab. Safe to call before Connect: reports are queued
	// and sent when OSF Settings is reached. Also written to the log by the caller.
	void ReportIssue(const char* a_id, std::string a_title, std::string a_impact, std::string a_nextSteps, bool a_error = true);

	// At SFSE kPostLoad: attaches to OSF Settings if it's loaded, subscribes to changes, reads the
	// values, and sends queued issues. Logs whether OSF Settings is in use.
	void Connect();

	// On the game thread, once per frame: applies values changed in OSF Settings since last time.
	void ApplyChanges();
}
