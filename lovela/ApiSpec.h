#pragma once

struct ApiSpec
{
	static constexpr int None = 0;
	static constexpr int Import = 1 << 0;
	static constexpr int Export = 1 << 1;
	static constexpr int Dynamic = 1 << 2;
	static constexpr int Standard = 1 << 3;
	static constexpr int C = 1 << 4;
	static constexpr int Cpp = 1 << 5;

	constexpr ApiSpec() noexcept = default;
	constexpr ApiSpec(int flags) noexcept : flags(flags) {}

	[[nodiscard]] constexpr auto operator<=>(const ApiSpec& rhs) const noexcept = default;
	
	constexpr void Set(int flag)
	{
		flags |= flag;
	}

	constexpr bool Is(int flag) const
	{
		return (flags & flag) == flag;
	}

	constexpr bool IsExplicit() const
	{
		return flags > Export;
	}

	[[nodiscard]] void Print(std::ostream& stream) const
	{
		stream << '[' << flags << ']';
	}

private:
	int flags{};
};

inline std::ostream& operator<<(std::ostream& stream, const ApiSpec& apiSpec)
{
	apiSpec.Print(stream);
	return stream;
}
