#include "graph.hpp"
#include <iostream>
int main() {
    int checks = 0;
    auto check = [&](bool ok) { if (!ok) throw std::runtime_error("Failed check " + std::to_string(checks + 1)); ++checks; };
    auto rejects = [&](auto action) { bool failed = false; try { action(); } catch (const std::exception&) { failed = true; } check(failed); };
    std::istringstream input("node A\nnode B\nnode C\nnode D\nnode E\nnode Z\nedge A B 10\nedge B D 1\nedge A C 1\nedge C E 1\nedge E D 1\nedge D A 0\n");
    auto g = routes::read_graph(input);
    auto bfs = g.shortest_path("A", "D", false);
    auto dijkstra = g.shortest_path("A", "D", true);
    check(bfs.distance == 2 && bfs.path == std::vector<std::string>({"A", "B", "D"}));
    check(dijkstra.distance == 3 && dijkstra.path == std::vector<std::string>({"A", "C", "E", "D"}));
    check(!g.shortest_path("A", "Z", false).reachable);
    check(!g.shortest_path("Z", "A", true).reachable);
    check(g.shortest_path("A", "A", true).path == std::vector<std::string>({"A"}));
    check(g.shortest_path("D", "A", true).distance == 0);
    rejects([&] { g.shortest_path("unknown", "A", true); });
    rejects([&] { g.add_edge("A", "B", -1); });
    rejects([&] { g.add_edge("A", "B", std::numeric_limits<double>::infinity()); });
    rejects([&] { g.add_node("A"); });
    rejects([&] { g.add_node("bad id"); });
    rejects([&] { g.add_edge("A", "unknown", 1); });
    rejects([&] { std::istringstream bad("node A trailing"); routes::read_graph(bad); });
    rejects([&] { std::istringstream bad("node A\nedge A A NaN"); routes::read_graph(bad); });
    g.add_edge("A", "B", 0.5);
    check(g.shortest_path("A", "D", true).distance == 1.5);
    check(g.shortest_path("A", "A", false).distance == 0);
    std::cout << checks << " graph checks passed\n";
}
