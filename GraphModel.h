#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

// Forward declarations
struct Node;
struct Edge;
struct Graph;

// Data structure for a node
struct Node {
    std::string id;
    float x = 0.0f;
    float y = 0.0f;
    bool selected = false;

    Node(const std::string& nodeId) : id(nodeId) {}
};

// Data structure for an edge
struct Edge {
    std::string from;
    std::string to;
    float weight = 1.0f;
    bool selected = false;

    Edge(const std::string& fromNode, const std::string& toNode, float edgeWeight = 1.0f)
        : from(fromNode), to(toNode), weight(edgeWeight) {
    }

    bool operator==(const Edge& other) const {
        return from == other.from && to == other.to;
    }
};

// Data structure for a graph
struct Graph {
    std::string name;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Edge>> edges;

    Graph(const std::string& graphName) : name(graphName) {}

    std::shared_ptr<Node> findNode(const std::string& id) {
        for (auto& node : nodes) {
            if (node->id == id) {
                return node;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Edge> findEdge(const std::string& from, const std::string& to) {
        for (auto& edge : edges) {
            if (edge->from == from && edge->to == to) {
                return edge;
            }
        }
        return nullptr;
    }

    void addNode(const std::string& id) {
        if (!findNode(id)) {
            nodes.push_back(std::make_shared<Node>(id));
        }
    }

    void removeNode(const std::string& id) {
        // First remove all edges associated with this node
        auto edgeIt = edges.begin();
        while (edgeIt != edges.end()) {
            if ((*edgeIt)->from == id || (*edgeIt)->to == id) {
                edgeIt = edges.erase(edgeIt);
            }
            else {
                ++edgeIt;
            }
        }

        // Then remove the node
        auto nodeIt = std::find_if(nodes.begin(), nodes.end(),
            [&id](const std::shared_ptr<Node>& n) { return n->id == id; });
        if (nodeIt != nodes.end()) {
            nodes.erase(nodeIt);
        }
    }

    void addEdge(const std::string& from, const std::string& to, float weight = 1.0f) {
        // Make sure both nodes exist
        if (!findNode(from) || !findNode(to)) {
            return;
        }

        // Check if edge already exists
        if (!findEdge(from, to)) {
            edges.push_back(std::make_shared<Edge>(from, to, weight));
        }
    }

    void removeEdge(const std::string& from, const std::string& to) {
        auto it = std::find_if(edges.begin(), edges.end(),
            [&from, &to](const std::shared_ptr<Edge>& e) {
            return e->from == from && e->to == to;
        });
        if (it != edges.end()) {
            edges.erase(it);
        }
    }



};

// Class to manage all graph data
class GraphModel {
public:
    GraphModel() = default;
    ~GraphModel() = default;

    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename);

    std::shared_ptr<Graph> getGraph(const std::string& name) {
        if (graphs.find(name) != graphs.end()) {
            return graphs[name];
        }
        return nullptr;
    }

    std::vector<std::string> getGraphNames() const {
        std::vector<std::string> names;
        for (const auto& pair : graphs) {
            names.push_back(pair.first);
        }
        return names;
    }

    void createGraph(const std::string& name) {
        if (graphs.find(name) == graphs.end()) {
            graphs[name] = std::make_shared<Graph>(name);
        }
    }

    void removeGraph(const std::string& name) {
        graphs.erase(name);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Graph>> graphs;
};