#include "CameraHook.h"

#include "AddressCheck.h"
#include "GameIDs.h"

// The game enters the dialogue close-up by calling SetCameraState(camera, 17). The jump at the start
// of SetCameraState drops requests for state 17; every other request goes to the original function
// unchanged. Based on the approach of Jonathan Feenstra's DisableDialogueCameraSFSE (GPL-3.0),
// rewritten for 1.16.244.

namespace DO::CameraHook
{
	namespace
	{
		using SetCameraState_t = void (*)(void* a_camera, std::uint32_t a_state);

		SetCameraState_t           g_callOriginal = nullptr;
		std::atomic<std::uint32_t> g_lastState{ UINT32_MAX };
		std::atomic<std::uint64_t> g_skipped{ 0 };
		std::atomic<std::int64_t>  g_lastSkipMs{ 0 };

		std::int64_t NowMs()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}

		// Called from several game threads.
		void SetCameraStateHook(void* a_camera, std::uint32_t a_state)
		{
			if (a_state == GameID::kDialogueState) {
				g_skipped.fetch_add(1, std::memory_order_relaxed);
				// The request repeats every frame during a conversation: log only the first of a run.
				const auto now = NowMs();
				if (now - g_lastSkipMs.exchange(now) > 2000) {
					REX::INFO("Dialogue camera request skipped (the game repeats it every frame while you talk)");
				}
				return;
			}
			if (g_lastState.exchange(a_state) != a_state) {
				REX::DEBUG("Camera state -> {}", a_state);
			}
			g_callOriginal(a_camera, a_state);
		}
	}

	std::uint64_t SkippedCount()
	{
		return g_skipped.load(std::memory_order_relaxed);
	}

	bool Install()
	{
		using namespace GameID;

		if (!Address::AllPresent("Dialogue camera", { { SetCameraState.id(), "PlayerCamera::SetCameraState" } })) {
			return false;
		}

		const auto target = SetCameraState.address();
		if (std::memcmp(reinterpret_cast<const void*>(target), kSetCameraStateBytes.data(), kSetCameraStateBytes.size()) != 0) {
			REX::ERROR("SetCameraState at {:X} does not match the expected code (a game update or another "
					   "plugin's hook); the dialogue camera is left alone",
				target);
			return false;
		}

		auto& trampoline = REL::GetTrampoline();

		// Stub that runs the original function: the 5 overwritten bytes, then a jump back past them.
		// mov [rsp+8], rbx has no RIP-relative operand, so it can run from anywhere.
		constexpr std::size_t kStubSize = kPrologueSize + 14;
		auto* stub = static_cast<std::uint8_t*>(trampoline.allocate(kStubSize));
		std::memcpy(stub, kSetCameraStateBytes.data(), kPrologueSize);
		const std::uint8_t jmpAbs[6]{ 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };  // jmp [rip+0]
		std::memcpy(stub + kPrologueSize, jmpAbs, sizeof(jmpAbs));
		const std::uint64_t back = target + kPrologueSize;
		std::memcpy(stub + kPrologueSize + sizeof(jmpAbs), &back, sizeof(back));
		g_callOriginal = reinterpret_cast<SetCameraState_t>(stub);

		trampoline.write_jmp<5>(target, &SetCameraStateHook);
		REX::INFO("SetCameraState hook installed at {:X}", target);
		return true;
	}
}
