#pragma once

struct ImDrawList;
class MaterialNodeManager;

class MaterialNode
{
public:
	struct InOutSlot
	{
		enum Type
		{
			eNone = 0,
			eIn,
			eOut,
		};

		enum ValueType
		{
			eFloat = 0,
			eFloat2,
			eFloat3,
			eFloat4,
		};

		MaterialNodeManager* pManager = nullptr;

		Type emType = eNone;
		ValueType emValueType = ValueType::eFloat;

		bool isActive = false;
		bool isHovered = false;

		int nNodeID = -1;
		int nSlotIdx = -1;
		std::string strLabel;

		est::math::float2 f2Pos;
		est::math::float2 f2Size;

		est::math::float2 f2LinkerPos;

		est::math::float2 f2ButtonSize;

		InOutSlot() {}
		InOutSlot(MaterialNodeManager* pManager, Type emType, int nNodeID, int nSlotIdx, const char* strLabel, const est::math::float2& f2ButtonSize, ValueType emValueType = ValueType::eFloat)
			: pManager(pManager)
			, nNodeID(nNodeID)
			, emType(emType)
			, nSlotIdx(nSlotIdx)
			, strLabel(strLabel)
			, f2ButtonSize(f2ButtonSize)
			, emValueType(emValueType)
		{
		}

		Type GetType() const { return emType; }
		ValueType GetValueType() const { return emValueType; }
		void SetValueType(ValueType _emValueType) { emValueType = _emValueType; }
		bool IsHovered() const { return isHovered; }
		bool IsActive() const { return isActive; }

		const est::math::float2& GetLinkerPos() const { return f2LinkerPos; }
		const est::math::float2& GetPos() const { return f2Pos; }
		const est::math::float2& GetSize() const { return f2Size; }

		void Render(ImDrawList* pDrawList, const est::math::float2& f2Offset);
		void RenderBackground(ImDrawList* pDrawList, const est::math::float2& f2Offset);
	};

	struct Link
	{
		int nOutputNodeID = -1;
		int nOutputNodeSlotIndex = -1;

		int nInputNodeID = -1;
		int nInputNodeSlotIndex = -1;

		Link() {}
		Link(int nOutputNodeID, int nOutputNodeSlotIndex, int nInputNodeID, int nInputNodeSlotIndex)
			: nOutputNodeID(nOutputNodeID)
			, nOutputNodeSlotIndex(nOutputNodeSlotIndex)
			, nInputNodeID(nInputNodeID)
			, nInputNodeSlotIndex(nInputNodeSlotIndex)
		{
		}
	};

public:
	MaterialNode(MaterialNodeManager* pManager, int nID, const char* name, const est::math::float2& pos)
		: m_pManager(pManager)
		, m_nID(nID)
		, m_strName(name)
		, m_f2Pos(pos)
	{
	}
	virtual ~MaterialNode() = 0 {}

	int GetID() const { return m_nID; }
	const char* GetName() const { return m_strName.c_str(); }

	size_t GetInputSlotCount() const { return m_vecInputSlot.size(); }
	size_t GetOutputSlotCount() const { return m_vecOutputSlot.size(); }

	const InOutSlot* GetInputSlot(int nSlotIndex) const
	{
		if (nSlotIndex < 0 || nSlotIndex >= static_cast<int>(m_vecInputSlot.size()))
			return nullptr;

		return &m_vecInputSlot[nSlotIndex];
	}

	const InOutSlot* GetOutputSlot(int nSlotIndex) const
	{
		if (nSlotIndex < 0 || nSlotIndex >= static_cast<int>(m_vecOutputSlot.size()))
			return nullptr;

		return &m_vecOutputSlot[nSlotIndex];
	}

	virtual void RenderNode(ImDrawList* pDrawList, const est::math::float2& f2Offset);
	virtual void RenderNodeBackground(ImDrawList* pDrawList, const est::math::float2& f2Offset);
	virtual void Update(ImDrawList* pDrawList, const est::math::float2& f2Offset);

