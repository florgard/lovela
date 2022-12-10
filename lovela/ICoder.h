#pragma once
#include "IParser.h"

struct ICoder : IEnumerator<Node>
{
	using OutputT = std::ostream;

	virtual void InitializeOutput(OutputT& output) noexcept = 0;
	virtual void Code() noexcept = 0;
	virtual void Visit(Node& node) noexcept = 0;
};

template <class CoderT, class NodeRangeT = IParser::OutputT>
using BasicRangeCoder = RangeEnumerator<CoderT, NodeRangeT>;
