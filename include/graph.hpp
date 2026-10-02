#pragma once
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <queue>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace routes {
struct Edge { std::string to; double weight; };
struct Result { bool reachable; double distance; std::vector<std::string> path; };
class Graph {
    std::unordered_map<std::string, std::vector<Edge>> adjacency;
public:
    void add_node(const std::string& id) {
        if (!std::regex_match(id, std::regex("[A-Za-z0-9_-]+")))
            throw std::invalid_argument("Node IDs must contain letters, digits, _ or -.");
        if (!adjacency.emplace(id, std::vector<Edge>{}).second)
            throw std::invalid_argument("Duplicate node: " + id);
    }
    void add_edge(const std::string& from, const std::string& to, double weight) {
        if (!adjacency.count(from) || !adjacency.count(to))
            throw std::invalid_argument("Declare both nodes before adding an edge.");
        if (!std::isfinite(weight) || weight < 0)
            throw std::invalid_argument("Weights must be finite and non-negative.");
        adjacency.at(from).push_back({to, weight});
    }
    Result shortest_path(const std::string& start, const std::string& goal, bool weighted) const {
        if (!adjacency.count(start) || !adjacency.count(goal))
            throw std::invalid_argument("Unknown origin or destination.");
        const double inf = std::numeric_limits<double>::infinity();
        std::unordered_map<std::string, double> distance;
        std::unordered_map<std::string, std::string> parent;
        for (const auto& item : adjacency) distance[item.first] = inf;
        distance[start] = 0;
        if (weighted) {
            using State = std::pair<double, std::string>;
            std::priority_queue<State, std::vector<State>, std::greater<State>> pending;
            pending.push({0, start});
            while (!pending.empty()) {
                auto [cost, node] = pending.top(); pending.pop();
                if (cost != distance.at(node)) continue;
                if (node == goal) break;
                for (const auto& edge : adjacency.at(node)) {
                    const double next = cost + edge.weight;
                    if (!std::isfinite(next)) throw std::overflow_error("Path cost overflow.");
                    if (next < distance.at(edge.to)) {
                        distance[edge.to] = next;
                        parent[edge.to] = node;
                        pending.push({next, edge.to});
                    }
                }
            }
        } else {
            std::queue<std::string> pending; pending.push(start);
            while (!pending.empty()) {
                const auto node = pending.front(); pending.pop();
                if (node == goal) break;
                for (const auto& edge : adjacency.at(node)) {
                    if (distance.at(edge.to) == inf) {
                        distance[edge.to] = distance.at(node) + 1;
                        parent[edge.to] = node;
                        pending.push(edge.to);
                    }
                }
            }
        }
        if (distance.at(goal) == inf) return {false, 0, {}};
        std::vector<std::string> path;
        for (auto node = goal;; node = parent.at(node)) {
            path.push_back(node);
            if (node == start) break;
        }
        std::reverse(path.begin(), path.end());
        return {true, distance.at(goal), path};
    }
};
inline Graph read_graph(std::istream& input) {
    Graph graph; std::string line; std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        line = line.substr(0, line.find('#'));
        std::istringstream row(line);
        std::string kind, a, b, extra; double weight;
        if (!(row >> kind)) continue;
        try {
            if (kind == "node" && (row >> a) && !(row >> extra)) graph.add_node(a);
            else if (kind == "edge" && (row >> a >> b >> weight) && !(row >> extra))
                graph.add_edge(a, b, weight);
            else throw std::invalid_argument("Expected 'node ID' or 'edge FROM TO WEIGHT'.");
        } catch (const std::exception& e) {
            throw std::invalid_argument("Line " + std::to_string(line_number) + ": " + e.what());
        }
    }
    if (input.bad()) throw std::runtime_error("Could not read graph input.");
    return graph;
}
}