	bool IsSlotHovered() const
	{
		for (auto& slot : m_vecInputSlot)
		{
			if (slot.IsHovered() == true)
				return true;
		}

		for (auto& slot : m_vecOutputSlot)
		{
			if (slot.IsHovered() == true)
				return true;
		}

		return false;
	}

	bool IsSlotActive() const
	{
		for (auto& slot : m_vecInputSlot)
		{
			if (slot.IsActive() == true)
				return true;
		}

		for (auto& slot : m_vecOutputSlot)
		{
			if (slot.IsActive() == true)
				return true;
		}

		return false;
	}

	void LinkToInput(int nOutputNodeID, int nOutputNodeSlotIndex, int nInputNodeID, int nInputNodeSlotIndex)
	{
		auto iter = std::find_if(m_listLinkToInput.begin(), m_listLinkToInput.end(), [&](const Link& source)
		{
			return source.nInputNodeID == nInputNodeID && source.nInputNodeSlotIndex == nInputNodeSlotIndex && source.nOutputNodeID == nOutputNodeID && source.nOutputNodeSlotIndex == nOutputNodeSlotIndex;
		});

		if (iter == m_listLinkToInput.end())
		{
			m_listLinkToInput.emplace_back(nOutputNodeID, nOutputNodeSlotIndex, nInputNodeID, nInputNodeSlotIndex);
		}
	}
	void LinkToOutput(int nOutputNodeID, int nOutputNodeSlotIndex, int nInputNodeID, int nInputNodeSlotIndex)
	{
		auto iter = std::find_if(m_listLinkToOutput.begin(), m_listLinkToOutput.end(), [&](const Link& source)
		{
			return source.nInputNodeID == nInputNodeID && source.nInputNodeSlotIndex == nInputNodeSlotIndex && source.nOutputNodeID == nOutputNodeID && source.nOutputNodeSlotIndex == nOutputNodeSlotIndex;
		});

		if (iter == m_listLinkToOutput.end())
		{
			m_listLinkToOutput.emplace_back(nOutputNodeID, nOutputNodeSlotIndex, nInputNodeID, nInputNodeSlotIndex);
		}
	}

	std::list<Link>& GetLinkToInput() { return m_listLinkToInput; }
	std::list<Link>& GetLinkToOutput() { return m_listLinkToOutput; }

protected:
	void CreateInputSlot(const char* strLabel, MaterialNode::InOutSlot::ValueType emValueType = MaterialNode::InOutSlot::ValueType::eFloat, const est::math::float2& f2ButtonSize = est::math::float2(125.f, 20.f)) { m_vecInputSlot.emplace_back(m_pManager, InOutSlot::eIn, m_nID, m_vecInputSlot.size(), strLabel, f2ButtonSize, emValueType); }
	void CreateOutputSlot(const char* strLabel, MaterialNode::InOutSlot::ValueType emValueType = MaterialNode::InOutSlot::ValueType::eFloat, const est::math::float2& f2ButtonSize = est::math::float2(125.f, 20.f)) { m_vecOutputSlot.emplace_back(m_pManager, InOutSlot::eOut, m_nID, m_vecOutputSlot.size(), strLabel, f2ButtonSize, emValueType); }

protected:
	MaterialNodeManager* m_pManager;

	int m_nID;
	std::string m_strName;
	est::math::float2 m_f2Pos;
	est::math::float2 m_f2Size;

	std::vector<InOutSlot> m_vecInputSlot;
	std::vector<InOutSlot> m_vecOutputSlot;

	std::list<Link> m_listLinkToInput;
	std::list<Link> m_listLinkToOutput;
};

