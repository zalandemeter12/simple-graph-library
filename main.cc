#include "SGL/sgl.hxx"

#include <string>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    sgl::version();

    sgl::Graph<std::string, sgl::AdjacencyList> graph;

    // [[maybe_unused]] auto id1 = graph.add_vertex(251);
    // [[maybe_unused]] auto id2 = graph.add_vertex(50);
    // [[maybe_unused]] auto id3 = graph.add_vertex(100);
    // [[maybe_unused]] auto id4 = graph.add_vertex(25);
    // [[maybe_unused]] auto id5 = graph.add_vertex(1152);
    // [[maybe_unused]] auto id6 = graph.add_vertex(1);
    // [[maybe_unused]] auto id7 = graph.add_vertex(2);

    [[maybe_unused]] auto id1 = graph.add_vertex("251");
    [[maybe_unused]] auto id2 = graph.add_vertex("50");
    [[maybe_unused]] auto id3 = graph.add_vertex("100");
    [[maybe_unused]] auto id4 = graph.add_vertex("25");
    [[maybe_unused]] auto id5 = graph.add_vertex("1152");
    [[maybe_unused]] auto id6 = graph.add_vertex("1");
    [[maybe_unused]] auto id7 = graph.add_vertex("2");

    sgl::AdjacencyList<std::string> adj_list;
    adj_list.add_vertex("251");
    adj_list.add_vertex("50");

    sgl::Vertex<std::string> vertex{"251"};

    std::cout << sgl::VertexFormat::LONG << graph << std::endl;
    std::cout << sgl::VertexFormat::LONG << adj_list << std::endl;
    std::cout << sgl::VertexFormat::LONG << vertex << std::endl;

    graph.print();

    adj_list.traverse<sgl::BFS, true>(sgl::add<std::string>, " added");
    graph.traverse<sgl::DFS, true>(sgl::add<std::string>, "5");

    std::cout << sgl::VertexFormat::LONG << graph << std::endl;

    return EXIT_SUCCESS;
}