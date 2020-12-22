#include "stdafx.h"
#include "nodeGraphEditor.h"

#include "CommonLib/Timer.h"
#include "Input/InputInterface.h"
#include "Graphics/Interface/imguiHelper.h"

template <NodeGraphEditor::Type Type, size_t InputAttributeCount, size_t OutputAttributeCount>
struct NodeData
{
	static constexpr NodeGraphEditor::Type GetType() { return Type; }
	static constexpr size_t GetInputAttributeCount() { return InputAttributeCount; }
	static constexpr size_t GetOutputAttributeCount() { return OutputAttributeCount; }

	NodeGraphEditor::InputAttribute CreateInputAttribute(size_t index, int parentID, int id)
	{
		inputNodeIDs[index] = id;
		return NodeGraphEditor::InputAttribute(parentID, id);
	}

	std::array<int, InputAttributeCount> inputNodeIDs{};
	std::array<int, OutputAttributeCount> outputNodeIDs{};
};

template <NodeGraphEditor::Type Type, size_t InputAttributeCount>
struct NodeData<Type, InputAttributeCount, 0>
{
	static constexpr NodeGraphEditor::Type GetType() { return Type; }
	static constexpr size_t GetInputAttributeCount() { return InputAttributeCount; }
	static constexpr size_t GetOutputAttributeCount() { return 0; }

	NodeGraphEditor::InputAttribute CreateInputAttribute(size_t index, int parentID, int id)
	{
		inputNodeIDs[index] = id;
		return NodeGraphEditor::InputAttribute(parentID, id);
	}

	std::array<int, InputAttributeCount> inputNodeIDs{};
};

template <NodeGraphEditor::Type Type, size_t OutputAttributeCount>
struct NodeData<Type, 0, OutputAttributeCount>
{
	static constexpr NodeGraphEditor::Type GetType() { return Type; }
	static constexpr size_t GetInputAttributeCount() { return 0; }
	static constexpr size_t GetOutputAttributeCount() { return OutputAttributeCount; }

	std::array<int, OutputAttributeCount> outputNodeIDs{};
};

////////// 변수 //////////
struct NumberNode : public NodeData<NodeGraphEditor::Type::eFloat, 0, 1>
{
	static constexpr const char* Title{ "number " };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "number" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eNumber, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				values.emplace(outputAttribute.value);
			});
	}
};

//struct NumberNode : public NodeData<NodeGraphEditor::Type::eFloat2, 0, 2>
//{
//	static constexpr const char* Title{ "number " };
//	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "number" };
//
//	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
//	{
//		outputNodeIDs[index] = id;
//		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eNumber, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
//			{
//				values.emplace(outputAttribute.value);
//			});
//	}
//};

////////// 시간 함수 //////////
struct TimeNode : public NodeData<NodeGraphEditor::Type::eTime, 0, 1>
{
	static constexpr const char* Title{ "time" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "time" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				outputAttribute.value = static_cast<float>(est::Timer::GetInstance()->GetGameTime());
				values.emplace(outputAttribute.value);
			});
	}
};

struct ElapsedTimeNode : public NodeData<NodeGraphEditor::Type::eElapsedTime, 0, 1>
{
	static constexpr const char* Title{ "elapsedTime " };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "elapsedTime" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				outputAttribute.value = static_cast<float>(est::Timer::GetInstance()->GetElapsedTime());
				values.emplace(outputAttribute.value);
			});
	}
};

////////// 수학 함수 //////////
struct AddNode : public NodeData<NodeGraphEditor::Type::eAdd, 2, 1>
{
	static constexpr const char* Title{ "add" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "add" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = x + y;
				values.emplace(outputAttribute.value);
			});
	}
};

struct MinusNode : public NodeData<NodeGraphEditor::Type::eMinus, 2, 1>
{
	static constexpr const char* Title{ "minus" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "minus" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = x - y;
				values.emplace(outputAttribute.value);
			});
	}
};

struct MultiplyNode : public NodeData<NodeGraphEditor::Type::eMultiply, 2, 1>
{
	static constexpr const char* Title{ "multiply" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "multiply" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = x * y;
				values.emplace(outputAttribute.value);
			});
	}
};

struct DivideNode : public NodeData<NodeGraphEditor::Type::eDivide, 2, 1>
{
	static constexpr const char* Title{ "divide" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "minus" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = x / y;
				values.emplace(outputAttribute.value);
			});
	}
};

