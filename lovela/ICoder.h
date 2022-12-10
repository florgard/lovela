#pragma once
#include "IParser.h"

struct ICoder : IEnumerator<Node>
{
	using OutputT = std::ostream;

	virtual void Code() noexcept = 0;
};

template <class CoderT, class NodeRangeT = IParser::OutputT>
using BasicRangeCoder = RangeEnumerator<CoderT, NodeRangeT>;