class MaterialNodeManager
{
private:
	class OutputNode : public MaterialNode
	{
	public:
		OutputNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Output", f2Pos)
		{
			m_vecInputSlot.reserve(13);
			CreateInputSlot("BaseColor", MaterialNode::InOutSlot::eFloat3);
			CreateInputSlot("SpecularColor", MaterialNode::InOutSlot::eFloat3);
			CreateInputSlot("EmissiveColor", MaterialNode::InOutSlot::eFloat3);
			CreateInputSlot("EmissiveIntensity", MaterialNode::InOutSlot::eFloat);

			CreateInputSlot("Normal", MaterialNode::InOutSlot::eFloat3);

			CreateInputSlot("Roughness", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("Metallic", MaterialNode::InOutSlot::eFloat);

			CreateInputSlot("Surface", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("Specular", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("SpecularTint", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("Anisotropic", MaterialNode::InOutSlot::eFloat);

			CreateInputSlot("Sheen", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("SheenTint", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("Clearcoat", MaterialNode::InOutSlot::eFloat);
			CreateInputSlot("ClearcoatGloss", MaterialNode::InOutSlot::eFloat);
		}

		virtual ~OutputNode() {}
	};

	class InputNode : public MaterialNode
	{
	public:
		InputNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Input", f2Pos)
		{
			m_vecOutputSlot.reserve(8);
			CreateOutputSlot("Position", MaterialNode::InOutSlot::eFloat4);
			CreateOutputSlot("WorldPosition", MaterialNode::InOutSlot::eFloat4);
			CreateOutputSlot("WorldVIewPosition", MaterialNode::InOutSlot::eFloat4);
			CreateOutputSlot("WorldViewProjPosition", MaterialNode::InOutSlot::eFloat4);
			CreateOutputSlot("UV", MaterialNode::InOutSlot::eFloat2);
			CreateOutputSlot("NormalDirection", MaterialNode::InOutSlot::eFloat3);
			CreateOutputSlot("TangentDirection", MaterialNode::InOutSlot::eFloat3);
			CreateOutputSlot("BinormalDirection", MaterialNode::InOutSlot::eFloat3);
		}

		virtual ~InputNode() {}
	};

	class ValueFloatNode : public MaterialNode
	{
	public:
		ValueFloatNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Float", f2Pos)
			, m_fValue(0.f)
		{
			m_vecOutputSlot.reserve(1);

			CreateOutputSlot("x", MaterialNode::InOutSlot::eFloat, est::math::float2(50.f, 20.f));
		}

		virtual ~ValueFloatNode() {}

		virtual void RenderNode(ImDrawList* pDrawList, const est::math::float2& f2Offset);
		virtual void RenderNodeBackground(ImDrawList* pDrawList, const est::math::float2& f2Offset);

	private:
		float m_fValue;
	};

	class ValueFloat2Node : public MaterialNode
	{
	public:
		ValueFloat2Node(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Float2", f2Pos)
		{
			CreateOutputSlot("xy", MaterialNode::InOutSlot::eFloat2, est::math::float2(50.f, 20.f));
			CreateOutputSlot("x", MaterialNode::InOutSlot::eFloat, est::math::float2(50.f, 20.f));
			CreateOutputSlot("y", MaterialNode::InOutSlot::eFloat, est::math::float2(50.f, 20.f));
		}

		virtual ~ValueFloat2Node() {}

		virtual void RenderNode(ImDrawList* pDrawList, const est::math::float2& f2Offset);
		virtual void RenderNodeBackground(ImDrawList* pDrawList, const est::math::float2& f2Offset);

	private:
		est::math::float2 m_f2Value;
	};

	class FuncAddNode : public MaterialNode
	{
	public:
		FuncAddNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Add", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");
			
			CreateOutputSlot("X");
		}

		virtual ~FuncAddNode() {}
	};

	class FuncSubtractNode : public MaterialNode
	{
	public:
		FuncSubtractNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Subtract", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncSubtractNode() {}
	};

	class FuncMultiplyNode : public MaterialNode
	{
	public:
		FuncMultiplyNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Multiply", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncMultiplyNode() {}
	};

	class FuncDivideNode : public MaterialNode
	{
	public:
		FuncDivideNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Divide", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncDivideNode() {}
	};

	class FuncReciprocalNode : public MaterialNode
	{
	public:
		FuncReciprocalNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Reciprocal", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncReciprocalNode() {}
	};

	class FuncPowerNode : public MaterialNode
	{
	public:
		FuncPowerNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Power", f2Pos)
		{
			CreateInputSlot("Val");
			CreateInputSlot("Exp");

			CreateOutputSlot("X");
		}

		virtual ~FuncPowerNode() {}
	};

	class FuncSqrtNode : public MaterialNode
	{
	public:
		FuncSqrtNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Sqrt", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncSqrtNode() {}
	};

	class FuncLogNode : public MaterialNode
	{
	public:
		FuncLogNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Log", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncLogNode() {}
	};

	class FuncMinNode : public MaterialNode
	{
	public:
		FuncMinNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Min", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncMinNode() {}
	};

	class FuncMaxNode : public MaterialNode
	{
	public:
		FuncMaxNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Max", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncMaxNode() {}
	};

	class FuncAbsNode : public MaterialNode
	{
	public:
		FuncAbsNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Abs", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncAbsNode() {}
	};

	class FuncSignNode : public MaterialNode
	{
	public:
		FuncSignNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Sign", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncSignNode() {}
	};

	class FuncCeilNode : public MaterialNode
	{
	public:
		FuncCeilNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Ceil", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncCeilNode() {}
	};

	class FuncRoundNode : public MaterialNode
	{
	public:
		FuncRoundNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Round", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncRoundNode() {}
	};

	class FuncFloorNode : public MaterialNode
	{
	public:
		FuncFloorNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Floor", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncFloorNode() {}
	};

	class FuncTruncNode : public MaterialNode
	{
	public:
		FuncTruncNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Trunc", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncTruncNode() {}
	};

	class FuncStepNode : public MaterialNode
	{
	public:
		FuncStepNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Step", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncStepNode() {}
	};

	class FuncSmoothstepNode : public MaterialNode
	{
	public:
		FuncSmoothstepNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Smoothstep", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");
			CreateInputSlot("C");

			CreateOutputSlot("X");
		}

		virtual ~FuncSmoothstepNode() {}
	};

	class FuncIfNode : public MaterialNode
	{
	public:
		FuncIfNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "If", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncIfNode() {}
	};

	class FuncFracNode : public MaterialNode
	{
	public:
		FuncFracNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Frac", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncFracNode() {}
	};

	class FuncFmodNode : public MaterialNode
	{
	public:
		FuncFmodNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Fmod", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncFmodNode() {}
	};

	class FuncClampNode : public MaterialNode
	{
	public:
		FuncClampNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Clamp", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncClampNode() {}
	};

	class FuncLerpNode : public MaterialNode
	{
	public:
		FuncLerpNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Lerp", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");
			CreateInputSlot("C");

			CreateOutputSlot("X");
		}

		virtual ~FuncLerpNode() {}
	};

