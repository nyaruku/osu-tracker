#pragma once
// HELPER FUNCTIONS
namespace Core::Helpers {
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

	static long double scoreNeeded(int n) {
		if (n <= 100) {
			return (5000.0L / 3.0L) * (4.0L * n * n * n - 3.0L * n * n - n)
				+ 1.25L * powl(1.8L, n - 60);
		}
		else {
			return 26931190827.0L + 99999999999.0L * (n - 100);
		}
	}
	static long double getLevelFromScore(long double totalScore) {
		if (totalScore < 0)
			return 1;

		int low = 1, high = 1500;
		int level = 1;

		while (low <= high) {
			int mid = (low + high) / 2;
			if (scoreNeeded(mid) <= totalScore) {
				level = mid;
				low = mid + 1;
			}
			else {
				high = mid - 1;
			}
		}

		long double baseScore = scoreNeeded(level);
		long double nextScore = scoreNeeded(level + 1);
		long double fraction = (totalScore - baseScore) / (nextScore - baseScore);

		return std::round((level + fraction) * 1000.0L) / 1000.0L;
	}
};