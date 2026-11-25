#include "CameraHook.h"
#include "DialogueControls.h"
#include "OSF.h"
#include "Settings.h"

namespace
{
	bool g_controls = false;

	void OnFrame()
	{
		DO::OSF::ApplyChanges();
		if (g_controls) {
			DO::DialogueControls::OnFrame();
		}
	}

	void OnMessage(SFSE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg) {
			return;
		}
		switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			// OSF Settings starts its services during its own plugin load; kPostLoad is the earliest
			// point that doesn't depend on load order.
			DO::OSF::Connect();
			break;
		case SFSE::MessagingInterface::kPostDataLoad:
			if (const auto* tasks = SFSE::GetTaskInterface()) {
				// Runs once per frame on the game thread (SFSE's command-processing hook).
				tasks->AddPermanentTask([]() { OnFrame(); });
				REX::INFO("Ready");
			} else {
				REX::ERROR("SFSE task interface unavailable; moving during dialogue and live setting changes are off");
			}
			break;
		default:
			break;
		}
	}
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse, { .trampoline = true, .trampolineSize = 64 });

	REX::INFO("{} {} loading", SFSE::GetPluginName(), SFSE::GetPluginVersion().string());
	auto& settings = DO::Settings::Get();
	settings.Load();

	// Installed whenever its checks pass, so the setting can be switched on and off while the game
	// runs; while off, it passes every camera request through.
	DO::CameraHook::SetEnabled(settings.disableDialogueCamera);
	DO::CameraHook::Install();

	// Every ID is checked up front: a missing one turns the feature off instead of ending the game.
	g_controls = DO::DialogueControls::Available();

	const auto* messaging = SFSE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(OnMessage)) {
		REX::ERROR("Could not register for SFSE messages; OSF Settings, moving during dialogue and live setting changes are off");
	}
	return true;
}
