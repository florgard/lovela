#pragma once

struct NameSpace
{
	std::vector<std::string> parts;
	bool root{};

	[[nodiscard]] constexpr auto operator<=>(const NameSpace& rhs) const noexcept = default;

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		if (root)
		{
			stream << Token::Constant::NameSpaceSeparator;
		}

		for (auto& part : parts)
		{
			stream << part << Token::Constant::NameSpaceSeparator;
		}
	}
};

inline std::ostream& operator<<(std::ostream& stream, const NameSpace& nameSpace)
{
	nameSpace.Print(stream);
	return stream;
}
