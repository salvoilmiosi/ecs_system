#include "console.h"

namespace console {

std::string_view getArgument(std::string_view args, size_t arg_i) {
	static const char *SPACE = " \t";
	static const char QUOTE = '"';

	size_t first = 0, last = 0;

	// I will use regexps one day

	for (size_t i=0; i<=arg_i; ++i) {
		first = args.find_first_not_of(SPACE, last);
		if (first == std::string_view::npos) {
			return std::string_view();
		}
		if (args[first] == QUOTE) {
			last = args.find(QUOTE, first + 1);
			if (last == std::string_view::npos) {
				return std::string_view();
			}
		} else {
			last = args.find_first_of(SPACE, first);
			if (last == std::string_view::npos) {
				if (i == arg_i) {
					break;
				} else {
					return std::string_view();
				}
			}
		}
	}

	if (args[first] == QUOTE) {
		return args.substr(first + 1, last - first - 1);
	} else {
		return args.substr(first, last - first);
	}
}

std::string_view getCommand(std::string_view cmd) {
	return getArgument(cmd, 0);
}

}