struct SqrtNode : public NodeData<NodeGraphEditor::Type::eSqrt, 1, 1>
{
	static constexpr const char* Title{ "sqrt" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "sqrt" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::sqrt(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct ReciprocalSqrtNode : public NodeData<NodeGraphEditor::Type::eReciprocalSqrt, 1, 1>
{
	static constexpr const char* Title{ "rsqrt" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "rsqrt" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = 1.f / std::sqrt(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct ExpNode : public NodeData<NodeGraphEditor::Type::eExp, 1, 1>
{
	static constexpr const char* Title{ "exp" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "exp" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::exp(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct Exp2Node : public NodeData<NodeGraphEditor::Type::eExp2, 1, 1>
{
	static constexpr const char* Title{ "exp2" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "exp2" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::exp2(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct PowNode : public NodeData<NodeGraphEditor::Type::ePow, 2, 1>
{
	static constexpr const char* Title{ "pow" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "pow" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::pow(x, y);
				values.emplace(outputAttribute.value);
			});
	}
};

struct LdExpNode : public NodeData<NodeGraphEditor::Type::eLdExp, 2, 1>
{
	static constexpr const char* Title{ "ldexp" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "ldexp" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::ldexp(x, static_cast<int>(y));
				values.emplace(outputAttribute.value);
			});
	}
};

struct LogNode : public NodeData<NodeGraphEditor::Type::eLog, 1, 1>
{
	static constexpr const char* Title{ "log" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "log" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::log(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct Log10Node : public NodeData<NodeGraphEditor::Type::eLog10, 1, 1>
{
	static constexpr const char* Title{ "log10" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "log10" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::log10(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct Log2Node : public NodeData<NodeGraphEditor::Type::eLog2, 1, 1>
{
	static constexpr const char* Title{ "log2" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "log2" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::log2(x);
				values.emplace(outputAttribute.value);
			});
	}
};

////////// 값 변환 함수 //////////
struct AbsNode : public NodeData<NodeGraphEditor::Type::eAbs, 1, 1>
{
	static constexpr const char* Title{ "abs" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "abs" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::abs(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct SignNode : public NodeData<NodeGraphEditor::Type::eSign, 1, 1>
{
	static constexpr const char* Title{ "sign" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "sign" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = (x > 0.f ? 1.f : (x < 0.f ? -1.f : 0.f));
				values.emplace(outputAttribute.value);
			});
	}
};

struct CeilNode : public NodeData<NodeGraphEditor::Type::eCeil, 1, 1>
{
	static constexpr const char* Title{ "ceil" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "ceil" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::ceil(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct FloorNode : public NodeData<NodeGraphEditor::Type::eFloor, 1, 1>
{
	static constexpr const char* Title{ "floor" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "floor" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::floor(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct RoundNode : public NodeData<NodeGraphEditor::Type::eRound, 1, 1>
{
	static constexpr const char* Title{ "round" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "round" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::round(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct MinNode : public NodeData<NodeGraphEditor::Type::eMin, 2, 1>
{
	static constexpr const char* Title{ "min" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "min" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::min(x, y);

				values.emplace(outputAttribute.value);
			});
	}
};

struct MaxNode : public NodeData<NodeGraphEditor::Type::eMax, 2, 1>
{
	static constexpr const char* Title{ "max" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "max" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::max(x, y);

				values.emplace(outputAttribute.value);
			});
	}
};

struct ClampNode : public NodeData<NodeGraphEditor::Type::eClamp, 3, 1>
{
	static constexpr const char* Title{ "clamp" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "min", "max" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "clamp" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float max = values.top();
				values.pop();

				const float min = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::clamp(x, min, max);

				values.emplace(outputAttribute.value);
			});
	}
};

struct SaturateNode : public NodeData<NodeGraphEditor::Type::eSaturate, 1, 1>
{
	static constexpr const char* Title{ "saturate" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "saturate" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::clamp(x, 0.f, 1.f);
				values.emplace(outputAttribute.value);
			});
	}
};

struct LerpNode : public NodeData<NodeGraphEditor::Type::eLerp, 3, 1>
{
	static constexpr const char* Title{ "lerp" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y", "s" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "lerp" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float s = values.top();
				values.pop();

				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = est::math::Lerp(x, y, s);
				values.emplace(outputAttribute.value);
			});
	}
};

struct StepNode : public NodeData<NodeGraphEditor::Type::eStep, 2, 1>
{
	static constexpr const char* Title{ "step" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "step" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = x >= y ? 1.f : 0.f;
				values.emplace(outputAttribute.value);
			});
	}
};

struct SmoothstepNode : public NodeData<NodeGraphEditor::Type::eSmoothstep, 3, 1>
{
	static constexpr const char* Title{ "smoothstep" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "min", "max", "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "smoothstep" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				const float max = values.top();
				values.pop();

				const float min = values.top();
				values.pop();

				outputAttribute.value = est::math::Smoothstep(min, max, x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct FModNode : public NodeData<NodeGraphEditor::Type::eFMod, 2, 1>
{
	static constexpr const char* Title{ "fmod" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "fmod" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::fmod(x, y);
				values.emplace(outputAttribute.value);
			});
	}
};

struct FracNode : public NodeData<NodeGraphEditor::Type::eFrac, 1, 1>
{
	static constexpr const char* Title{ "frac" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "frac" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = x - std::trunc(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct FrExpNode : public NodeData<NodeGraphEditor::Type::eFrExp, 1, 2>
{
	static constexpr const char* Title{ "frexp" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "mantissa", "exp" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		
		if (index == 0)
		{
			return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
				{
					const float x = values.top();
					values.pop();

					int exp = 0;
					outputAttribute.value = std::frexp(x, &exp);
					values.emplace(outputAttribute.value);
				});
		}
		else
		{
			return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
				{
					const float x = values.top();
					values.pop();

					int exp = 0;
					std::frexp(x, &exp);
					outputAttribute.value = static_cast<float>(exp);
					values.emplace(outputAttribute.value);
				});
		}
	}
};

struct ModfNode : public NodeData<NodeGraphEditor::Type::eModf, 1, 2>
{
	static constexpr const char* Title{ "modf" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "fraction", "int" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		if (index == 0)
		{
			return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
				{
					const float x = values.top();
					values.pop();

					float integer = 0.f;
					outputAttribute.value = std::modf(x, &integer);
					values.emplace(outputAttribute.value);
				});
		}
		else
		{
			return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
				{
					const float x = values.top();
					values.pop();

					std::modf(x, &outputAttribute.value);
					values.emplace(outputAttribute.value);
				});
		}
	}
};

////////// 삼각 함수 //////////
struct SinNode : public NodeData<NodeGraphEditor::Type::eSin, 1, 1>
{
	static constexpr const char* Title{ "sin" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "sin" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::sin(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct CosNode : public NodeData<NodeGraphEditor::Type::eCos, 1, 1>
{
	static constexpr const char* Title{ "cos" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "cos" };
	
	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::cos(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct TanNode : public NodeData<NodeGraphEditor::Type::eTan, 1, 1>
{
	static constexpr const char* Title{ "tan" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "tan" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::tan(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct ASinNode : public NodeData<NodeGraphEditor::Type::eASin, 1, 1>
{
	static constexpr const char* Title{ "asin" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "asin" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::asin(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct ACosNode : public NodeData<NodeGraphEditor::Type::eACos, 1, 1>
{
	static constexpr const char* Title{ "acos" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "acos" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::acos(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct ATanNode : public NodeData<NodeGraphEditor::Type::eATan, 1, 1>
{
	static constexpr const char* Title{ "atan" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "atan" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::atan(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct ATan2Node : public NodeData<NodeGraphEditor::Type::eATan2, 2, 1>
{
	static constexpr const char* Title{ "atan2" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x", "y" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "atan2" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float y = values.top();
				values.pop();

				const float x = values.top();
				values.pop();

				outputAttribute.value = std::atan2(x, y);
				values.emplace(outputAttribute.value);
			});
	}
};

struct SinHNode : public NodeData<NodeGraphEditor::Type::eSinH, 1, 1>
{
	static constexpr const char* Title{ "sinh" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "sinh" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::sinh(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct CosHNode : public NodeData<NodeGraphEditor::Type::eCosH, 1, 1>
{
	static constexpr const char* Title{ "cosh" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "cosh" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::cosh(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct TanHNode : public NodeData<NodeGraphEditor::Type::eTanH, 1, 1>
{
	static constexpr const char* Title{ "tanh" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "tanh" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = std::tanh(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct DegreesNode : public NodeData<NodeGraphEditor::Type::eDegrees, 1, 1>
{
	static constexpr const char* Title{ "degrees" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "degrees" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = est::math::ToDegrees(x);
				values.emplace(outputAttribute.value);
			});
	}
};

struct RadiansNode : public NodeData<NodeGraphEditor::Type::eRadians, 1, 1>
{
	static constexpr const char* Title{ "radians" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "radians" };

	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
	{
		outputNodeIDs[index] = id;
		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
			{
				const float x = values.top();
				values.pop();

				outputAttribute.value = est::math::ToRadians(x);
				values.emplace(outputAttribute.value);
			});
	}
};

////////// 벡터 함수 //////////
//struct CrossNode : public NodeData<NodeGraphEditor::Type::eCross, 1, 1>
//{
//	static constexpr const char* Title{ "cross" };
//	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "x" };
//	static constexpr std::array<const char*, NodeData::GetOutputAttributeCount()> OutputNodeName = { "cross" };
//
//	NodeGraphEditor::OutputAttribute CreateOutputAttribute(size_t index, int parentID, int id)
//	{
//		outputNodeIDs[index] = id;
//		return NodeGraphEditor::OutputAttribute(parentID, id, NodeGraphEditor::AttributeType::eOperation, [](NodeGraphEditor::OutputAttribute& outputAttribute, std::stack<float>& values)
//			{
//				const float x = values.top();
//				values.pop();
//
//				outputAttribute.value = est::math::ToRadians(x);
//				values.emplace(outputAttribute.value);
//			});
//	}
//};

struct OutputNode : public NodeData<NodeGraphEditor::Type::eOutput, 3, 0>
{
	static constexpr const char* Title{ "Output" };
	static constexpr std::array<const char*, NodeData::GetInputAttributeCount()> InputNodeName = { "r", "g", "b" };
};

template <typename T>
struct NodeGraphEditor::NodeTemplate : public NodeGraphEditor::INode
{
	T nodeValue{};

	NodeTemplate(int id)
		: INode(id)
	{
		if constexpr (T::GetInputAttributeCount() > 0)
		{
			nodeValue.inputNodeIDs.fill(NodeGraphEditor::eInvalidNodeID);
		}

		if constexpr (T::GetOutputAttributeCount() > 0)
		{
			nodeValue.outputNodeIDs.fill(NodeGraphEditor::eInvalidNodeID);
		}
	}

	virtual void Update(Graph& graph) override
	{
		//const float node_width = 100.0f;
		//imnodes::BeginNode(id);
		//
		//if constexpr (T::GetInputAttributeCount() > 0)
		//{
		//	for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
		//	{
		//		InputAttribute& inputAttribute = graph.GetInputAttribute(nodeValue.inputNodeIDs[i]);
		//
		//		imnodes::BeginInputAttribute(inputAttribute.id);
		//
		//		const float label_width = ImGui::CalcTextSize(T::InputNodeName[i]).x;
		//		ImGui::Text(T::InputNodeName[i]);
		//
		//		ImGui::SameLine();
		//		ImGui::PushItemWidth(node_width - label_width);
		//
		//		if (inputAttribute.linkedOutputNodeID == eInvalidNodeID)
		//		{
		//			ImGui::DragFloat("##hidelabel", &inputAttribute.value, 0.01f);
		//		}
		//		else
		//		{
		//			ImGui::Text("%.3f", inputAttribute.value);
		//		}
		//
		//		ImGui::PopItemWidth();
		//
		//		imnodes::EndAttribute();
		//	}
		//
		//	ImGui::Spacing();
		//}
		//
		//if constexpr (T::GetOutputAttributeCount() > 0)
		//{
		//	for (size_t i = 0; i < T::GetOutputAttributeCount(); ++i)
		//	{
		//		OutputAttribute& outputAttribute = graph.GetOutputAttribute(nodeValue.outputNodeIDs[i]);
		//
		//		imnodes::BeginOutputAttribute(nodeValue.outputNodeIDs[i]);
		//		if (outputAttribute.type == AttributeType::eNumber)
		//		{
		//			ImGui::PushItemWidth(node_width);
		//			ImGui::DragFloat("##hidelabel", &outputAttribute.value, 0.01f);
		//			ImGui::PopItemWidth();
		//		}
		//		else
		//		{
		//			const float label_width = ImGui::CalcTextSize(T::OutputNodeName[i]).x;
		//			ImGui::Indent(node_width - label_width);
		//			ImGui::Text(T::OutputNodeName[i]);
		//		}
		//		imnodes::EndAttribute();
		//	}
		//}
		//
		//imnodes::EndNode();
	}

	virtual void UpdateLink(Graph& graph) override
	{
		//if constexpr (T::GetInputAttributeCount() > 0)
		//{
		//	for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
		//	{
		//		InputAttribute& inputAttribute = graph.GetInputAttribute(nodeValue.inputNodeIDs[i]);
		//
		//		if (inputAttribute.linkedOutputNodeID != eInvalidNodeID)
		//		{
		//			imnodes::Link(inputAttribute.id, inputAttribute.linkedOutputNodeID, inputAttribute.id);
		//		}
		//	}
		//}
	}

	virtual void Process(Graph& graph, std::stack<float>& values) override
	{
		if constexpr (T::GetInputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
			{
				InputAttribute& inputAttribute = graph.GetInputAttribute(nodeValue.inputNodeIDs[i]);

				if (inputAttribute.linkedOutputNodeID != NodeGraphEditor::eInvalidNodeID)
				{
					if (graph.IsValidOutputAttribute(inputAttribute.linkedOutputNodeID) == true)
					{
						OutputAttribute& linkedOutputAttribute = graph.GetOutputAttribute(inputAttribute.linkedOutputNodeID);
						INode* pNode = graph.GetNode(linkedOutputAttribute.parentID);
						pNode->Process(graph, values);

						linkedOutputAttribute.Process(values);
						inputAttribute.value = values.top();
					}
					else
					{
						inputAttribute.linkedOutputNodeID = eInvalidNodeID;
						values.emplace(inputAttribute.value);
					}
				}
				else
				{
					values.emplace(inputAttribute.value);
				}
			}
		}
	}

	virtual void Destroy(Graph& graph) override
	{
		if constexpr (T::GetInputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
			{
				graph.RemoveInputAttribute(nodeValue.inputNodeIDs[i]);
			}
		}

		if constexpr (T::GetOutputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetOutputAttributeCount(); ++i)
			{
				graph.RemoveOutputAttribute(nodeValue.outputNodeIDs[i]);
			}
		}
	}

	virtual Type GetType() const override { return T::GetType(); }
};

NodeGraphEditor::InputAttribute::InputAttribute(int parentID, int id)
	: parentID(parentID)
	, id(id)
{
}

NodeGraphEditor::OutputAttribute::OutputAttribute(int parentID, int id, NodeGraphEditor::AttributeType type, std::function<void(NodeGraphEditor::OutputAttribute&, std::stack<float>&)> operationFunc)
	: parentID(parentID)
	, id(id)
	, type(type)
	, operationFunc(operationFunc)
{
}

NodeGraphEditor::INode::INode(int id)
	: id(id)
{
}

NodeGraphEditor::INode::~INode()
{
}

NodeGraphEditor::NodeGraphEditor()
{
}

NodeGraphEditor::~NodeGraphEditor()
{
}

void NodeGraphEditor::Update()
{
	//ImGui::Begin("Color node editor");
	//
	//imnodes::BeginNodeEditor();
	//
	//for (auto& iter : m_graph.nodes)
	//{
	//	iter.second->Update(m_graph);
	//}
	//
	//for (auto& iter : m_graph.nodes)
	//{
	//	iter.second->UpdateLink(m_graph);
	//}
	//
	//imnodes::EndNodeEditor();
	//
	//const bool isOpenPopup = ImGui::IsMouseClicked(1) || est::input::keyboard::IsKeyDown(est::input::keyboard::eA);
	//Popup(isOpenPopup);
	//
	//{
	//	const int num_selected = imnodes::NumSelectedLinks();
	//	if (num_selected > 0 && est::input::keyboard::IsKeyDown(est::input::keyboard::eDelete))
	//	{
	//		static std::vector<int> selected_links;
	//		selected_links.resize(static_cast<size_t>(num_selected), -1);
	//		imnodes::GetSelectedLinks(selected_links.data());
	//		for (const int link_id : selected_links)
	//		{
	//			InputAttribute& inputAttribute = m_graph.GetInputAttribute(link_id);
	//			inputAttribute.linkedOutputNodeID = eInvalidNodeID;
	//		}
	//		selected_links.clear();
	//	}
	//}
	//
	//{
	//	const int num_selected = imnodes::NumSelectedNodes();
	//	if (num_selected > 0 && est::input::keyboard::IsKeyDown(est::input::keyboard::eDelete))
	//	{
	//		static std::vector<int> selected_nodes;
	//		selected_nodes.resize(static_cast<size_t>(num_selected), -1);
	//		imnodes::GetSelectedNodes(selected_nodes.data());
	//		for (const int node_id : selected_nodes)
	//		{
	//			INode* pNode = m_graph.GetNode(node_id);
	//			pNode->Destroy(m_graph);
	//
	//			m_graph.RemoveNode(node_id);
	//		}
	//		selected_nodes.clear();
	//	}
	//}
	//
	//int link_start{ eInvalidNodeID };
	//int link_end{ eInvalidNodeID };
	//if (imnodes::IsLinkCreated(&link_start, &link_end))
	//{
	//	OutputAttribute& outputAttribute = m_graph.GetOutputAttribute(link_start);
	//	InputAttribute& inputAttribute = m_graph.GetInputAttribute(link_end);
	//
	//	inputAttribute.linkedOutputNodeID = outputAttribute.id;
	//}
	//
	//ImGui::End();
	//
	//ImU32 color = IM_COL32(255, 20, 147, 255);
	//if (m_outputNodeID != eInvalidNodeID)
	//{
	//	NodeTemplate<OutputNode>* pOutputNode = static_cast<NodeTemplate<OutputNode>*>(m_graph.GetNode(m_outputNodeID));
	//
	//	std::stack<float> values;
	//	pOutputNode->Process(m_graph, values);
	//
	//	const int r = static_cast<int>(255.f * std::clamp(values.top(), 0.f, 1.f));
	//	values.pop();
	//	const int g = static_cast<int>(255.f * std::clamp(values.top(), 0.f, 1.f));
	//	values.pop();
	//	const int b = static_cast<int>(255.f * std::clamp(values.top(), 0.f, 1.f));
	//	values.pop();
	//
	//	color = IM_COL32(r, g, b, 255);
	//}
	//
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
	//ImGui::Begin("output color");
	//ImGui::End();
	//ImGui::PopStyleColor();
}

template <typename T>
int CreateNodeMemnuItem(NodeGraphEditor::Graph& graph, const ImVec2& clickPosition, std::function<bool()> funcCheck = nullptr)
{
	if (ImGui::MenuItem(T::Title) == true && (funcCheck == nullptr || funcCheck() == true))
	{
		const int id = graph.CreateNode<T>();
		//NodeGraphEditor::NodeTemplate<T>* pNodeTemplate = static_cast<NodeGraphEditor::NodeTemplate<T>*>(graph.GetNode(id));
		//
		//if constexpr (T::GetInputAttributeCount() > 0)
		//{
		//	for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
		//	{
		//		const int attributeID = graph.GenerateID();
		//		const NodeGraphEditor::InputAttribute inputAttribute = pNodeTemplate->nodeValue.CreateInputAttribute(i, id, attributeID);
		//		graph.AddInputAttribute(inputAttribute);
		//	}
		//}
		//
		//if constexpr (T::GetOutputAttributeCount() > 0)
		//{
		//	for (size_t i = 0; i < T::GetOutputAttributeCount(); ++i)
		//	{
		//		const int attributeID = graph.GenerateID();
		//		const NodeGraphEditor::OutputAttribute outputAttribute = pNodeTemplate->nodeValue.CreateOutputAttribute(i, id, attributeID);
		//		graph.AddOutputAttribute(outputAttribute);
		//	}
		//}
		//
		//imnodes::SetNodePos(id, clickPosition);
		//imnodes::SetNodeName(id, T::Title);

		return id;
	}
	else
	{
		return NodeGraphEditor::eInvalidNodeID;
	}
}

void NodeGraphEditor::Popup(bool isOpenPopup)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));

	if (ImGui::IsAnyItemHovered() == false && isOpenPopup == true)
	{
		ImGui::OpenPopup("add Node");
	}

	if (ImGui::BeginPopup("add Node"))
	{
		const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

		const int outputNodeID = CreateNodeMemnuItem<OutputNode>(m_graph, click_pos, [&]() { return m_outputNodeID == eInvalidNodeID; });
		if (outputNodeID != eInvalidNodeID)
		{
			m_outputNodeID = outputNodeID;
		}
		
		// 변수
		if (ImGui::BeginMenu("변수"))
		{
			CreateNodeMemnuItem<NumberNode>(m_graph, click_pos);
			ImGui::EndMenu();
		}

		// 시간 함수
		if (ImGui::BeginMenu("시간"))
		{
			CreateNodeMemnuItem<TimeNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ElapsedTimeNode>(m_graph, click_pos);
			ImGui::EndMenu();
		}

		// 수학 함수
		if (ImGui::BeginMenu("산수"))
		{
			CreateNodeMemnuItem<AddNode>(m_graph, click_pos);
			CreateNodeMemnuItem<MinusNode>(m_graph, click_pos);
			CreateNodeMemnuItem<MultiplyNode>(m_graph, click_pos);
			CreateNodeMemnuItem<DivideNode>(m_graph, click_pos);
			CreateNodeMemnuItem<SqrtNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ReciprocalSqrtNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ExpNode>(m_graph, click_pos);
			CreateNodeMemnuItem<Exp2Node>(m_graph, click_pos);
			CreateNodeMemnuItem<PowNode>(m_graph, click_pos);
			CreateNodeMemnuItem<LdExpNode>(m_graph, click_pos);
			CreateNodeMemnuItem<LogNode>(m_graph, click_pos);
			CreateNodeMemnuItem<Log10Node>(m_graph, click_pos);
			CreateNodeMemnuItem<Log2Node>(m_graph, click_pos);
			ImGui::EndMenu();
		}

		// 값 변환 함수
		if (ImGui::BeginMenu("변환"))
		{
			CreateNodeMemnuItem<AbsNode>(m_graph, click_pos);
			CreateNodeMemnuItem<SignNode>(m_graph, click_pos);
			CreateNodeMemnuItem<CeilNode>(m_graph, click_pos);
			CreateNodeMemnuItem<FloorNode>(m_graph, click_pos);
			CreateNodeMemnuItem<RoundNode>(m_graph, click_pos);
			CreateNodeMemnuItem<MinNode>(m_graph, click_pos);
			CreateNodeMemnuItem<MaxNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ClampNode>(m_graph, click_pos);
			CreateNodeMemnuItem<SaturateNode>(m_graph, click_pos);
			CreateNodeMemnuItem<LerpNode>(m_graph, click_pos);
			CreateNodeMemnuItem<StepNode>(m_graph, click_pos);
			CreateNodeMemnuItem<SmoothstepNode>(m_graph, click_pos);
			CreateNodeMemnuItem<FModNode>(m_graph, click_pos);
			CreateNodeMemnuItem<FracNode>(m_graph, click_pos);
			CreateNodeMemnuItem<FrExpNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ModfNode>(m_graph, click_pos);
			ImGui::EndMenu();
		}

		// 삼각 함수
		if (ImGui::BeginMenu("삼각 함수"))
		{
			CreateNodeMemnuItem<SinNode>(m_graph, click_pos);
			CreateNodeMemnuItem<CosNode>(m_graph, click_pos);
			CreateNodeMemnuItem<TanNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ASinNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ACosNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ATanNode>(m_graph, click_pos);
			CreateNodeMemnuItem<ATan2Node>(m_graph, click_pos);
			CreateNodeMemnuItem<SinHNode>(m_graph, click_pos);
			CreateNodeMemnuItem<CosHNode>(m_graph, click_pos);
			CreateNodeMemnuItem<TanHNode>(m_graph, click_pos);
			CreateNodeMemnuItem<DegreesNode>(m_graph, click_pos);
			CreateNodeMemnuItem<RadiansNode>(m_graph, click_pos);
			ImGui::EndMenu();
		}

		// 벡터 함수
		if (ImGui::BeginMenu("벡터"))
		{
			//CreateNodeMemnuItem<CrossNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<DotNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<DistanceNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<LengthNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<NormalizeNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<DeterminantNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<TransposeNode>(m_graph, click_pos);
			//CreateNodeMemnuItem<MulNode>(m_graph, click_pos);
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

static NodeGraphEditor editor;
void NodeEditorShow() { editor.Update(); }