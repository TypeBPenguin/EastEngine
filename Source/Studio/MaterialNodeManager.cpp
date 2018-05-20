#include "stdafx.h"
#include "MaterialNodeManager.h"

#include "imgui.h"
#include "imguiConvertor.h"

using namespace eastengine;

const float NODE_SLOT_RADIUS = 4.f;
const math::Vector2 NODE_WINDOW_PADDING(8.f, 8.f);

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

void MaterialNode::InOutSlot::Render(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	isActive = false;
	isHovered = false;

	int nID = ImGui::GetID(strLabel.c_str());

	ImGui::PushID(nID);
	ImGui::PushStyleColor(ImGuiCol_Button, ImColor(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(0, 0, 0, 0));

	ImGui::Button(strLabel.c_str(), math::Convert(f2ButtonSize));

	ImGui::PopStyleColor(3);
	ImGui::PopID();

	if (ImGui::IsItemActive() == true)
	{
		pManager->SetSelectedNodeSlotType(emType);
		pManager->SetSelectedNodeSlotIndex(nSlotIdx);

		isActive = true;
	}

	if (ImGui::IsItemHoveredRect() == true)
	{
		bool isNone = pManager->GetSelectedNodeSlotType() == eNone;
		if (isNone == true || (pManager->GetSelectedNodeID() != -1 && pManager->GetSelectedNodeID() != nNodeID))
		{
			if (isNone == true || (pManager->GetSelectedNodeSlotType() == eOut && emType == eIn) || (pManager->GetSelectedNodeSlotType() == eIn && emType == eOut))
			{
				isHovered = true;

				if (isNone == false)
				{
					pManager->SetHoveredNodeID(nNodeID);
					pManager->SetHoveredNodeSlotIndex(nSlotIdx);
				}
			}
		}
	}

	math::Vector2 f2CursorPos(math::Convert(ImGui::GetCursorPos()));
	
	if (f2Size == math::Vector2::Zero)
	{
		f2Size = math::Convert(ImGui::GetItemRectSize());
	}

	f2Pos = f2LinkerPos = f2CursorPos;

	if (emType == Type::eIn)
	{
		f2LinkerPos.y -= f2Size.y * 0.5f;
	}
	else
	{
		f2LinkerPos.x += f2Size.x;
		f2LinkerPos.y -= f2Size.y * 0.5f;
	}
}

void MaterialNode::InOutSlot::RenderBackground(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	math::Vector2 f2RectMin(f2Offset + f2Pos);
	f2RectMin.y -= f2Size.y;

	math::Vector2 f2RectMax(f2RectMin + f2Size);

	ImColor color;
	switch (emValueType)
	{
	case ValueType::eFloat:
		color = ImColor(32, 192, 32, 128);

		if (isActive == true)
		{
			color = ImColor(32, 64, 32, 128);
		}
		else if (isHovered == true)
		{
			color = ImColor(32, 96, 32, 128);
		}
		break;
	case ValueType::eFloat2:
		color = ImColor(32, 32, 192, 128);

		if (isActive == true)
		{
			color = ImColor(32, 32, 64, 128);
		}
		else if (isHovered == true)
		{
			color = ImColor(32, 32, 96, 128);
		}
		break;
	case ValueType::eFloat3:
		color = ImColor(192, 32, 32, 128);

		if (isActive == true)
		{
			color = ImColor(64, 32, 32, 128);
		}
		else if (isHovered == true)
		{
			color = ImColor(96, 32, 32, 128);
		}
		break;
	case ValueType::eFloat4:
		color = ImColor(32, 192, 192, 128);

		if (isActive == true)
		{
			color = ImColor(32, 64, 64, 128);
		}
		else if (isHovered == true)
		{
			color = ImColor(32, 96, 96, 128);
		}
		break;
	}

	pDrawList->AddRectFilled(math::Convert(f2RectMin), math::Convert(f2RectMax), color, 4.f);
	pDrawList->AddRect(math::Convert(f2RectMin), math::Convert(f2RectMax), color, 4.f);

	pDrawList->AddCircleFilled(math::Convert(f2Offset + f2LinkerPos), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));
}

