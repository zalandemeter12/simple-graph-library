#include "SGL/sgl.hxx"

#include <string>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    sgl::version();

    sgl::Graph<std::string, sgl::AdjacencyList> graph;

    [[maybe_unused]] auto f = graph.add_vertex("F");
    [[maybe_unused]] auto b = graph.add_vertex("B");
    [[maybe_unused]] auto a = graph.add_vertex("A");
    [[maybe_unused]] auto d = graph.add_vertex("D");
    [[maybe_unused]] auto c = graph.add_vertex("C");
    [[maybe_unused]] auto e = graph.add_vertex("E");
    [[maybe_unused]] auto g = graph.add_vertex("G");
    [[maybe_unused]] auto i = graph.add_vertex("I");
    [[maybe_unused]] auto h = graph.add_vertex("H");

    graph.add_edge(f, b);
    graph.add_edge(b, a);
    graph.add_edge(b, d);
    graph.add_edge(d, c);
    graph.add_edge(d, e);
    graph.add_edge(f, g);
    graph.add_edge(g, i);
    graph.add_edge(i, h);

    std::cout << "Breadth First Search: " << std::endl;
    graph.traverse<sgl::BFS>(sgl::add<std::string>, " node");

    std::cout << std::endl << "Depth First Search: " << std::endl;
    graph.traverse<sgl::DFS>();

    std::cout << std::endl << "Removed vertex, DFS: " << std::endl;
    graph.remove_vertex(g);
    graph.traverse<sgl::DFS,true>();

    std::cout << std::endl << "Removed edge, DFS: " << std::endl;
    g = graph.add_vertex("G node");
    graph.add_edge(f, g);
    graph.add_edge(g, i);
    graph.remove_edge(f, b);
    graph.traverse<sgl::DFS>();

    for(auto it = graph.begin(); it != graph.end(); ++it)
    {
        if(it->get_data() == "A node")
        {
            std::cout << "Found A node" << std::endl;
            it->get_data() = "Changed";
            std::cout << *it << std::endl;
        }
    }

    return EXIT_SUCCESS;
}