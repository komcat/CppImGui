#include "GraphEditor.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <imgui_internal.h>

// Constants
const float NODE_RADIUS = 30.0f;
const ImU32 NODE_COLOR = IM_COL32(100, 150, 250, 255);
const ImU32 NODE_SELECTED_COLOR = IM_COL32(250, 100, 100, 255);
const ImU32 EDGE_COLOR = IM_COL32(200, 200, 200, 255);
const ImU32 EDGE_SELECTED_COLOR = IM_COL32(250, 150, 50, 255);
const ImU32 CANVAS_BG_COLOR = IM_COL32(50, 50, 50, 255);
const float EDGE_THICKNESS = 2.0f;
const float ARROW_SIZE = 10.0f;
const float PI = 3.14159265358979323846f;

GraphEditor::GraphEditor() {}

void GraphEditor::setModel(std::shared_ptr<GraphModel> graphModel) {
    model = graphModel;

    // Auto-select the first graph if available
    auto graphNames = model->getGraphNames();
    if (!graphNames.empty()) {
        currentGraphName = graphNames[0];
        currentGraph = model->getGraph(currentGraphName);
    }
}

void GraphEditor::render() {
    if (!model) {
        ImGui::Text("No graph model loaded");
        return;
    }

    renderMainMenu();

    ImGui::Columns(2, "GraphEditorColumns", true);

    // Left column - Controls
    ImGui::BeginChild("ControlsPanel", ImVec2(0, 0), true);

    // Graph selection
    renderGraphList();

    ImGui::Separator();

    if (currentGraph) {
        // Node operations
        renderNodeList();

        ImGui::Separator();

        // Edge operations
        renderEdgeList();
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    // Right column - Canvas
    ImGui::BeginChild("CanvasPanel", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
    renderGraphCanvas();
    ImGui::EndChild();

    ImGui::Columns(1);
}

void GraphEditor::renderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                loadFile("WorkingGraphs.json"); // In a real app, this would use a file dialog
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                saveFile("WorkingGraphs.json"); // In a real app, this would use a file dialog
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                exit(0); // In a real app, you would handle this more gracefully
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graph")) {
            if (ImGui::MenuItem("Auto Layout")) {
                layoutGraph();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void GraphEditor::renderGraphList() {
    ImGui::Text("Graphs");

    auto graphNames = model->getGraphNames();

    if (ImGui::BeginListBox("##GraphList", ImVec2(-1, 100))) {
        for (const auto& name : graphNames) {
            bool isSelected = (name == currentGraphName);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                currentGraphName = name;
                currentGraph = model->getGraph(name);
                clearSelections();
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

    // New graph input
    static char newGraphName[64] = "";
    ImGui::InputText("New Graph", newGraphName, 64);

    ImGui::SameLine();
    if (ImGui::Button("Add") && strlen(newGraphName) > 0) {
        model->createGraph(newGraphName);
        currentGraphName = newGraphName;
        currentGraph = model->getGraph(currentGraphName);
        newGraphName[0] = '\0';
    }

    // Remove graph button
    if (ImGui::Button("Remove Graph") && !currentGraphName.empty()) {
        model->removeGraph(currentGraphName);

        // Select another graph if available
        graphNames = model->getGraphNames();
        if (!graphNames.empty()) {
            currentGraphName = graphNames[0];
            currentGraph = model->getGraph(currentGraphName);
        }
        else {
            currentGraphName = "";
            currentGraph = nullptr;
        }
    }
}

void GraphEditor::renderNodeList() {
    ImGui::Text("Nodes");

    if (ImGui::BeginListBox("##NodeList", ImVec2(-1, 150))) {
        for (const auto& node : currentGraph->nodes) {
            bool isSelected = (node->id == selectedNodeId);
            if (ImGui::Selectable(node->id.c_str(), isSelected)) {
                selectNode(node->id);
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

    // Node operations
    char nodeIdBuffer[64] = "";
    ImGui::InputText("New Node ID", nodeIdBuffer, sizeof(nodeIdBuffer));

    ImGui::SameLine();
    if (ImGui::Button("Add Node") && nodeIdBuffer[0] != '\0') {
        newNodeId = nodeIdBuffer;
        addNode();
    }

    if (ImGui::Button("Remove Selected Node")) {
        removeSelectedNode();
    }
}

void GraphEditor::renderEdgeList() {
    ImGui::Text("Edges");

    if (ImGui::BeginListBox("##EdgeList", ImVec2(-1, 150))) {
        for (const auto& edge : currentGraph->edges) {
            std::string label = edge->from + " -> " + edge->to + " (" + std::to_string(edge->weight) + ")";
            bool isSelected = (selectedEdge && *edge == *selectedEdge);

            if (ImGui::Selectable(label.c_str(), isSelected)) {
                selectEdge(edge->from, edge->to);
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

    // Edge operations
    // Using BeginCombo is correct - keep this as is
    if (ImGui::BeginCombo("From Node", newEdgeFrom.c_str())) {
        for (const auto& node : currentGraph->nodes) {
            bool isSelected = (node->id == newEdgeFrom);
            if (ImGui::Selectable(node->id.c_str(), isSelected)) {
                newEdgeFrom = node->id;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("To Node", newEdgeTo.c_str())) {
        for (const auto& node : currentGraph->nodes) {
            bool isSelected = (node->id == newEdgeTo);
            if (ImGui::Selectable(node->id.c_str(), isSelected)) {
                newEdgeTo = node->id;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SliderFloat("Weight", &newEdgeWeight, 0.1f, 10.0f);

    if (ImGui::Button("Add Edge") && !newEdgeFrom.empty() && !newEdgeTo.empty()) {
        addEdge();
    }

    ImGui::SameLine();
    if (ImGui::Button("Remove Selected Edge")) {
        removeSelectedEdge();
    }
}

void GraphEditor::renderGraphCanvas() {
    if (!currentGraph) {
        ImGui::Text("No graph selected");
        return;
    }

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw canvas background
    drawList->AddRectFilled(canvasPos,
        ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
        CANVAS_BG_COLOR);

    // Handle canvas controls (panning, zooming)
    ImGui::InvisibleButton("canvas", canvasSize);

    // Panning
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        canvasOffset.x += ImGui::GetIO().MouseDelta.x;
        canvasOffset.y += ImGui::GetIO().MouseDelta.y;
    }

    // Zooming
    if (ImGui::IsItemHovered()) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0) {
            canvasScale += wheel * 0.1f;
            canvasScale = std::max(0.1f, canvasScale);
        }
    }

    // Draw edges
    for (const auto& edge : currentGraph->edges) {
        auto fromNode = currentGraph->findNode(edge->from);
        auto toNode = currentGraph->findNode(edge->to);

        if (fromNode && toNode) {
            drawEdge(drawList, edge, fromNode, toNode, canvasPos);
        }
    }

    // Draw nodes
    for (const auto& node : currentGraph->nodes) {
        drawNode(drawList, node, canvasPos);
    }

    // Check for node selection
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        bool nodeSelected = false;

        // Check if a node was clicked
        for (const auto& node : currentGraph->nodes) {
            ImVec2 nodePos = ImVec2(
                canvasPos.x + node->x * canvasScale + canvasOffset.x,
                canvasPos.y + node->y * canvasScale + canvasOffset.y
            );

            float distSq = (mousePos.x - nodePos.x) * (mousePos.x - nodePos.x) +
                (mousePos.y - nodePos.y) * (mousePos.y - nodePos.y);

            if (distSq <= NODE_RADIUS * NODE_RADIUS * canvasScale * canvasScale) {
                selectNode(node->id);
                nodeSelected = true;
                break;
            }
        }

        if (!nodeSelected) {
            clearSelections();
        }
    }

    // Node dragging
    if (!selectedNodeId.empty() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        auto node = currentGraph->findNode(selectedNodeId);
        if (node) {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            node->x += delta.x / canvasScale;
            node->y += delta.y / canvasScale;
        }
    }
}

void GraphEditor::drawNode(ImDrawList* drawList, const std::shared_ptr<Node>& node, const ImVec2& canvasPos) {
    ImVec2 nodePos = ImVec2(
        canvasPos.x + node->x * canvasScale + canvasOffset.x,
        canvasPos.y + node->y * canvasScale + canvasOffset.y
    );

    ImU32 color = (node->id == selectedNodeId) ? NODE_SELECTED_COLOR : NODE_COLOR;

    drawList->AddCircleFilled(nodePos, NODE_RADIUS * canvasScale, color);
    drawList->AddCircle(nodePos, NODE_RADIUS * canvasScale, IM_COL32(255, 255, 255, 100), 0, 2.0f);

    // Center the text
    ImVec2 textSize = ImGui::CalcTextSize(node->id.c_str());
    ImVec2 textPos = ImVec2(
        nodePos.x - textSize.x * 0.5f,
        nodePos.y - textSize.y * 0.5f
    );

    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), node->id.c_str());
}

void GraphEditor::drawEdge(ImDrawList* drawList, const std::shared_ptr<Edge>& edge,
    const std::shared_ptr<Node>& fromNode,
    const std::shared_ptr<Node>& toNode,
    const ImVec2& canvasPos) {
    ImVec2 fromPos = ImVec2(
        canvasPos.x + fromNode->x * canvasScale + canvasOffset.x,
        canvasPos.y + fromNode->y * canvasScale + canvasOffset.y
    );

    ImVec2 toPos = ImVec2(
        canvasPos.x + toNode->x * canvasScale + canvasOffset.x,
        canvasPos.y + toNode->y * canvasScale + canvasOffset.y
    );

    ImU32 color = (selectedEdge && *edge == *selectedEdge) ? EDGE_SELECTED_COLOR : EDGE_COLOR;

    // Adjust start and end points to be on the node boundaries
    float angle = atan2(toPos.y - fromPos.y, toPos.x - fromPos.x);
    ImVec2 fromAdjusted = ImVec2(
        fromPos.x + cos(angle) * NODE_RADIUS * canvasScale,
        fromPos.y + sin(angle) * NODE_RADIUS * canvasScale
    );

    ImVec2 toAdjusted = ImVec2(
        toPos.x - cos(angle) * NODE_RADIUS * canvasScale,
        toPos.y - sin(angle) * NODE_RADIUS * canvasScale
    );

    // Check if there's a bidirectional edge
    bool isBidirectional = false;
    for (const auto& otherEdge : currentGraph->edges) {
        if (otherEdge->from == edge->to && otherEdge->to == edge->from) {
            isBidirectional = true;
            break;
        }
    }

    // Draw the arrow differently if it's bidirectional
    if (isBidirectional) {
        // Calculate offset for curved lines
        float dx = toPos.x - fromPos.x;
        float dy = toPos.y - fromPos.y;
        float dist = sqrt(dx * dx + dy * dy);

        // Normal vector
        float nx = -dy / dist;
        float ny = dx / dist;

        // Control point offset
        float offset = std::min(dist * 0.2f, 50.0f * canvasScale);

        ImVec2 control = ImVec2(
            (fromPos.x + toPos.x) * 0.5f + nx * offset,
            (fromPos.y + toPos.y) * 0.5f + ny * offset
        );

        // Draw the curved arrow
        drawList->AddBezierCubic(
            fromAdjusted,
            ImVec2(fromAdjusted.x + (control.x - fromAdjusted.x) * 0.5f,
                fromAdjusted.y + (control.y - fromAdjusted.y) * 0.5f),
            ImVec2(toAdjusted.x + (control.x - toAdjusted.x) * 0.5f,
                toAdjusted.y + (control.y - toAdjusted.y) * 0.5f),
            toAdjusted,
            color,
            EDGE_THICKNESS * canvasScale
        );

        // Calculate the angle at the end of the curve for the arrow
        ImVec2 tangent = ImVec2(
            toAdjusted.x - (control.x + toAdjusted.x) * 0.5f,
            toAdjusted.y - (control.y + toAdjusted.y) * 0.5f
        );

        float arrowAngle = atan2(tangent.y, tangent.x);

        // Draw the arrow head
        ImVec2 arrowP1 = ImVec2(
            toAdjusted.x - ARROW_SIZE * canvasScale * cos(arrowAngle - 0.5f),
            toAdjusted.y - ARROW_SIZE * canvasScale * sin(arrowAngle - 0.5f)
        );

        ImVec2 arrowP2 = ImVec2(
            toAdjusted.x - ARROW_SIZE * canvasScale * cos(arrowAngle + 0.5f),
            toAdjusted.y - ARROW_SIZE * canvasScale * sin(arrowAngle + 0.5f)
        );

        drawList->AddTriangleFilled(toAdjusted, arrowP1, arrowP2, color);
    }
    else {
        // Draw a straight arrow
        drawList->AddLine(fromAdjusted, toAdjusted, color, EDGE_THICKNESS * canvasScale);

        // Draw the arrow head
        drawDirectedArrow(drawList, fromAdjusted, toAdjusted, color, EDGE_THICKNESS * canvasScale, ARROW_SIZE * canvasScale);
    }

    // Draw the weight
    std::string weightText = std::to_string(edge->weight);
    ImVec2 midpoint = ImVec2(
        (fromAdjusted.x + toAdjusted.x) * 0.5f,
        (fromAdjusted.y + toAdjusted.y) * 0.5f
    );

    ImVec2 textSize = ImGui::CalcTextSize(weightText.c_str());
    drawList->AddRectFilled(
        ImVec2(midpoint.x - textSize.x * 0.5f - 2, midpoint.y - textSize.y * 0.5f - 2),
        ImVec2(midpoint.x + textSize.x * 0.5f + 2, midpoint.y + textSize.y * 0.5f + 2),
        IM_COL32(30, 30, 30, 200)
    );

    drawList->AddText(
        ImVec2(midpoint.x - textSize.x * 0.5f, midpoint.y - textSize.y * 0.5f),
        IM_COL32(255, 255, 255, 255),
        weightText.c_str()
    );
}

void GraphEditor::drawDirectedArrow(ImDrawList* drawList, const ImVec2& from, const ImVec2& to,
    ImU32 color, float thickness, float arrowSize) {
    float angle = atan2(to.y - from.y, to.x - from.x);

    ImVec2 arrowP1 = ImVec2(
        to.x - arrowSize * cos(angle - 0.5f),
        to.y - arrowSize * sin(angle - 0.5f)
    );

    ImVec2 arrowP2 = ImVec2(
        to.x - arrowSize * cos(angle + 0.5f),
        to.y - arrowSize * sin(angle + 0.5f)
    );

    drawList->AddTriangleFilled(to, arrowP1, arrowP2, color);
}

void GraphEditor::addNode() {
    if (currentGraph && !newNodeId.empty() && !currentGraph->findNode(newNodeId)) {
        currentGraph->addNode(newNodeId);

        // Place the new node at a random position on the canvas
        auto node = currentGraph->findNode(newNodeId);
        if (node) {
            node->x = 100.0f + (rand() % int(canvasWidth - 200.0f));
            node->y = 100.0f + (rand() % int(canvasHeight - 200.0f));
        }

        // Clear the input field
        newNodeId.clear();
    }
}

void GraphEditor::removeSelectedNode() {
    if (currentGraph && !selectedNodeId.empty()) {
        currentGraph->removeNode(selectedNodeId);
        selectedNodeId.clear();
    }
}

void GraphEditor::addEdge() {
    if (currentGraph && !newEdgeFrom.empty() && !newEdgeTo.empty()) {
        currentGraph->addEdge(newEdgeFrom, newEdgeTo, newEdgeWeight);

        // Clear the selection
        newEdgeFrom.clear();
        newEdgeTo.clear();
        newEdgeWeight = 1.0f;
    }
}

void GraphEditor::removeSelectedEdge() {
    if (currentGraph && selectedEdge) {
        currentGraph->removeEdge(selectedEdge->from, selectedEdge->to);
        selectedEdge = nullptr;
    }
}

void GraphEditor::selectNode(const std::string& nodeId) {
    selectedNodeId = nodeId;
    selectedEdge = nullptr;
}

void GraphEditor::selectEdge(const std::string& from, const std::string& to) {
    selectedNodeId.clear();
    selectedEdge = currentGraph->findEdge(from, to);
}

void GraphEditor::clearSelections() {
    selectedNodeId.clear();
    selectedEdge = nullptr;
}

void GraphEditor::layoutGraph() {
    if (!currentGraph || currentGraph->nodes.empty()) {
        return;
    }

    const float SPACING = 150.0f;
    const float RADIUS = std::min(canvasWidth, canvasHeight) * 0.4f;

    int nodeCount = static_cast<int>(currentGraph->nodes.size());

    // Simple circular layout
    if (nodeCount <= 10) {
        for (int i = 0; i < nodeCount; i++) {
            float angle = (2.0f * PI * i) / nodeCount;
            currentGraph->nodes[i]->x = canvasWidth / 2.0f + RADIUS * cos(angle);
            currentGraph->nodes[i]->y = canvasHeight / 2.0f + RADIUS * sin(angle);
        }
    }
    else {
        // Grid layout for many nodes
        int cols = static_cast<int>(sqrt(nodeCount));
        int rows = (nodeCount + cols - 1) / cols;

        float startX = canvasWidth / 2.0f - (cols * SPACING) / 2.0f;
        float startY = canvasHeight / 2.0f - (rows * SPACING) / 2.0f;

        for (int i = 0; i < nodeCount; i++) {
            int row = i / cols;
            int col = i % cols;

            currentGraph->nodes[i]->x = startX + col * SPACING;
            currentGraph->nodes[i]->y = startY + row * SPACING;
        }
    }
}

void GraphEditor::loadFile(const std::string& filename) {
    if (model->loadFromFile(filename)) {
        std::cout << "Successfully loaded graph data from: " << filename << std::endl;

        // Auto-select the first graph if available
        auto graphNames = model->getGraphNames();
        if (!graphNames.empty()) {
            currentGraphName = graphNames[0];
            currentGraph = model->getGraph(currentGraphName);

            // Apply auto-layout
            layoutGraph();
        }
        else {
            currentGraphName = "";
            currentGraph = nullptr;
        }
    }
    else {
        std::cerr << "Failed to load graph data from: " << filename << std::endl;

    }
}

void GraphEditor::saveFile(const std::string& filename) {
    if (model->saveToFile(filename)) {
        std::cout << "Successfully saved graph data to: " << filename << std::endl;
    }
    else {
        std::cerr << "Failed to save graph data to: " << filename << std::endl;
    }
}