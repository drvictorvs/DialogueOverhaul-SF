#pragma once

// Address Library safety net (same approach as MoreInformativeConsole-SF).
//
// CommonLibSF resolves an ID the first time it is used. If the ID is missing for the running game
// version, REL::IDDB::offset() calls REX::FAIL, which ends the game. So every ID this plugin uses is
// checked once at load, without going through offset(); if any is missing, the plugin stays inactive.

namespace DO::Address
{
	// True when the Address Library loads and the ID is non-zero and has an offset in it.
	// Never calls REX::FAIL.
	bool IsPresent(std::uint64_t a_id);

	struct Need
	{
		std::uint64_t id;
		const char*   name;
	};

	// True when every ID is present; logs each missing one under the feature's name.
	bool AllPresent(std::string_view a_feature, std::initializer_list<Need> a_needs);
}