	class FuncDotNode : public MaterialNode
	{
	public:
		FuncDotNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Dot", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncDotNode() {}
	};

	class FuncCrossNode : public MaterialNode
	{
	public:
		FuncCrossNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Cross", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncCrossNode() {}
	};

	class FuncReflectNode : public MaterialNode
	{
	public:
		FuncReflectNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Reflect", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncReflectNode() {}
	};

	class FuncNormalizeNode : public MaterialNode
	{
	public:
		FuncNormalizeNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Normalize", f2Pos)
		{
			CreateInputSlot("A");
			CreateInputSlot("B");

			CreateOutputSlot("X");
		}

		virtual ~FuncNormalizeNode() {}
	};

	class FuncSinNode : public MaterialNode
	{
	public:
		FuncSinNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Sin", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncSinNode() {}
	};

	class FuncCosNode : public MaterialNode
	{
	public:
		FuncCosNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Cos", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncCosNode() {}
	};

	class FuncTanNode : public MaterialNode
	{
	public:
		FuncTanNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "Tan", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncTanNode() {}
	};

	class FuncArcSinNode : public MaterialNode
	{
	public:
		FuncArcSinNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "ArcSin", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncArcSinNode() {}
	};

	class FuncArcCosNode : public MaterialNode
	{
	public:
		FuncArcCosNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "ArcCos", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncArcCosNode() {}
	};

