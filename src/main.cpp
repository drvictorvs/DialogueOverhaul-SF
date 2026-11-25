#include "CameraHook.h"
#include "DialogueControls.h"
#include "Settings.h"

namespace
{
	bool g_controls = false;

	void OnMessage(SFSE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg || a_msg->type != SFSE::MessagingInterface::kPostDataLoad || !g_controls) {
			return;
		}
		const auto* tasks = SFSE::GetTaskInterface();
		if (!tasks) {
			REX::ERROR("SFSE task interface unavailable; moving during dialogue is off");
			return;
		}
		// Runs once per frame on the game thread (SFSE's command-processing hook).
		tasks->AddPermanentTask([]() { DO::DialogueControls::OnFrame(); });
		REX::INFO("Dialogue controls ready");
	}
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse, { .trampoline = true, .trampolineSize = 64 });

	REX::INFO("{} {} loading", SFSE::GetPluginName(), SFSE::GetPluginVersion().string());
	const auto& settings = DO::Settings::Get();
	DO::Settings::Get().Load();

	if (settings.disableDialogueCamera) {
		DO::CameraHook::Install();
	}

	// Every ID is checked up front: a missing one turns the feature off instead of ending the game.
	g_controls = (settings.allowMovement || settings.allowLooking) && DO::DialogueControls::Available();
	if (g_controls) {
		const auto* messaging = SFSE::GetMessagingInterface();
		if (!messaging || !messaging->RegisterListener(OnMessage)) {
			REX::ERROR("Could not register for SFSE messages; moving during dialogue is off");
		}
	}
	return true;
}
