#include "OSF.h"

#include "CameraHook.h"
#include "Settings.h"

#include "vendor/OSFSettings.h"
#include "vendor/OSFSettings_Diagnostics.h"

namespace DO::OSF
{
	namespace
	{
		using OSFSettings::API::Status;

		struct Issue
		{
			std::string id;
			std::string title;
			std::string impact;
			std::string nextSteps;
			bool        error{ true };
		};

		OSFSettings::API::Client              g_settings;
		OSFSettings::API::Diagnostics::Client g_diagnostics;
		std::vector<Issue>                    g_pendingIssues;  // before Connect, game thread / load thread only
		std::atomic<bool>                     g_dirty{ false };
		bool                                  g_connected{ false };

		void Send(const Issue& a_issue)
		{
			const auto status = g_diagnostics.Report({
				.modId = kModID,
				.id = a_issue.id.c_str(),
				.severity = a_issue.error ? OSFSettings::API::Diagnostics::Severity::Error : OSFSettings::API::Diagnostics::Severity::Warning,
				.title = a_issue.title.c_str(),
				.impact = a_issue.impact.empty() ? nullptr : a_issue.impact.c_str(),
				.nextSteps = a_issue.nextSteps.empty() ? nullptr : a_issue.nextSteps.c_str(),
			});
			if (status != Status::Ok) {
				REX::WARN("OSF Settings did not accept issue {} (status {})", a_issue.id, std::to_underlying(status));
			}
		}

		void ReadBool(const char* a_key, bool& a_value)
		{
			bool value = a_value;
			if (const auto status = g_settings.GetBool(kModID, a_key, &value); status == Status::Ok) {
				a_value = value;
			} else {
				REX::WARN("OSF Settings: could not read {} (status {}); keeping {}", a_key, std::to_underlying(status), a_value);
			}
		}

		void Read()
		{
			auto& settings = Settings::Get();
			ReadBool("disableDialogueCamera", settings.disableDialogueCamera);
			ReadBool("allowMovement", settings.allowMovement);
			ReadBool("allowLooking", settings.allowLooking);
			CameraHook::SetEnabled(settings.disableDialogueCamera);
			REX::INFO("Settings from OSF Settings: keep normal camera {}, move while talking {}, look while talking {}",
				settings.disableDialogueCamera, settings.allowMovement, settings.allowLooking);
		}

		// Runs on an SFSE task, without a main-thread guarantee: only flag the change here.
		void OnChanged(const char*, const char*, void*) noexcept
		{
			g_dirty.store(true);
		}
	}

	void ReportIssue(const char* a_id, std::string a_title, std::string a_impact, std::string a_nextSteps, bool a_error)
	{
		Issue issue{ a_id, std::move(a_title), std::move(a_impact), std::move(a_nextSteps), a_error };
		if (g_diagnostics) {
			Send(issue);
		} else {
			g_pendingIssues.push_back(std::move(issue));
		}
	}

	void Connect()
	{
		if (g_diagnostics.Init()) {
			for (const auto& issue : g_pendingIssues) {
				Send(issue);
			}
			g_pendingIssues.clear();
		}

		if (!g_settings.Init()) {
			REX::INFO("OSF Settings not found; using the INI");
			return;
		}

		// Subscribe before the first read, so no change falls in between.
		OSFSettings::API::Subscription subscription{};
		if (const auto status = g_settings.Subscribe(kModID, OnChanged, nullptr, &subscription); status != Status::Ok) {
			REX::WARN("OSF Settings: could not subscribe to changes (status {}); values are read once", std::to_underlying(status));
		}
		g_connected = true;

		if (g_settings.IsReady()) {
			Read();
		} else {
			// Its first change notification arrives when it's ready.
			REX::INFO("OSF Settings found but not ready yet; reading values when it is");
		}
	}

	void ApplyChanges()
	{
		if (g_connected && g_dirty.exchange(false)) {
			Read();
		}
	}
}
