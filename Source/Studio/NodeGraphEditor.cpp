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
		const float node_width = 100.0f;
		imnodes::BeginNode(id);

		if constexpr (T::GetInputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
			{
				InputAttribute& inputAttribute = graph.GetInputAttribute(nodeValue.inputNodeIDs[i]);

				imnodes::BeginInputAttribute(inputAttribute.id);

				const float label_width = ImGui::CalcTextSize(T::InputNodeName[i]).x;
				ImGui::Text(T::InputNodeName[i]);

				ImGui::SameLine();
				ImGui::PushItemWidth(node_width - label_width);

				if (inputAttribute.linkedOutputNodeID == eInvalidNodeID)
				{
					ImGui::DragFloat("##hidelabel", &inputAttribute.value, 0.01f);
				}
				else
				{
					ImGui::Text("%.3f", inputAttribute.value);
				}

				ImGui::PopItemWidth();

				imnodes::EndAttribute();
			}

			ImGui::Spacing();
		}

		if constexpr (T::GetOutputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetOutputAttributeCount(); ++i)
			{
				OutputAttribute& outputAttribute = graph.GetOutputAttribute(nodeValue.outputNodeIDs[i]);

				imnodes::BeginOutputAttribute(nodeValue.outputNodeIDs[i]);
				if (outputAttribute.type == AttributeType::eNumber)
				{
					ImGui::PushItemWidth(node_width);
					ImGui::DragFloat("##hidelabel", &outputAttribute.value, 0.01f);
					ImGui::PopItemWidth();
				}
				else
				{
					const float label_width = ImGui::CalcTextSize(T::OutputNodeName[i]).x;
					ImGui::Indent(node_width - label_width);
					ImGui::Text(T::OutputNodeName[i]);
				}
				imnodes::EndAttribute();
			}
		}

		imnodes::EndNode();
	}

	virtual void UpdateLink(Graph& graph) override
	{
		if constexpr (T::GetInputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
			{
				InputAttribute& inputAttribute = graph.GetInputAttribute(nodeValue.inputNodeIDs[i]);

				if (inputAttribute.linkedOutputNodeID != eInvalidNodeID)
				{
					imnodes::Link(inputAttribute.id, inputAttribute.linkedOutputNodeID, inputAttribute.id);
				}
			}
		}
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
	ImGui::Begin("Color node editor");

	imnodes::BeginNodeEditor();

	for (auto& iter : m_graph.nodes)
	{
		iter.second->Update(m_graph);
	}

	for (auto& iter : m_graph.nodes)
	{
		iter.second->UpdateLink(m_graph);
	}

	imnodes::EndNodeEditor();

	const bool isOpenPopup = ImGui::IsMouseClicked(1) || est::input::keyboard::IsKeyDown(est::input::keyboard::eA);
	Popup(isOpenPopup);

	{
		const int num_selected = imnodes::NumSelectedLinks();
		if (num_selected > 0 && est::input::keyboard::IsKeyDown(est::input::keyboard::eDelete))
		{
			static std::vector<int> selected_links;
			selected_links.resize(static_cast<size_t>(num_selected), -1);
			imnodes::GetSelectedLinks(selected_links.data());
			for (const int link_id : selected_links)
			{
				InputAttribute& inputAttribute = m_graph.GetInputAttribute(link_id);
				inputAttribute.linkedOutputNodeID = eInvalidNodeID;
			}
			selected_links.clear();
		}
	}

	{
		const int num_selected = imnodes::NumSelectedNodes();
		if (num_selected > 0 && est::input::keyboard::IsKeyDown(est::input::keyboard::eDelete))
		{
			static std::vector<int> selected_nodes;
			selected_nodes.resize(static_cast<size_t>(num_selected), -1);
			imnodes::GetSelectedNodes(selected_nodes.data());
			for (const int node_id : selected_nodes)
			{
				INode* pNode = m_graph.GetNode(node_id);
				pNode->Destroy(m_graph);

				m_graph.RemoveNode(node_id);
			}
			selected_nodes.clear();
		}
	}

	int link_start{ eInvalidNodeID };
	int link_end{ eInvalidNodeID };
	if (imnodes::IsLinkCreated(&link_start, &link_end))
	{
		OutputAttribute& outputAttribute = m_graph.GetOutputAttribute(link_start);
		InputAttribute& inputAttribute = m_graph.GetInputAttribute(link_end);

		inputAttribute.linkedOutputNodeID = outputAttribute.id;
	}

	ImGui::End();

	ImU32 color = IM_COL32(255, 20, 147, 255);
	if (m_outputNodeID != eInvalidNodeID)
	{
		NodeTemplate<OutputNode>* pOutputNode = static_cast<NodeTemplate<OutputNode>*>(m_graph.GetNode(m_outputNodeID));

		std::stack<float> values;
		pOutputNode->Process(m_graph, values);

		const int r = static_cast<int>(255.f * std::clamp(values.top(), 0.f, 1.f));
		values.pop();
		const int g = static_cast<int>(255.f * std::clamp(values.top(), 0.f, 1.f));
		values.pop();
		const int b = static_cast<int>(255.f * std::clamp(values.top(), 0.f, 1.f));
		values.pop();

		color = IM_COL32(r, g, b, 255);
	}

	ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
	ImGui::Begin("output color");
	ImGui::End();
	ImGui::PopStyleColor();
}

