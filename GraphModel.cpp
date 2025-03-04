#include "GraphModel.h"
#include <fstream>
#include <iostream>

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
            if (graphData.contains("nodes") && graphData["nodes"].is_array()) {
                for (const auto& nodeId : graphData["nodes"]) {
                    if (nodeId.is_string()) {
                        graph->addNode(nodeId);
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

bool GraphModel::saveToFile(const std::string& filename) {
    try {
        nlohmann::json jsonData;
        jsonData["graphs"] = nlohmann::json::object();

        // Convert graphs to JSON
        for (auto it = graphs.begin(); it != graphs.end(); ++it) {
            const std::string& graphName = it->first;
            const auto& graph = it->second;

            nlohmann::json graphJson;

            // Add nodes
            graphJson["nodes"] = nlohmann::json::array();
            for (const auto& node : graph->nodes) {
                graphJson["nodes"].push_back(node->id);
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