void MaterialNode::RenderNode(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	ImGui::Text("%s", m_strName.c_str());

	if (m_vecInputSlot.empty() == false)
	{
		ImGui::BeginGroup();
		for (auto& slot : m_vecInputSlot)
		{
			slot.Render(pDrawList, f2Offset);
		}
		ImGui::EndGroup();

		ImGui::SameLine();
	}

	if (m_vecOutputSlot.empty() == false)
	{
		ImGui::BeginGroup();
		for (auto& slot : m_vecOutputSlot)
		{
			slot.Render(pDrawList, f2Offset);
		}
		ImGui::EndGroup();
	}
}

void MaterialNode::RenderNodeBackground(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	if (m_vecInputSlot.empty() == false)
	{
		for (auto& slot : m_vecInputSlot)
		{
			slot.RenderBackground(pDrawList, f2Offset);
		}
	}

	if (m_vecOutputSlot.empty() == false)
	{
		for (auto& slot : m_vecOutputSlot)
		{
			slot.RenderBackground(pDrawList, f2Offset);
		}
	}
}

void MaterialNode::Update(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	ImGui::PushID(m_nID);
	eastengine::math::Vector2 f2NodeRectMin(f2Offset + m_f2Pos);

	// Display node contents first
	pDrawList->ChannelsSetCurrent(1); // Foreground
	bool isOldAnyActive = ImGui::IsAnyItemActive();

	ImGui::SetCursorScreenPos(math::Convert(f2NodeRectMin + NODE_WINDOW_PADDING));

	ImGui::BeginGroup(); // Lock horizontal position

	RenderNode(pDrawList, f2Offset);

	ImGui::EndGroup();

	// Save the size of what we have emitted and whether any of the widgets are being used
	bool isNodeActive = (!isOldAnyActive && ImGui::IsAnyItemActive());
	m_f2Size = math::Convert(ImGui::GetItemRectSize()) + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;

	math::Vector2 f2NodeRectMax(f2NodeRectMin + m_f2Size);

	// Display node box
	pDrawList->ChannelsSetCurrent(0); // Background

	ImGui::SetCursorScreenPos(math::Convert(f2NodeRectMin));

	ImGui::InvisibleButton("node", math::Convert(m_f2Size));

	if (ImGui::IsItemHovered())
	{
		m_pManager->SetHoveredNodeIDInScene(m_nID);

		bool isOpenContextMenu = m_pManager->GetIsOpenContextMenu();
		isOpenContextMenu |= ImGui::IsMouseClicked(1);
		m_pManager->SetIsOpenContextMenu(isOpenContextMenu);
	}

	bool isNodeMovingActive = ImGui::IsItemActive();
	if (isNodeActive || isNodeMovingActive)
	{
		m_pManager->SetSelectedNodeID(m_nID);
	}

	if (isNodeMovingActive && ImGui::IsMouseDragging(0))
	{
		m_f2Pos += math::Convert(ImGui::GetIO().MouseDelta);
	}

	ImU32 nodeBgColor = (m_pManager->GetHoveredNodeIDInList() == m_nID || m_pManager->GetHoveredNodeIDInScene() == m_nID || (m_pManager->GetHoveredNodeIDInList() == -1 && m_pManager->GetSelectedNodeID() == m_nID)) ? ImColor(75, 75, 75) : ImColor(60, 60, 60);

	pDrawList->AddRectFilled(math::Convert(f2NodeRectMin), math::Convert(f2NodeRectMax), nodeBgColor, 4.f);
	pDrawList->AddRect(math::Convert(f2NodeRectMin), math::Convert(f2NodeRectMax), ImColor(100, 100, 100), 4.f);

	RenderNodeBackground(pDrawList, f2Offset);

	ImGui::PopID();
}