template <typename T>
int CreateNodeMemnuItem(NodeGraphEditor::Graph& graph, const ImVec2& clickPosition, std::function<bool()> funcCheck = nullptr)
{
	if (ImGui::MenuItem(T::Title) == true && (funcCheck == nullptr || funcCheck() == true))
	{
		const int id = graph.CreateNode<T>();
		NodeGraphEditor::NodeTemplate<T>* pNodeTemplate = static_cast<NodeGraphEditor::NodeTemplate<T>*>(graph.GetNode(id));

		if constexpr (T::GetInputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetInputAttributeCount(); ++i)
			{
				const int attributeID = graph.GenerateID();
				const NodeGraphEditor::InputAttribute inputAttribute = pNodeTemplate->nodeValue.CreateInputAttribute(i, id, attributeID);
				graph.AddInputAttribute(inputAttribute);
			}
		}

		if constexpr (T::GetOutputAttributeCount() > 0)
		{
			for (size_t i = 0; i < T::GetOutputAttributeCount(); ++i)
			{
				const int attributeID = graph.GenerateID();
				const NodeGraphEditor::OutputAttribute outputAttribute = pNodeTemplate->nodeValue.CreateOutputAttribute(i, id, attributeID);
				graph.AddOutputAttribute(outputAttribute);
			}
		}

		imnodes::SetNodePos(id, clickPosition);
		imnodes::SetNodeName(id, T::Title);

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

//// The type T must be POD
//template<class T, size_t N>
//class StaticVector
//{
//public:
//	using Iterator = T*;
//	using ConstIterator = const T*;
//
//	StaticVector() : storage_(), size_(0) {}
//	~StaticVector() { size_ = 0; }
//
//	// Element access
//
//	inline T* data() { return storage_; }
//	inline const T* data() const { return storage_; }
//
//	inline T& back()
//	{
//		return const_cast<T&>(static_cast<const StaticVector*>(this)->back());
//	}
//	inline const T& back() const
//	{
//		assert(size_ > 0u);
//		assert(size_ <= N);
//		return storage_[size_ - 1];
//	}
//
//	inline T& operator[](const size_t i)
//	{
//		return const_cast<T&>(
//			static_cast<const StaticVector*>(this)->operator[](i));
//	}
//	inline const T& operator[](const size_t i) const
//	{
//		assert(i < size_);
//		return storage_[i];
//	}
//
//	inline Iterator find(const T& t)
//	{
//		return const_cast<Iterator>(
//			static_cast<const StaticVector*>(this)->find(t));
//	}
//	inline ConstIterator find(const T& t) const
//	{
//		auto iter = begin();
//		while (iter != end())
//		{
//			if (*iter == t)
//			{
//				return iter;
//			}
//			++iter;
//		}
//		return iter;
//	}
//
//	// Capacity
//
//	inline bool empty() const { return size_ == 0u; }
//
//	inline size_t size() const { return size_; }
//
//	inline size_t capacity() const { return N; }
//
//	// Modifiers
//
//	inline void push_back(const T& elem)
//	{
//		assert(size_ < N);
//		storage_[size_] = elem;
//		++size_;
//	}
//
//	inline void pop_back()
//	{
//		assert(size_);
//		--size_;
//	}
//
//	inline void swap_erase(size_t remove_at) { swap_erase(data() + remove_at); }
//	inline void swap_erase(Iterator iter)
//	{
//		assert(size_ > 0u);
//		assert(size_t(iter - begin()) < size_);
//
//		if (iter != &back())
//		{
//			std::swap(*iter, back());
//		}
//
//		pop_back();
//	}
//
//	inline void clear() { size_ = 0u; }
//
//	// Iterators
//
//	inline Iterator begin() { return data(); }
//	inline ConstIterator begin() const { return data(); }
//
//	inline Iterator end() { return storage_ + size_; }
//	inline ConstIterator end() const { return storage_ + size_; }
//
//private:
//	T storage_[N];
//	size_t size_;
//};
//
//enum NodeType
//{
//	Node_Number,
//	Node_NumberExpression, // the number isn't stored in the node, but is
//						   // provided by another node
//						   Node_Operation,
//						   Node_Output
//};
//
//using StackType = std::stack<float>;
//using OperationFn = void (*)(StackType&);
//
//struct Node
//{
//	NodeType type;
//
//	union
//	{
//		float number;
//		OperationFn operation;
//	};
//};
//
//struct Edge
//{
//	// the from, to variables store the node ids of the nodes contained in the
//	// edge.
//	size_t from, to;
//
//	Edge(size_t f, size_t t) : from(f), to(t) {}
//
//	// seems like std::unordered_map requires this to be
//	// default-constructible...
//	Edge() : from(), to() {}
//
//	inline size_t opposite(size_t n) const { return n == from ? to : from; }
//};
//
//// a directional graph
//class Graph
//{
//public:
//	// the graph has a limited number of adjacencies, simplifies memory usage
//	using AdjacencyArray = StaticVector<size_t, 3u>;
//
//	using EdgeIterator = std::unordered_map<size_t, Edge>::iterator;
//	using ConstEdgeIterator = std::unordered_map<size_t, Edge>::const_iterator;
//
//	Graph()
//		: current_id_(0u), nodes_(), edges_from_node_(), edges_to_node_(),
//		edges_()
//	{
//	}
//
//	// Element access
//
//	inline Node& node(const size_t node_id)
//	{
//		return const_cast<Node&>(
//			static_cast<const Graph*>(this)->node(node_id));
//	}
//	inline const Node& node(const size_t node_id) const
//	{
//		assert(nodes_.find(node_id) != nodes_.end());
//		return nodes_.at(node_id);
//	}
//
//	inline const AdjacencyArray& edges_from_node(const size_t node_id)
//	{
//		return edges_from_node_[node_id];
//	}
//
//	inline const AdjacencyArray& edges_to_node(const size_t node_id)
//	{
//		return edges_to_node_[node_id];
//	}
//
//	inline Edge& edge(const size_t edge_id)
//	{
//		return const_cast<Edge&>(
//			static_cast<const Graph*>(this)->edge(edge_id));
//	}
//	inline const Edge& edge(const size_t edge_id) const
//	{
//		assert(edges_.find(edge_id) != edges_.end());
//		return edges_.at(edge_id);
//	}
//
//	inline EdgeIterator begin_edges() { return edges_.begin(); }
//	inline ConstEdgeIterator end_edges() const { return edges_.begin(); }
//
//	inline EdgeIterator end_edges() { return edges_.end(); }
//	inline ConstEdgeIterator end_edge() const { return edges_.end(); }
//
//	// Modifiers
//
//	size_t add_node(const Node& node)
//	{
//		const size_t id = current_id_++;
//		nodes_.insert(std::make_pair(id, node));
//		edges_from_node_.insert(std::make_pair(id, AdjacencyArray()));
//		edges_to_node_.insert(std::make_pair(id, AdjacencyArray()));
//		return id;
//	}
//
//	void erase_node(const size_t node_id)
//	{
//		// first, collect all the edges from the adjacency lists
//		// since erasing an edge invalidates the adjacency list iterators
//		StaticVector<size_t, 6> edges_to_erase;
//		for (size_t edge : edges_from_node_[node_id])
//		{
//			edges_to_erase.push_back(edge);
//		}
//		for (size_t edge : edges_to_node_[node_id])
//		{
//			edges_to_erase.push_back(edge);
//		}
//		for (size_t edge : edges_to_erase)
//		{
//			erase_edge(edge);
//		}
//		nodes_.erase(node_id);
//		edges_from_node_.erase(node_id);
//		edges_to_node_.erase(node_id);
//	}
//
//	size_t add_edge(const size_t from, const size_t to)
//	{
//		const size_t id = current_id_++;
//		edges_.insert(std::make_pair(id, Edge(from, to)));
//		edges_from_node_[from].push_back(id);
//		edges_to_node_[to].push_back(id);
//		return id;
//	}
//
//	void erase_edge(const size_t edge_id)
//	{
//		auto edge = edges_.find(edge_id);
//		assert(edge != edges_.end());
//
//		{
//			auto& edges_from = edges_from_node_[edge->second.from];
//			auto iter = edges_from.find(edge_id);
//			assert(iter != edges_from.end());
//			edges_from.swap_erase(iter);
//		}
//
//		{
//			auto& edges_to = edges_to_node_[edge->second.to];
//			auto iter = edges_to.find(edge_id);
//			assert(iter != edges_to.end());
//			edges_to.swap_erase(iter);
//		}
//
//		edges_.erase(edge);
//	}
//
//	ImU32 evaluate(const size_t root_node)
//	{
//		// this function does a depth-first evaluation of the graph
//		// the nodes are evaluated post-order using two stacks.
//		std::stack<float> eval_stack;
//		std::stack<size_t> preorder;
//		std::stack<size_t> postorder;
//
//		preorder.push(root_node);
//
//		while (!preorder.empty())
//		{
//			const size_t node = preorder.top();
//			preorder.pop();
//
//			postorder.push(node);
//
//			for (const size_t edge : edges_from_node_[node])
//			{
//				const size_t neighbor = edges_[edge].opposite(node);
//				assert(neighbor != root_node);
//				preorder.push(neighbor);
//			}
//		}
//
//		// unwind the stack and call each operation along the way
//		while (!postorder.empty())
//		{
//			const size_t node = postorder.top();
//			postorder.pop();
//			switch (nodes_[node].type)
//			{
//			case Node_Number:
//				eval_stack.push(nodes_[node].number);
//				break;
//			case Node_NumberExpression:
//				break;
//			case Node_Operation:
//				nodes_[node].operation(eval_stack);
//				break;
//			case Node_Output:
//				break;
//			default:
//				assert("Invalid enum value!");
//			}
//		}
//
//		// there should be three values on the stack
//		assert(eval_stack.size() == 3);
//		const int b = 255 * std::clamp(eval_stack.top(), 0.f, 1.f);
//		eval_stack.pop();
//		const int g = 255 * std::clamp(eval_stack.top(), 0.f, 1.f);
//		eval_stack.pop();
//		const int r = 255 * std::clamp(eval_stack.top(), 0.f, 1.f);
//		eval_stack.pop();
//
//		return IM_COL32(r, g, b, 255);
//	}
//
//private:
//	size_t current_id_;
//	std::unordered_map<size_t, Node> nodes_;
//	std::unordered_map<size_t, AdjacencyArray> edges_from_node_;
//	std::unordered_map<size_t, AdjacencyArray> edges_to_node_;
//	std::unordered_map<size_t, Edge> edges_;
//};
//
//struct TimeContext
//{
//	std::chrono::steady_clock::time_point start;
//	float seconds_elapsed;
//
//	TimeContext()
//		: start(std::chrono::steady_clock::now()), seconds_elapsed(0.f)
//	{
//	}
//
//	void update()
//	{
//		seconds_elapsed = std::chrono::duration_cast<
//			std::chrono::duration<float, std::ratio<1>>>(
//				std::chrono::steady_clock::now() - start)
//			.count();
//	}
//};
//
//TimeContext time_context;
//
//void operation_time(std::stack<float>& stack)
//{
//	stack.push(time_context.seconds_elapsed);
//}
//
//void operation_sine(std::stack<float>& stack)
//{
//	const float x = stack.top();
//	stack.pop();
//	stack.push(std::abs(std::sin(x)));
//}
//
//void operation_multiply(std::stack<float>& stack)
//{
//	const float rhs = stack.top();
//	stack.pop();
//	const float lhs = stack.top();
//	stack.pop();
//	stack.push(lhs * rhs);
//}
//
//void operation_add(std::stack<float>& stack)
//{
//	const float rhs = stack.top();
//	stack.pop();
//	const float lhs = stack.top();
//	stack.pop();
//	stack.push(lhs + rhs);
//}
//
//class ColorNodeEditor
//{
//public:
//	ColorNodeEditor() = default;
//	~ColorNodeEditor() = default;
//
//	void show(float elapsedTime)
//	{
//		time_context.update();
//
//		ImGui::Begin("Color node editor");
//		ImGui::Text("a -- add node");
//		ImGui::Text("X -- delete selected node or link");
//
//		imnodes::BeginNodeEditor();
//
//		for (const auto& node : output_nodes_)
//		{
//			const float node_width = 100.0f;
//			imnodes::PushColorStyle(
//				imnodes::ColorStyle_TitleBar, IM_COL32(11, 109, 191, 255));
//			imnodes::PushColorStyle(
//				imnodes::ColorStyle_TitleBarHovered,
//				IM_COL32(45, 126, 194, 255));
//			imnodes::PushColorStyle(
//				imnodes::ColorStyle_TitleBarSelected,
//				IM_COL32(81, 148, 204, 255));
//			imnodes::BeginNode(node.out);
//
//			ImGui::Dummy(ImVec2(node_width, 0.f));
//			{
//				// TODO: the color style of the pin needs to be pushed here
//				imnodes::BeginInputAttribute(int(node.red));
//				const float label_width = ImGui::CalcTextSize("r").x;
//				ImGui::Text("r");
//				if (graph_.node(node.red).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel",
//						&graph_.node(node.red).number,
//						0.01f,
//						0.f,
//						1.0f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			ImGui::Spacing();
//
//			{
//				imnodes::BeginInputAttribute(int(node.green));
//				const float label_width = ImGui::CalcTextSize("g").x;
//				ImGui::Text("g");
//				if (graph_.node(node.green).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel",
//						&graph_.node(node.green).number,
//						0.01f,
//						0.f,
//						1.f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			ImGui::Spacing();
//
//			{
//				imnodes::BeginInputAttribute(int(node.blue));
//				const float label_width = ImGui::CalcTextSize("b").x;
//				ImGui::Text("b");
//				if (graph_.node(node.blue).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel",
//						&graph_.node(node.blue).number,
//						0.01f,
//						0.f,
//						1.0f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//			imnodes::EndNode();
//			imnodes::PopColorStyle();
//			imnodes::PopColorStyle();
//			imnodes::PopColorStyle();
//		}
//
//		for (const auto& node : sine_nodes_)
//		{
//			const float node_width = 100.0f;
//			imnodes::BeginNode(node.op);
//
//			{
//				imnodes::BeginInputAttribute(int(node.input));
//				const float label_width = ImGui::CalcTextSize("number").x;
//				ImGui::Text("number");
//				if (graph_.node(node.input).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel",
//						&graph_.node(node.input).number,
//						0.01f,
//						0.f,
//						1.0f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			ImGui::Spacing();
//
//			{
//				imnodes::BeginOutputAttribute(int(node.op));
//				const float label_width = ImGui::CalcTextSize("output").x;
//				ImGui::Indent(node_width - label_width);
//				ImGui::Text("output");
//				imnodes::EndAttribute();
//			}
//
//			imnodes::EndNode();
//		}
//
//		for (const auto& node : time_nodes_)
//		{
//			imnodes::BeginNode(node);
//
//			imnodes::BeginOutputAttribute(int(node));
//			ImGui::Text("output");
//			imnodes::EndAttribute();
//
//			imnodes::EndNode();
//		}
//
//		for (const auto& node : mul_nodes_)
//		{
//			const float node_width = 100.0f;
//			imnodes::BeginNode(node.op);
//
//			{
//				imnodes::BeginInputAttribute(int(node.lhs));
//				const float label_width = ImGui::CalcTextSize("left").x;
//				ImGui::Text("left");
//				if (graph_.node(node.lhs).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel", &graph_.node(node.lhs).number, 0.01f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			{
//				imnodes::BeginInputAttribute(int(node.rhs));
//				const float label_width = ImGui::CalcTextSize("right").x;
//				ImGui::Text("right");
//				if (graph_.node(node.rhs).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel", &graph_.node(node.rhs).number, 0.01f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			ImGui::Spacing();
//
//			{
//				imnodes::BeginOutputAttribute(int(node.op));
//				const float label_width = ImGui::CalcTextSize("result").x;
//				ImGui::Indent(node_width - label_width);
//				ImGui::Text("result");
//				imnodes::EndAttribute();
//			}
//
//			imnodes::EndNode();
//		}
//
//		for (const auto& node : add_nodes_)
//		{
//			const float node_width = 100.0f;
//			imnodes::BeginNode(node.op);
//
//			{
//				imnodes::BeginInputAttribute(int(node.lhs));
//				const float label_width = ImGui::CalcTextSize("left").x;
//				ImGui::Text("left");
//				if (graph_.node(node.lhs).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel", &graph_.node(node.lhs).number, 0.01f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			{
//				imnodes::BeginInputAttribute(int(node.rhs));
//				const float label_width = ImGui::CalcTextSize("right").x;
//				ImGui::Text("right");
//				if (graph_.node(node.rhs).type == Node_Number)
//				{
//					ImGui::SameLine();
//					ImGui::PushItemWidth(node_width - label_width);
//					ImGui::DragFloat(
//						"##hidelabel", &graph_.node(node.rhs).number, 0.01f);
//					ImGui::PopItemWidth();
//				}
//				imnodes::EndAttribute();
//			}
//
//			ImGui::Spacing();
//
//			{
//				imnodes::BeginOutputAttribute(int(node.op));
//				const float label_width = ImGui::CalcTextSize("result").x;
//				ImGui::Indent(node_width - label_width);
//				ImGui::Text("result");
//				imnodes::EndAttribute();
//			}
//
//			imnodes::EndNode();
//		}
//
//		for (auto iter = graph_.begin_edges(); iter != graph_.end_edges();
//			++iter)
//		{
//			// don't render internal edges
//			// internal edges always look like
//			//
//			// Node_Output | Node_Operation
//			// ->
//			// Node_Number | Node_NumberExpression
//			const NodeType type = graph_.node(iter->second.to).type;
//			if (type == Node_Number || type == Node_NumberExpression)
//				continue;
//			imnodes::Link(iter->first, iter->second.from, iter->second.to);
//		}
//
//		imnodes::EndNodeEditor();
//
//		const bool open_popup =
//			ImGui::IsMouseClicked(1) || est::input::keyboard::IsKeyDown(est::input::keyboard::eA);
//
//		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
//		if (!ImGui::IsAnyItemHovered() && open_popup)
//		{
//			ImGui::OpenPopup("add node");
//		}
//
//		if (ImGui::BeginPopup("add node"))
//		{
//			ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
//
//			if (ImGui::MenuItem("output") && output_nodes_.size() == 0u)
//			{
//				// The output node takes three input attributes:
//				// r, g, b
//				// They are rendered in top-to-bottom order in the node UI
//				// By convention, the adjacencies are added in top-to-bottom
//				// order
//				OutputNode node;
//				Node num = Node{ Node_Number, 0.f };
//				Node out = Node{ Node_Output, 0.f };
//
//				node.red = graph_.add_node(num);
//				node.green = graph_.add_node(num);
//				node.blue = graph_.add_node(num);
//				node.out = graph_.add_node(out);
//
//				output_nodes_.push_back(node);
//
//				graph_.add_edge(node.out, node.red);
//				graph_.add_edge(node.out, node.green);
//				graph_.add_edge(node.out, node.blue);
//
//				imnodes::SetNodePos(node.out, click_pos);
//				imnodes::SetNodeName(node.out, "output");
//			}
//
//			if (ImGui::MenuItem("time"))
//			{
//				Node op = Node{ Node_Operation, 0.f };
//				op.operation = operation_time;
//
//				const size_t node = graph_.add_node(op);
//				time_nodes_.push_back(node);
//
//				imnodes::SetNodePos(node, click_pos);
//				imnodes::SetNodeName(node, "time");
//			}
//
//			if (ImGui::MenuItem("sine"))
//			{
//				SineNode node;
//
//				Node num{ Node_Number, 0.f };
//				Node op{ Node_Operation, 0.f };
//				op.operation = operation_sine;
//
//				node.input = graph_.add_node(num);
//				node.op = graph_.add_node(op);
//
//				graph_.add_edge(node.op, node.input);
//
//				sine_nodes_.push_back(node);
//
//				imnodes::SetNodePos(node.op, click_pos);
//				imnodes::SetNodeName(node.op, "sine");
//			}
//
//			if (ImGui::MenuItem("multiply"))
//			{
//				MultiplyNode node;
//
//				Node num{ Node_Number, 0.f };
//				Node op{ Node_Operation, 0.f };
//				op.operation = operation_multiply;
//
//				node.lhs = graph_.add_node(num);
//				node.rhs = graph_.add_node(num);
//				node.op = graph_.add_node(op);
//
//				graph_.add_edge(node.op, node.lhs);
//				graph_.add_edge(node.op, node.rhs);
//
//				mul_nodes_.push_back(node);
//
//				imnodes::SetNodePos(node.op, click_pos);
//				imnodes::SetNodeName(node.op, "multiply");
//			}
//
//			if (ImGui::MenuItem("add"))
//			{
//				AddNode node;
//
//				Node num{ Node_Number, 0.f };
//				Node op{ Node_Operation, 0.f };
//				op.operation = operation_add;
//
//				node.lhs = graph_.add_node(num);
//				node.rhs = graph_.add_node(num);
//				node.op = graph_.add_node(op);
//
//				graph_.add_edge(node.op, node.lhs);
//				graph_.add_edge(node.op, node.rhs);
//
//				add_nodes_.push_back(node);
//
//				imnodes::SetNodePos(node.op, click_pos);
//				imnodes::SetNodeName(node.op, "add");
//			}
//			ImGui::EndPopup();
//		}
//
//		ImGui::PopStyleVar();
//
//		{
//			const int num_selected = imnodes::NumSelectedLinks();
//			if (num_selected > 0 && est::input::keyboard::IsKeyDown(est::input::keyboard::eX))
//			{
//				static std::vector<int> selected_links;
//				selected_links.resize(static_cast<size_t>(num_selected), -1);
//				imnodes::GetSelectedLinks(selected_links.data());
//				for (const int link_id : selected_links)
//				{
//					assert(link_id >= 0);
//					Node& node = graph_.node(graph_.edge(link_id).from);
//					assert(node.type == Node_NumberExpression);
//					node.type = Node_Number;
//					graph_.erase_edge(size_t(link_id));
//				}
//				selected_links.clear();
//			}
//		}
//
//		{
//			const int num_selected = imnodes::NumSelectedNodes();
//			if (num_selected > 0 && est::input::keyboard::IsKeyDown(est::input::keyboard::eX))
//			{
//				static std::vector<int> selected_nodes;
//				selected_nodes.resize(static_cast<size_t>(num_selected), -1);
//				imnodes::GetSelectedNodes(selected_nodes.data());
//				for (const int node_id : selected_nodes)
//				{
//					find_and_remove_node(node_id);
//				}
//				selected_nodes.clear();
//			}
//		}
//
//		Id link_start, link_end;
//		if (imnodes::IsLinkCreated(&link_start.id, &link_end.id))
//		{
//			// in the expression graph, we want the edge to always go from
//			// the number to the operation, since the graph is directed!
//			const size_t from_id = graph_.node(link_start).type == Node_Number
//				? link_start
//				: link_end;
//			const size_t to_id = graph_.node(link_end).type == Node_Operation
//				? link_end
//				: link_start;
//
//			bool invalid_node = false;
//			for (size_t edge : graph_.edges_to_node(from_id))
//			{
//				if (graph_.edge(edge).from == to_id)
//				{
//					invalid_node = true;
//					break;
//				}
//			}
//
//			invalid_node = (graph_.node(from_id).type != Node_Number ||
//				graph_.node(to_id).type != Node_Operation) ||
//				invalid_node;
//
//			if (!invalid_node)
//			{
//				graph_.add_edge(from_id, to_id);
//				Node& node_from = graph_.node(from_id);
//				Node& node_to = graph_.node(to_id);
//				node_from.type = node_from.type == Node_Number
//					? Node_NumberExpression
//					: node_from.type;
//			}
//		}
//
//		ImGui::End();
//
//		ImU32 color = IM_COL32(255, 20, 147, 255);
//		if (output_nodes_.size() > 0u)
//		{
//			color = graph_.evaluate(output_nodes_[0u].out);
//		}
//
//		ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
//		ImGui::Begin("output color");
//		ImGui::End();
//		ImGui::PopStyleColor();
//	}
//
//private:
//	struct OutputNode
//	{
//		size_t red, green, blue, out;
//	};
//
//	struct SineNode
//	{
//		size_t input, op;
//	};
//
//	struct MultiplyNode
//	{
//		size_t lhs, rhs, op;
//	};
//
//	struct AddNode
//	{
//		size_t lhs, rhs, op;
//	};
//
//	struct Id
//	{
//		int id;
//
//		inline bool is_valid() const { return id >= 0; }
//
//		inline void invalidate() { id = invalid_index; }
//
//		inline operator int() const
//		{
//			assert(is_valid());
//			return id;
//		}
//
//		inline Id& operator=(int i)
//		{
//			id = i;
//			return *this;
//		}
//
//		inline bool operator==(int i) const { return id == i; }
//
//		Id() : id(invalid_index) {}
//
//	private:
//		static const int invalid_index = -1;
//	};
//
//	inline void find_and_remove_node(const int id)
//	{
//		// this function is a spectacular feat of engineering...
//
//		assert(id >= 0);
//
//		if (remove_output_node(id))
//			return;
//
//		if (remove_time_node(id))
//			return;
//
//		if (remove_sine_node(id))
//			return;
//
//		if (remove_mul_node(id))
//			return;
//
//		remove_add_node(id);
//	}
//
//	inline bool remove_output_node(const int id)
//	{
//		if (output_nodes_.size() > 0u && output_nodes_[0].out == id)
//		{
//			const auto& node = output_nodes_[0];
//			graph_.erase_node(node.out);
//			graph_.erase_node(node.red);
//			graph_.erase_node(node.green);
//			graph_.erase_node(node.blue);
//			output_nodes_.pop_back();
//			return true;
//		}
//		return false;
//	}
//
//	inline bool remove_time_node(const int id)
//	{
//		auto iter =
//			std::find(time_nodes_.begin(), time_nodes_.end(), size_t(id));
//		if (iter != time_nodes_.end())
//		{
//			graph_.erase_node(*iter);
//			time_nodes_.erase(iter);
//			return true;
//		}
//		return false;
//	}
//
//	inline bool remove_sine_node(const int id)
//	{
//		for (auto iter = sine_nodes_.begin(); iter != sine_nodes_.end(); ++iter)
//		{
//			if (iter->op == id)
//			{
//				graph_.erase_node(iter->op);
//				graph_.erase_node(iter->input);
//				sine_nodes_.erase(iter);
//				return true;
//			}
//		}
//		return false;
//	}
//
//	inline bool remove_mul_node(const int id)
//	{
//		for (auto iter = mul_nodes_.begin(); iter != mul_nodes_.end(); ++iter)
//		{
//			if (iter->op == id)
//			{
//				graph_.erase_node(iter->op);
//				graph_.erase_node(iter->lhs);
//				graph_.erase_node(iter->rhs);
//				mul_nodes_.erase(iter);
//				return true;
//			}
//		}
//		return false;
//	}
//
//	inline bool remove_add_node(const int id)
//	{
//		for (auto iter = add_nodes_.begin(); iter != add_nodes_.end(); ++iter)
//		{
//			if (iter->op == id)
//			{
//				graph_.erase_node(iter->op);
//				graph_.erase_node(iter->lhs);
//				graph_.erase_node(iter->rhs);
//				add_nodes_.erase(iter);
//				return true;
//			}
//		}
//		return false;
//	}
//
//	Graph graph_;
//	StaticVector<OutputNode, 1> output_nodes_;
//	std::vector<size_t>
//		time_nodes_; // just a single node representing the operation
//	std::vector<SineNode> sine_nodes_;
//	std::vector<MultiplyNode> mul_nodes_;
//	std::vector<AddNode> add_nodes_;
//}; // namespace
//
//static ColorNodeEditor color_editor;

//void NodeEditorShow() { color_editor.show(0.f); }