#pragma once

class NodeGraphEditor
{
public:
	NodeGraphEditor();
	virtual ~NodeGraphEditor();

public:
	void Update();

public:
	enum class Type
	{
		// 변수
		eFloat = 0,
		eFloat2,
		eFloat3,
		eFloat4,

		// 시간 함수
		eTime,
		eElapsedTime,

		// 수학 함수
		eAdd,
		eMinus,
		eMultiply,
		eDivide,
		eSqrt,
		eReciprocalSqrt,
		eExp,
		eExp2,
		ePow,
		eLdExp,
		eLog,
		eLog10,
		eLog2,

		// 값 변환 함수
		eAbs,
		eSign,
		eCeil,
		eFloor,
		eRound,
		eMin,
		eMax,
		eClamp,
		eSaturate,
		eLerp,
		eStep,
		eSmoothstep,
		eFMod,
		eFrac,
		eFrExp,
		eModf,

		// 삼각 함수
		eSin,
		eCos,
		eTan,
		eASin,
		eACos,
		eATan,
		eATan2,
		eSinH,
		eCosH,
		eTanH,
		eDegrees,
		eRadians,

		// 벡터 함수
		eCross,
		eDot,
		eDistance,
		eLength,
		eNormalize,
		eDeterminant,
		eTranspose,
		eMul,

		eOutput,
	};

	enum class AttributeType
	{
		eNumber = 0,
		eOperation,
	};

	enum : int
	{
		eInvalidNodeID = -1,
	};

	struct InputAttribute
	{
		const int parentID{ eInvalidNodeID };
		const int id{ eInvalidNodeID };
		const AttributeType type{ AttributeType::eNumber };

		int linkedOutputNodeID{ eInvalidNodeID };
		float value{ 0.f };

		InputAttribute(int parentID, int id);
	};

	struct OutputAttribute
	{
		const int parentID{ eInvalidNodeID };
		const int id{ eInvalidNodeID };
		const AttributeType type{ AttributeType::eNumber };

		float value{ 0.f };
		std::function<void(OutputAttribute&, std::stack<float>&)> operationFunc;

		OutputAttribute(int parentID, int id, AttributeType type, std::function<void(OutputAttribute&, std::stack<float>&)> operationFunc);

		void Process(std::stack<float>& stack)
		{
			operationFunc(*this, stack);
		}
	};

	struct Graph;

	struct INode
	{
		const int id{ eInvalidNodeID };

		INode(int id);
		virtual ~INode() = 0;

		virtual void Update(Graph& graph) = 0;
		virtual void UpdateLink(Graph& graph) = 0;
		virtual void Process(Graph& graph, std::stack<float>& values) = 0;

		virtual void Destroy(Graph& graph) = 0;
		virtual Type GetType() const = 0;
	};

	template <typename T>
	struct NodeTemplate;

	struct Graph
	{
		int m_idGenerator{ 0 };

		std::unordered_map<int, std::unique_ptr<INode>> nodes;
		std::unordered_map<int, InputAttribute> inputAttributes;
		std::unordered_map<int, OutputAttribute> outputAttributes;

		int GenerateID() { return m_idGenerator++; }

		template <typename T>
		int CreateNode()
		{
			const int id = GenerateID();
			nodes.emplace(id, std::make_unique<NodeTemplate<T>>(id));
			return id;
		}

		void RemoveNode(int id)
		{
			auto iter = nodes.find(id);
			if (iter != nodes.end())
			{
				nodes.erase(id);
			}
		}

		void AddInputAttribute(const InputAttribute& inputAttribute)
		{
			inputAttributes.emplace(inputAttribute.id, inputAttribute);
		}

		void RemoveInputAttribute(int id)
		{
			inputAttributes.erase(id);
		}

		void AddOutputAttribute(const OutputAttribute& outputAttribute)
		{
			outputAttributes.emplace(outputAttribute.id, outputAttribute);
		}

		void RemoveOutputAttribute(int id)
		{
			outputAttributes.erase(id);
		}

		bool IsValidOutputAttribute(int id)
		{
			return outputAttributes.find(id) != outputAttributes.end();
		}

		INode* GetNode(int id) { return nodes.at(id).get(); }
		InputAttribute& GetInputAttribute(int id) { return inputAttributes.at(id); }
		OutputAttribute& GetOutputAttribute(int id) { return outputAttributes.at(id); }
	};

protected:
	void Popup(bool isOpenPopup);

protected:
	Graph m_graph;
	int m_outputNodeID{ eInvalidNodeID };
};

class MaterialEditor : public NodeGraphEditor
{
public:
	MaterialEditor();
	virtual ~MaterialEditor();


};

void NodeEditorShow();