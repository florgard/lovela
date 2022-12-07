#pragma once
#include "IParser.h"

struct ICoder
{
	using OutputT = std::ostream;

	virtual void Initialize(OutputT& output) noexcept = 0;
	virtual void Visit(Node& node) noexcept = 0;
};

template <class NodeRangeT, class CoderT = ICoder>
struct BasicTraverser
{
	NodeRangeT nodes;
	CoderT& coder;

	void Traverse()
	{
		::Traverse<Node>::DepthFirstPostorder(nodes, [&](Node& node) { coder.Visit(node); });
	}
};

inline BasicTraverser<std::vector<Node>&> operator>>(std::vector<Node>& nodes, ICoder& coder)
{
	return { nodes, coder };
}

inline void operator>>(BasicTraverser<std::vector<Node>&>&& input, ICoder::OutputT& output)
{
	input.coder.Initialize(output);
	input.Traverse();
}

inline BasicTraverser<IParser::OutputT> operator>>(IParser::OutputT&& nodes, ICoder& coder)
{
	return { std::move(nodes), coder };
}

inline void operator>>(BasicTraverser<IParser::OutputT>&& input, ICoder::OutputT& output)
{
	input.coder.Initialize(output);
	input.Traverse();
}