	class FuncArcTanNode : public MaterialNode
	{
	public:
		FuncArcTanNode(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "ArcTan", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncArcTanNode() {}
	};

	class FuncArcTan2Node : public MaterialNode
	{
	public:
		FuncArcTan2Node(MaterialNodeManager* pManager, int nID, const est::math::float2& f2Pos)
			: MaterialNode(pManager, nID, "ArcTan2", f2Pos)
		{
			CreateInputSlot("A");

			CreateOutputSlot("X");
		}

		virtual ~FuncArcTan2Node() {}
	};

public:
	MaterialNodeManager();
	~MaterialNodeManager();

	void AddNode(MaterialNode* pNode) { m_vecNodes.emplace_back(pNode); }

	void Update(bool& isOpend);

public:
	MaterialNode* GetMaterialNode(int nID)
	{
		for (auto pNode : m_vecNodes)
		{
			if (pNode->GetID() == nID)
				return pNode;
		}

		return nullptr;
	}

	int GetHoveredNodeIDInList() { return m_nHoveredNodeIDInList; }
	void SetHoveredNodeIDInList(int nHoveredNodeIDInList) { m_nHoveredNodeIDInList = nHoveredNodeIDInList; }

	int GetHoveredNodeIDInScene() { return m_nHoveredNodeIDInScene; }
	void SetHoveredNodeIDInScene(int nHoveredNodeIDInScene) { m_nHoveredNodeIDInScene = nHoveredNodeIDInScene; }

	bool GetIsOpenContextMenu() { return m_isOpenContextMenu; }
	void SetIsOpenContextMenu(bool isOpenContextMenu) { m_isOpenContextMenu = isOpenContextMenu; }

	int GetSelectedNodeID() { return m_nSelectedNodeID; }
	void SetSelectedNodeID(int nSelectedNodeID) { m_nSelectedNodeID = nSelectedNodeID; }

	int GetSelectedNodeSlotIndex() { return m_nSelectedNodeSlotIndex; }
	void SetSelectedNodeSlotIndex(int nSelectedNodeSlotIndex) { m_nSelectedNodeSlotIndex = nSelectedNodeSlotIndex; }

	MaterialNode::InOutSlot::Type GetSelectedNodeSlotType() { return m_emSelectedSlotType; }
	void SetSelectedNodeSlotType(MaterialNode::InOutSlot::Type emType) { m_emSelectedSlotType = emType; }

	int GetHoveredNodeID() { return m_nHoveredNodeID; }
	void SetHoveredNodeID(int nHoveredNodeID) { m_nHoveredNodeID = nHoveredNodeID; }

	void SetHoveredNodeSlotIndex(int nHoveredNodeSlotIndex) { m_nHoveredNodeSlotIndex = nHoveredNodeSlotIndex; }
	int GetHoveredNodeSlotIndex() { return m_nHoveredNodeSlotIndex; }

private:
	int AllocateID() { return m_nNodeID++; }

private:
	int m_nNodeID;
	std::vector<MaterialNode*> m_vecNodes;

	// perframe init
	int m_nHoveredNodeIDInList;
	int m_nHoveredNodeIDInScene;
	bool m_isOpenContextMenu;

	// maintain
	bool m_isShowGrid;
	int m_nSelectedNodeID;
	int m_nSelectedNodeSlotIndex;

	int m_nHoveredNodeID;
	int m_nHoveredNodeSlotIndex;

	MaterialNode::InOutSlot::Type m_emSelectedSlotType;
	est::math::float2 m_f2Scrolling;

	/*struct NodeLink
	{
		int nOutputID = -1;
		int nOutputSlotIdx = -1;

		int nInputID = -1;
		int nInputSlotIdx = -1;

		NodeLink() {}
		NodeLink(int nOutputID, int nOutputSlotIdx, int nInputID, int nInputSlotIdx)
			: nOutputID(nOutputID)
			, nOutputSlotIdx(nOutputSlotIdx)
			, nInputID(nInputID)
			, nInputSlotIdx(nInputSlotIdx)
		{
		}
	};
	std::list<NodeLink> m_listLInks;*/
};