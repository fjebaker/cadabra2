#include <algorithm> 
#include <cctype>
#include <locale>
#include <string>
#include <vector>

inline std::string ltrim(std::string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
	return s;
}

inline std::string rtrim(std::string s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}

inline std::string trim(std::string s)
{
	return ltrim(rtrim(s));
}
inline std::vector<std::string> string_to_vec(const std::string& s)
{
	std::vector<std::string> v;
	auto pos = s.begin();
	while (pos != s.end()) {
		auto it = std::find(pos, s.end(), '\n');
		if (it != s.end())
			++it;
		v.push_back(std::string(pos, it));
		pos = it;
	}
	return v;
}

inline std::string nth_line(const std::string& s, size_t n)
{
	size_t pos = 0;
	while (n > 0) {
		pos = s.find('\n', pos) + 1;
		if (pos == 0)
			return "";
		--n;
	}
	return s.substr(pos, s.find('\n', pos) - pos);
}

inline std::string escape_backslashes(std::string s)
{
	size_t start = 0;
	while ((start = s.find('\\', start)) != std::string::npos) {
		s.replace(start, 1, "\\\\");
		start += 2;
	}
	return s;
}

inline void replace_all(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}
