#pragma once

auto f_success(const auto& in)
{
	auto& var1 = in; var1;
	using Out = std::remove_reference_t<decltype(var1)>;
	return std::variant<Out, lovela_error>(var1);
}

auto f_error(const auto& in)
{
	auto& var1 = in; var1;
	using Out = std::remove_reference_t<decltype(var1)>;
	auto error = lovela_error{ .code = 1, .message = L"error" };
	return std::variant<Out, lovela_error>(error);
}
