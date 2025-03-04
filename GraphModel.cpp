#include "GraphModel.h"
#include <fstream>
#include <iostream>

// Modify the loadFromFile method in GraphModel.cpp to load node positions:

bool GraphModel::loadFromFile(const std::string& filename) {
    try {
        // Clear existing data
        graphs.clear();

        // Open JSON file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        // Parse JSON
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();

        // Check if the JSON has the expected structure
        if (!jsonData.contains("graphs") || !jsonData["graphs"].is_object()) {
            std::cerr << "Invalid JSON structure: 'graphs' object not found" << std::endl;
            return false;
        }

        // Process each graph
        for (auto it = jsonData["graphs"].begin(); it != jsonData["graphs"].end(); ++it) {
            std::string graphName = it.key();
            auto& graphData = it.value();

            createGraph(graphName);
            auto graph = getGraph(graphName);

            // Add nodes
            if (graphData.contains("nodes")) {
                // Check if nodes is an array of objects or just an array of strings
                if (graphData["nodes"].is_array()) {
                    if (graphData["nodes"].empty()) {
                        // Empty array, nothing to do
                    }
                    else if (graphData["nodes"][0].is_string()) {
                        // Old format: array of strings
                        for (const auto& nodeId : graphData["nodes"]) {
                            if (nodeId.is_string()) {
                                graph->addNode(nodeId);
                            }
                        }
                    }
                    else if (graphData["nodes"][0].is_object()) {
                        // New format: array of objects with position information
                        for (const auto& nodeData : graphData["nodes"]) {
                            if (nodeData.contains("id") && nodeData["id"].is_string()) {
                                std::string nodeId = nodeData["id"];
                                graph->addNode(nodeId);

                                auto node = graph->findNode(nodeId);
                                if (node) {
                                    // Load position if available
                                    if (nodeData.contains("x") && nodeData["x"].is_number()) {
                                        node->x = nodeData["x"];
                                    }
                                    if (nodeData.contains("y") && nodeData["y"].is_number()) {
                                        node->y = nodeData["y"];
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Add edges
            if (graphData.contains("edges") && graphData["edges"].is_array()) {
                for (const auto& edgeData : graphData["edges"]) {
                    if (edgeData.contains("from") && edgeData.contains("to") &&
                        edgeData["from"].is_string() && edgeData["to"].is_string()) {

                        float weight = 1.0f;
                        if (edgeData.contains("weight") && edgeData["weight"].is_number()) {
                            weight = edgeData["weight"];
                        }

                        graph->addEdge(edgeData["from"], edgeData["to"], weight);
                    }
                }
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading graph data: " << e.what() << std::endl;
        return false;
    }
}


// Modify the saveToFile method in GraphModel.cpp to include node positions:

bool GraphModel::saveToFile(const std::string& filename) {
    try {
        nlohmann::json jsonData;
        jsonData["graphs"] = nlohmann::json::object();

        // Convert graphs to JSON
        for (auto it = graphs.begin(); it != graphs.end(); ++it) {
            const std::string& graphName = it->first;
            const auto& graph = it->second;

            nlohmann::json graphJson;

            // Add nodes with position information
            graphJson["nodes"] = nlohmann::json::array();
            for (const auto& node : graph->nodes) {
                nlohmann::json nodeJson;
                nodeJson["id"] = node->id;
                nodeJson["x"] = node->x;
                nodeJson["y"] = node->y;
                graphJson["nodes"].push_back(nodeJson);
            }

            // Add edges
            graphJson["edges"] = nlohmann::json::array();
            for (const auto& edge : graph->edges) {
                nlohmann::json edgeJson;
                edgeJson["from"] = edge->from;
                edgeJson["to"] = edge->to;
                edgeJson["weight"] = edge->weight;
                graphJson["edges"].push_back(edgeJson);
            }

            jsonData["graphs"][graphName] = graphJson;
        }

        // Write to file
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        file << jsonData.dump(2); // Pretty print with 2 spaces
        file.close();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving graph data: " << e.what() << std::endl;
        return false;
    }
}