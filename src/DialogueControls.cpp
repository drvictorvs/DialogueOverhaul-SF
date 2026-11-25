#include "DialogueControls.h"

#include "AddressCheck.h"
#include "CameraHook.h"
#include "GameIDs.h"
#include "OSF.h"
#include "Settings.h"

// BSInputEnableManager keeps, besides the input layers, a "force-enabled" mask for user controls
// (+0x98) and other controls (+0x9C), applied over whatever the layers disable. The game's own
// DumpInputEnableLayers console command (diel) prints them as "Force-enabled controls" next to
// "Layer-disabled controls". CommonLibSF's ForceUserEventEnabled / ForceOtherEventEnabled set them
// under the manager's lock (+0x88), as the game does.

namespace DO::DialogueControls
{
	namespace
	{
		using RE::OTHER_EVENT_FLAG;
		using RE::USER_EVENT_FLAG;

		struct State
		{
			bool             open{ false };
			std::uint32_t    framesOpen{ 0 };
			USER_EVENT_FLAG  addedUser{};
			OTHER_EVENT_FLAG addedOther{};
			std::uint64_t    skippedAtOpen{ 0 };
		};
		State g_state;

		const RE::BSFixedString& DialogueMenu()
		{
			// Never freed: avoids releasing strings after the game's string pool is gone at exit.
			static const auto* name = new RE::BSFixedString("DialogueMenu");
			return *name;
		}

		bool CameraOptionOn()
		{
			static REL::Relocation<std::uint8_t*> option{ GameID::DialogueCameraOption };
			return *option != 0;
		}

		void LogFlags(const char* a_when, const RE::BSInputEnableManager* a_manager)
		{
			REX::INFO("{}: enabled user {:08X} other {:08X}; force-enabled user {:08X} other {:08X}; Dialogue Camera option {}",
				a_when,
				std::to_underlying(a_manager->m_cachedUserEventFlags), std::to_underlying(a_manager->m_cachedOtherEventFlags),
				std::to_underlying(a_manager->m_forcedUserEventFlags), std::to_underlying(a_manager->m_forcedOtherEventFlags),
				CameraOptionOn() ? "on" : "off");
		}

		void OnOpen(RE::BSInputEnableManager* a_manager)
		{
			const auto& settings = Settings::Get();
			g_state.skippedAtOpen = CameraHook::SkippedCount();
			LogFlags("Dialogue opened", a_manager);

			USER_EVENT_FLAG  wantUser{};
			OTHER_EVENT_FLAG wantOther{};
			if (settings.allowMovement) {
				wantUser |= USER_EVENT_FLAG::Walking | USER_EVENT_FLAG::Jumping | USER_EVENT_FLAG::Sneaking;
				wantOther |= OTHER_EVENT_FLAG::Running | OTHER_EVENT_FLAG::Sprinting;
			}
			if (settings.allowLooking) {
				wantUser |= USER_EVENT_FLAG::Looking;
			}

			// Only the bits nobody else has already forced, so closing takes back only ours.
			g_state.addedUser = wantUser & ~a_manager->m_forcedUserEventFlags;
			g_state.addedOther = wantOther & ~a_manager->m_forcedOtherEventFlags;
			if (std::to_underlying(g_state.addedUser) != 0) {
				a_manager->ForceUserEventEnabled(g_state.addedUser, true);
			}
			if (std::to_underlying(g_state.addedOther) != 0) {
				a_manager->ForceOtherEventEnabled(g_state.addedOther, true);
			}
		}

		void OnClose(RE::BSInputEnableManager* a_manager)
		{
			if (std::to_underlying(g_state.addedUser) != 0) {
				a_manager->ForceUserEventEnabled(g_state.addedUser, false);
			}
			if (std::to_underlying(g_state.addedOther) != 0) {
				a_manager->ForceOtherEventEnabled(g_state.addedOther, false);
			}
			g_state.addedUser = {};
			g_state.addedOther = {};

			const auto skipped = CameraHook::SkippedCount() - g_state.skippedAtOpen;
			REX::INFO("Dialogue closed after {} frames; {} dialogue camera requests skipped", g_state.framesOpen, skipped);
		}
	}

	bool Available()
	{
		std::string missing;
		const bool  available = Address::AllPresent("Dialogue controls", {
			{ RE::ID::BSInputEnableManager::Singleton.id(), "BSInputEnableManager::Singleton" },
			{ RE::ID::BSSpinLock::Lock.id(), "BSSpinLock::Lock" },
			{ RE::ID::BSSpinLock::Unlock.id(), "BSSpinLock::Unlock" },
			{ GameID::DialogueCameraOption.id(), "Dialogue Camera option" },
			{ RE::ID::UI::Singleton.id(), "UI::Singleton" },
			{ RE::ID::UI::IsMenuOpen.id(), "UI::IsMenuOpen" },
			{ RE::ID::BSStringPool::GetEntry.id(), "BSStringPool::GetEntry" },
			{ RE::ID::BSStringPool::Entry::Release.id(), "BSStringPool::Entry::Release" },
			{ RE::ID::BSStringPool::BucketTable::GetSingleton.id(), "BSStringPool::BucketTable::GetSingleton" },
		}, &missing);
		if (!available) {
			OSF::ReportIssue("controls-ids", "Moving during conversations is unavailable",
				"Controls stay locked while talking. Missing from the Address Library: " + missing + ".",
				"Install the Address Library for SFSE Plugins that matches your game version, or update Dialogue Overhaul.");
		}
		return available;
	}

	void OnFrame()
	{
		const auto* ui = RE::UI::GetSingleton();
		auto*       manager = RE::BSInputEnableManager::GetSingleton();
		if (!ui || !manager) {
			return;
		}

		const bool open = ui->IsMenuOpen(DialogueMenu());
		if (open && !g_state.open) {
			g_state.framesOpen = 0;
			OnOpen(manager);
		} else if (!open && g_state.open) {
			OnClose(manager);
		}
		g_state.open = open;

		if (open && ++g_state.framesOpen == 30) {
			// The dialogue's own layers are set up as it starts; show where things stand half a second in.
			LogFlags("Dialogue after 30 frames", manager);
		}
	}
}
