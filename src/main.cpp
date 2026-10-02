#include "graph.hpp"
#include <iomanip>
#include <iostream>
int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: route_lab GRAPH ORIGIN DESTINATION bfs|dijkstra\n"; return 0;
    }
    if (argc != 5) {
        std::cerr << "Usage: route_lab GRAPH ORIGIN DESTINATION bfs|dijkstra\n"; return 2;
    }
    try {
        const std::string algorithm = argv[4];
        if (algorithm != "bfs" && algorithm != "dijkstra")
            throw std::invalid_argument("Algorithm must be bfs or dijkstra.");
        std::ifstream input(argv[1]);
        if (!input) throw std::runtime_error("Cannot open graph file.");
        const auto graph = routes::read_graph(input);
        const auto result = graph.shortest_path(argv[2], argv[3], algorithm == "dijkstra");
        std::cout << std::setprecision(17) << "{\"algorithm\":\"" << algorithm
                  << "\",\"objective\":\"" << (algorithm == "bfs" ? "hops" : "weight")
                  << "\",\"reachable\":" << (result.reachable ? "true" : "false") << ",\"distance\":";
        if (result.reachable) std::cout << result.distance; else std::cout << "null";
        std::cout << ",\"path\":[";
        for (std::size_t i = 0; i < result.path.size(); ++i)
            std::cout << (i ? "," : "") << '"' << result.path[i] << '"';
        std::cout << "]}\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 2; }
}
