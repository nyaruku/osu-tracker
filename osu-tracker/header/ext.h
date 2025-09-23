#pragma once
// HELPER FUNCTIONS
class ext {
public:
	bool isNumeric(const std::string& str) {
		return std::all_of(str.begin(), str.end(), ::isdigit);
	}

	static std::string formatNumber(long long num) {
		std::string str = std::to_string(num);
		int n = str.length();
		for (int i = n - 3; i > 0; i -= 3) {
			str.insert(i, ".");
		}
		return str;
	}

	static std::string bool2str(bool b) {
		return b ? "true" : "false";
	}

	static bool str2bool(std::string s) {
		if (s == "true")
			return true;
		return false;
	}

	// split a string by a char
	static std::vector<std::string> split2vector(std::string str, char delimiter) {
		std::vector<std::string> result;
		size_t start = 0;
		size_t end = str.find(delimiter);
		while (end != std::string::npos) {
			result.push_back(str.substr(start, end - start));
			start = end + 1;
			end = str.find(delimiter, start);
		}
		result.push_back(str.substr(start));
		return result;
	}

	static std::tuple<std::string, std::string> split2tuple(std::string str, char delimiter) {
		size_t pos = str.find(delimiter);
		if (pos == std::string::npos) {
			return { str, "" };
		}
		return { str.substr(0, pos), str.substr(pos + 1) };
	}

	static inline std::string replace(std::string str, const std::string from, const std::string to) {
		if (from.empty())
			return str;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
		return str;
	}
};