void MaterialNodeManager::ValueFloatNode::RenderNode(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	ImGui::Text("%s", m_strName.c_str());

	ImGui::InvisibleButton("TempFloat", ImVec2(50.f, 20.f));

	ImGui::PushItemWidth(50.f);
	ImGui::DragFloat("x", &m_fValue, 0.001f, FLT_MIN, FLT_MAX);
	ImGui::PopItemWidth();

	if (m_vecInputSlot.empty() == false)
	{
		ImGui::BeginGroup();
		for (auto& slot : m_vecInputSlot)
		{
			slot.Render(pDrawList, f2Offset);
		}
		ImGui::EndGroup();

		ImGui::SameLine();
	}

	if (m_vecOutputSlot.empty() == false)
	{
		ImGui::BeginGroup();
		for (auto& slot : m_vecOutputSlot)
		{
			slot.Render(pDrawList, f2Offset);
		}
		ImGui::EndGroup();
	}
}

void MaterialNodeManager::ValueFloatNode::RenderNodeBackground(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	math::Vector2 f2NodeRectMin(f2Offset + m_f2Pos);
	f2NodeRectMin.x += m_f2Size.x * 0.1f;
	f2NodeRectMin.y += m_f2Size.y * 0.25f;

	math::Vector2 f2NodeRectMax(f2NodeRectMin);
	f2NodeRectMax.x += m_f2Size.x * 0.7f;
	f2NodeRectMax.y += m_f2Size.y * 0.25f;

	ImColor color(m_fValue, 0.f, 0.f, 1.f);

	pDrawList->AddRectFilled(math::Convert(f2NodeRectMin), math::Convert(f2NodeRectMax), color, 4.f);
	pDrawList->AddRect(math::Convert(f2NodeRectMin), math::Convert(f2NodeRectMax), ImColor(100, 100, 100), 4.f);

	MaterialNode::RenderNodeBackground(pDrawList, f2Offset);
}

void MaterialNodeManager::ValueFloat2Node::RenderNode(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	ImGui::Text("%s", m_strName.c_str());

	ImGui::InvisibleButton("TempFloat", ImVec2(60.f, 20.f));

	ImGui::PushItemWidth(30.f);
	ImGui::DragFloat("x", &m_f2Value.x, 0.001f, FLT_MIN, FLT_MAX);
	ImGui::SameLine();
	ImGui::DragFloat("y", &m_f2Value.y, 0.001f, FLT_MIN, FLT_MAX);
	ImGui::PopItemWidth();

	if (m_vecInputSlot.empty() == false)
	{
		ImGui::BeginGroup();
		for (auto& slot : m_vecInputSlot)
		{
			slot.Render(pDrawList, f2Offset);
		}
		ImGui::EndGroup();

		ImGui::SameLine();
	}

	if (m_vecOutputSlot.empty() == false)
	{
		ImGui::BeginGroup();
		for (auto& slot : m_vecOutputSlot)
		{
			slot.Render(pDrawList, f2Offset);
		}
		ImGui::EndGroup();
	}
}

void MaterialNodeManager::ValueFloat2Node::RenderNodeBackground(ImDrawList* pDrawList, const eastengine::math::Vector2& f2Offset)
{
	math::Vector2 f2NodeRectMin(f2Offset + m_f2Pos);
	f2NodeRectMin.x += m_f2Size.x * 0.075f;
	f2NodeRectMin.y += m_f2Size.y * 0.175f;

	math::Vector2 f2NodeRectMax(f2NodeRectMin);
	f2NodeRectMax.x += m_f2Size.x * 0.85f;
	f2NodeRectMax.y += m_f2Size.y * 0.15f;

	ImColor color(m_f2Value.x, m_f2Value.y, 0.f, 1.f);

	pDrawList->AddRectFilled(math::Convert(f2NodeRectMin), math::Convert(f2NodeRectMax), color, 4.f);
	pDrawList->AddRect(math::Convert(f2NodeRectMin), math::Convert(f2NodeRectMax), ImColor(100, 100, 100), 4.f);

	MaterialNode::RenderNodeBackground(pDrawList, f2Offset);
}

