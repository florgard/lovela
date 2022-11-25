#pragma once

enum class LexerError
{
	NoError,
	InternalError,
	SyntaxError,
	CommentOpen,
	StringLiteralOpen,
	StringFieldIllformed,
	StringFieldUnknown,
	StringInterpolationOverflow,
};
