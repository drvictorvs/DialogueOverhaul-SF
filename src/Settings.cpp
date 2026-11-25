#include "Settings.h"

namespace DO
{
	namespace
	{
		std::string Trim(std::string_view a_text)
		{
			const auto first = a_text.find_first_not_of(" \t\r\n");
			if (first == std::string_view::npos) {
				return {};
			}
			const auto last = a_text.find_last_not_of(" \t\r\n");
			return std::string(a_text.substr(first, last - first + 1));
		}

		std::string Lower(std::string a_text)
		{
			std::ranges::transform(a_text, a_text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return a_text;
		}
	}

	void Settings::Load()
	{
		struct Option
		{
			const char* section;
			const char* key;
			bool*       target;
		};
		const Option options[]{
			{ "General", "bDisableDialogueCamera", &disableDialogueCamera },
			{ "General", "bAllowMovement", &allowMovement },
			{ "General", "bAllowLooking", &allowLooking },
		};

		const auto    path = std::filesystem::path("Data/SFSE/Plugins/DialogueOverhaulSF.ini");
		std::ifstream file(path);
		if (!file) {
			REX::INFO("No {} found; using defaults", path.string());
			return;
		}

		std::string section;
		std::string line;
		while (std::getline(file, line)) {
			if (const auto comment = line.find_first_of(";#"); comment != std::string::npos) {
				line.erase(comment);
			}
			line = Trim(line);
			if (line.empty()) {
				continue;
			}
			if (line.front() == '[' && line.back() == ']') {
				section = Lower(Trim(std::string_view(line).substr(1, line.size() - 2)));
				continue;
			}
			const auto eq = line.find('=');
			if (eq == std::string::npos) {
				continue;
			}
			const auto key = Lower(Trim(std::string_view(line).substr(0, eq)));
			const auto value = Lower(Trim(std::string_view(line).substr(eq + 1)));
			for (const auto& opt : options) {
				if (Lower(opt.section) == section && Lower(opt.key) == key) {
					*opt.target = value == "1" || value == "true" || value == "on" || value == "yes";
					break;
				}
			}
		}
	}
}