MaterialNodeManager::MaterialNodeManager()
	: m_nNodeID(0)
	, m_nHoveredNodeIDInList(-1)
	, m_nHoveredNodeIDInScene(-1)
	, m_isOpenContextMenu(false)
	, m_isShowGrid(false)
	, m_nSelectedNodeID(-1)
	, m_nSelectedNodeSlotIndex(-1)
	, m_nHoveredNodeID(-1)
	, m_nHoveredNodeSlotIndex(-1)
	, m_emSelectedSlotType(MaterialNode::InOutSlot::eNone)
{
	m_vecNodes.emplace_back(new InputNode(this, AllocateID(), math::Vector2(50.f, 50.f)));
	m_vecNodes.emplace_back(new OutputNode(this, AllocateID(), math::Vector2(350.f, 50.f)));
}

MaterialNodeManager::~MaterialNodeManager()
{
	std::for_each(m_vecNodes.begin(), m_vecNodes.end(), DeleteSTLObject());
	m_vecNodes.clear();
}

void MaterialNodeManager::Update(bool& isOpend)
{
	if (ImGui::Begin("Example: Custom Node Graph", &isOpend) == false)
	{
		ImGui::End();
		return;
	}

	bool isMouseDragging = ImGui::IsMouseDragging(0);
	//LOG_MESSAGE("IsMouseDragging[%d], SelectedID[%d, %d], HoveredID[%d, %d]", isMouseDragging, m_nSelectedNodeID, m_nSelectedNodeSlotIndex, m_nHoveredNodeID, m_nHoveredNodeSlotIndex);

	m_nHoveredNodeIDInList = -1;
	m_nHoveredNodeIDInScene = -1;
	m_isOpenContextMenu = false;

	bool isAnyHoveredSlot = false;
	bool isAnyActiveSLot = false;

	ImGui::BeginChild("node_list", ImVec2(100, 100));
	ImGui::Text("Nodes");
	ImGui::Separator();
	for (auto pNode : m_vecNodes)
	{
		ImGui::PushID(pNode->GetID());
		if (ImGui::Selectable(pNode->GetName(), pNode->GetID() == m_nSelectedNodeID))
		{
			//m_nSelectedNodeID = pNode->GetID();
		}

		if (ImGui::IsItemHovered())
		{
			m_nHoveredNodeIDInList = pNode->GetID();
			m_isOpenContextMenu |= ImGui::IsMouseClicked(1);
		}

		isAnyHoveredSlot |= pNode->IsSlotHovered();
		isAnyActiveSLot |= pNode->IsSlotActive();

		ImGui::PopID();
	}
	ImGui::EndChild();

	if (isAnyHoveredSlot == false)
	{
		m_nHoveredNodeID = m_nHoveredNodeSlotIndex = -1;
	}

	if (isAnyActiveSLot == false)
	{
		m_nSelectedNodeID = m_nSelectedNodeSlotIndex = -1;
	}

	ImGui::SameLine();
	ImGui::BeginGroup();

	// Create our child canvas
	ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", m_f2Scrolling.x, m_f2Scrolling.y);
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	ImGui::Checkbox("Show grid", &m_isShowGrid);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(60, 60, 70, 200));
	ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::PushItemWidth(120.0f);

	math::Vector2 f2Offset = math::Convert(ImGui::GetCursorScreenPos()) - m_f2Scrolling;
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	pDrawList->ChannelsSplit(2);

	// Display grid
	if (m_isShowGrid)
	{
		ImU32 GRID_COLOR = ImColor(200, 200, 200, 40);
		float GRID_SZ = 64.f;
		ImVec2 win_pos = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetWindowSize();
		for (float x = fmodf(f2Offset.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
		{
			pDrawList->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
		}

		for (float y = fmodf(f2Offset.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
		{
			pDrawList->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
		}
	}

	// Display links
	pDrawList->ChannelsSetCurrent(1); // Background
	if (isMouseDragging == false)
	{
		if (m_nSelectedNodeID != -1 && m_nSelectedNodeSlotIndex != -1 && m_nHoveredNodeID != -1 && m_nHoveredNodeSlotIndex != -1 && m_emSelectedSlotType != MaterialNode::InOutSlot::eNone)
		{
			if (m_emSelectedSlotType == MaterialNode::InOutSlot::eIn)
			{
				MaterialNode* pOutputNode = GetMaterialNode(m_nHoveredNodeID);
				MaterialNode* pInputNode = GetMaterialNode(m_nSelectedNodeID);

				if (pOutputNode != nullptr || pInputNode != nullptr)
				{
					const MaterialNode::InOutSlot* pOutputSlot = pOutputNode->GetOutputSlot(m_nHoveredNodeSlotIndex);
					const MaterialNode::InOutSlot* pInputSlot = pInputNode->GetInputSlot(m_nSelectedNodeSlotIndex);

					if (pOutputSlot != nullptr && pInputSlot != nullptr)
					{
						pOutputNode->LinkToInput(m_nHoveredNodeID, m_nHoveredNodeSlotIndex, m_nSelectedNodeID, m_nSelectedNodeSlotIndex);
						pInputNode->LinkToOutput(m_nHoveredNodeID, m_nHoveredNodeSlotIndex, m_nSelectedNodeID, m_nSelectedNodeSlotIndex);
					}
				}
			}
			else if (m_emSelectedSlotType == MaterialNode::InOutSlot::eOut)
			{
				MaterialNode* pInputNode = GetMaterialNode(m_nHoveredNodeID);
				MaterialNode* pOutputNode = GetMaterialNode(m_nSelectedNodeID);
				if (pInputNode != nullptr && pOutputNode != nullptr)
				{
					const MaterialNode::InOutSlot* pInputSlot = pInputNode->GetInputSlot(m_nHoveredNodeSlotIndex);
					const MaterialNode::InOutSlot* pOutputSlot = pOutputNode->GetOutputSlot(m_nSelectedNodeSlotIndex);

					if (pOutputSlot != nullptr && pInputSlot != nullptr)
					{
						const std::list<MaterialNode::Link>& list = pInputNode->GetLinkToOutput();
						auto iter = std::find_if(list.begin(), list.end(), [&](const MaterialNode::Link& link)
						{
							return link.nInputNodeID == m_nHoveredNodeID && link.nInputNodeSlotIndex == m_nHoveredNodeSlotIndex;
						});

						if (iter == list.end())
						{
							pInputNode->LinkToOutput(m_nSelectedNodeID, m_nSelectedNodeSlotIndex, m_nHoveredNodeID, m_nHoveredNodeSlotIndex);
							pOutputNode->LinkToInput(m_nSelectedNodeID, m_nSelectedNodeSlotIndex, m_nHoveredNodeID, m_nHoveredNodeSlotIndex);
						}
					}
				}
			}

			m_nSelectedNodeID = m_nSelectedNodeSlotIndex = m_nHoveredNodeID = m_nHoveredNodeSlotIndex = -1;
			m_emSelectedSlotType = MaterialNode::InOutSlot::eNone;
		}
	}
	else
	{
		if (m_nSelectedNodeID != -1 && m_nSelectedNodeSlotIndex != -1 && m_emSelectedSlotType != MaterialNode::InOutSlot::eNone)
		{
			MaterialNode* pNode = GetMaterialNode(m_nSelectedNodeID);
			if (pNode != nullptr)
			{
				ImVec2 p1;
				ImVec2 p2;
				float fDistance = 0.f;

				const MaterialNode::InOutSlot* pSlot = nullptr;
				if (m_emSelectedSlotType == MaterialNode::InOutSlot::eIn)
				{
					pSlot = pNode->GetInputSlot(m_nSelectedNodeSlotIndex);
					if (pSlot != nullptr)
					{
						p1 = ImVec2(ImGui::GetMousePos());
						p2 = ImVec2(math::Convert(f2Offset + pSlot->GetLinkerPos()));

						fDistance = math::Vector2::Distance(math::Convert(p1), math::Convert(p2)) * 0.5f;
					}
				}
				else if (m_emSelectedSlotType == MaterialNode::InOutSlot::eOut)
				{
					pSlot = pNode->GetOutputSlot(m_nSelectedNodeSlotIndex);
					if (pSlot != nullptr)
					{
						p1 = ImVec2(math::Convert(f2Offset + pSlot->GetLinkerPos()));
						p2 = ImVec2(ImGui::GetMousePos());

						fDistance = math::Vector2::Distance(math::Convert(p1), math::Convert(p2)) * 0.5f;
					}
				}

				if (pSlot != nullptr)
				{
					switch (pSlot->GetValueType())
					{
					case MaterialNode::InOutSlot::ValueType::eFloat:
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 2.f);
						break;
					case MaterialNode::InOutSlot::ValueType::eFloat2:
						p1.y -= 1.5f;
						p2.y -= 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.75f);

						p1.y += 3.f;
						p2.y += 3.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.75f);
						break;
					case MaterialNode::InOutSlot::ValueType::eFloat3:
						p1.y -= 2.f;
						p2.y -= 2.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.5f);

						p1.y += 2.f;
						p2.y += 2.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.5f);

						p1.y += 2.f;
						p2.y += 2.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.5f);
						break;
					case MaterialNode::InOutSlot::ValueType::eFloat4:
						p1.y -= 3.f;
						p2.y -= 3.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);

						p1.y += 1.5f;
						p2.y += 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);

						p1.y += 1.5f;
						p2.y += 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);

						p1.y += 1.5f;
						p2.y += 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);
						break;
					}
				}
			}
		}
	}

	for (auto& pNode : m_vecNodes)
	{
		std::list<MaterialNode::Link>& listLInk = pNode->GetLinkToInput();
		for (auto iter = listLInk.begin(); iter != listLInk.end();)
		{
			MaterialNode::Link& link = *iter;

			bool isValid = false;

			MaterialNode* pInputNode = GetMaterialNode(link.nInputNodeID);
			if (pInputNode != nullptr)
			{
				const MaterialNode::InOutSlot* pOutputSlot = pNode->GetOutputSlot(link.nOutputNodeSlotIndex);
				const MaterialNode::InOutSlot* pInputSlot = pInputNode->GetInputSlot(link.nInputNodeSlotIndex);
				if (pOutputSlot != nullptr && pInputSlot != nullptr)
				{
					ImVec2 p1(math::Convert(f2Offset + pOutputSlot->GetLinkerPos()));
					ImVec2 p2(math::Convert(f2Offset + pInputSlot->GetLinkerPos()));

					float fDistance = math::Vector2::Distance(math::Convert(p1), math::Convert(p2)) * 0.5f;

					switch (pOutputSlot->GetValueType())
					{
					case MaterialNode::InOutSlot::ValueType::eFloat:
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 2.f);
						break;
					case MaterialNode::InOutSlot::ValueType::eFloat2:
						p1.y -= 1.5f;
						p2.y -= 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.75f);

						p1.y += 3.f;
						p2.y += 3.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.75f);
						break;
					case MaterialNode::InOutSlot::ValueType::eFloat3:
						p1.y -= 2.f;
						p2.y -= 2.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.5f);

						p1.y += 2.f;
						p2.y += 2.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.5f);

						p1.y += 2.f;
						p2.y += 2.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.5f);
						break;
					case MaterialNode::InOutSlot::ValueType::eFloat4:
						p1.y -= 3.f;
						p2.y -= 3.f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);

						p1.y += 1.5f;
						p2.y += 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);

						p1.y += 1.5f;
						p2.y += 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);

						p1.y += 1.5f;
						p2.y += 1.5f;
						pDrawList->AddBezierCurve(p1, p1 + ImVec2(+fDistance, 0), p2 + ImVec2(-fDistance, 0), p2, ImColor(200, 200, 100), 1.25f);
						break;
					}

					isValid = true;
				}
			}

			if (isValid == false)
			{
				iter = listLInk.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}

	// Display nodes
	for (auto& pNode : m_vecNodes)
	{
		pNode->Update(pDrawList, f2Offset);
	}
	pDrawList->ChannelsMerge();

	// Open context menu
	if (ImGui::IsAnyItemHovered() == false && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
	{
		m_nSelectedNodeID = m_nSelectedNodeSlotIndex = m_nHoveredNodeID = m_nHoveredNodeSlotIndex = m_nHoveredNodeIDInList = m_nHoveredNodeIDInScene = -1;
		m_emSelectedSlotType = MaterialNode::InOutSlot::eNone;
		m_isOpenContextMenu = true;
	}

	if (m_isOpenContextMenu == true)
	{
		ImGui::OpenPopup("context_menu");
		if (m_nHoveredNodeIDInList != -1)
		{
			m_nSelectedNodeID = m_nHoveredNodeIDInList;
		}

		if (m_nHoveredNodeIDInScene != -1)
		{
			m_nSelectedNodeID = m_nHoveredNodeIDInScene;
		}
	}

	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("context_menu"))
	{
		MaterialNode* pNode = GetMaterialNode(m_nSelectedNodeID);

		math::Vector2 f2ScenePos(math::Convert(ImGui::GetMousePosOnOpeningCurrentPopup()));
		f2ScenePos -= f2Offset;

		if (pNode != nullptr)
		{
			ImGui::Text("Node '%s'", pNode->GetName());
			ImGui::Separator();
			if (ImGui::MenuItem("Delete"))
			{
				auto iter = std::find_if(m_vecNodes.begin(), m_vecNodes.end(), [&](MaterialNode* pNode)
				{
					return pNode->GetID() == m_nSelectedNodeID;
				});

				if (iter != m_vecNodes.end())
				{
					SafeDelete(*iter);
					m_vecNodes.erase(iter);
				}
			}
		}
		else
		{
			if (ImGui::BeginMenu("Input"))
			{

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Value"))
			{
				if (ImGui::MenuItem("Float"))
				{
					m_vecNodes.emplace_back(new ValueFloatNode(this, AllocateID(), f2ScenePos));
				}

				if (ImGui::MenuItem("Float2"))
				{
					m_vecNodes.emplace_back(new ValueFloat2Node(this, AllocateID(), f2ScenePos));
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Function"))
			{
				if (ImGui::MenuItem("Add"))
				{
					m_vecNodes.emplace_back(new FuncAddNode(this, AllocateID(), f2ScenePos));
				}

				if (ImGui::MenuItem("Subtract"))
				{
					m_vecNodes.emplace_back(new FuncSubtractNode(this, AllocateID(), f2ScenePos));
				}

				if (ImGui::MenuItem("Multiply"))
				{
					m_vecNodes.emplace_back(new FuncMultiplyNode(this, AllocateID(), f2ScenePos));
				}

				if (ImGui::MenuItem("Divide"))
				{
					m_vecNodes.emplace_back(new FuncDivideNode(this, AllocateID(), f2ScenePos));
				}

				ImGui::EndMenu();
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Scrolling
	if (ImGui::IsWindowHovered() && ImGui::IsAnyItemActive() == false && ImGui::IsMouseDragging(2, 0.f))
	{
		m_f2Scrolling -= math::Convert(ImGui::GetIO().MouseDelta);
	}

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::EndGroup();

	ImGui::End();
}
