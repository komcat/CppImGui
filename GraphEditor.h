#pragma once

#include "GraphModel.h"
#include "imgui.h"
#include <memory>
#include <string>
#include <functional>

class GraphEditor {
public:
    GraphEditor();
    ~GraphEditor() = default;

    void render();
    void setModel(std::shared_ptr<GraphModel> model);

private:
    // Rendering functions
    void renderMainMenu();
    void renderGraphList();
    void renderNodeList();
    void renderEdgeList();
    void renderGraphCanvas();

    // Node and edge operations
    void addNode();
    void removeSelectedNode();
    void addEdge();
    void removeSelectedEdge();

    // Selection handling
    void selectNode(const std::string& nodeId);
    void selectEdge(const std::string& from, const std::string& to);
    void clearSelections();

    // Auto-layout
    void layoutGraph();

    // File operations
    void loadFile(const std::string& filename);
    void saveFile(const std::string& filename);

    // State variables
    std::shared_ptr<GraphModel> model;
    std::string currentGraphName;
    std::shared_ptr<Graph> currentGraph;

    // Node operation state
    std::string newNodeId;

    // Edge operation state
    std::string newEdgeFrom;
    std::string newEdgeTo;
    float newEdgeWeight = 1.0f;

    // UI state
    float canvasWidth = 800.0f;
    float canvasHeight = 600.0f;
    ImVec2 canvasOffset = ImVec2(0.0f, 0.0f);
    float canvasScale = 1.0f;
    bool isDragging = false;
    std::string selectedNodeId;
    std::shared_ptr<Edge> selectedEdge;

    // Drawing helpers
    void drawNode(ImDrawList* drawList, const std::shared_ptr<Node>& node, const ImVec2& canvasPos);
    void drawEdge(ImDrawList* drawList, const std::shared_ptr<Edge>& edge,
        const std::shared_ptr<Node>& fromNode,
        const std::shared_ptr<Node>& toNode,
        const ImVec2& canvasPos);
    void drawDirectedArrow(ImDrawList* drawList, const ImVec2& from, const ImVec2& to,
        ImU32 color, float thickness, float arrowSize);
};