#include "SGL/sgl.hxx"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    /*
              F
            /   \
            B     G
          /   \     \
          A     D     I
              /   \     \
              C     E     H
    */

    sgl::version();

    sgl::Graph<std::string, sgl::AdjacencyList> graph1;

    auto f = graph1.add_vertex("F");
    auto b = graph1.add_vertex("B");
    auto a = graph1.add_vertex("A");
    auto d = graph1.add_vertex("D");
    auto c = graph1.add_vertex("C");
    auto e = graph1.add_vertex("E");
    auto g = graph1.add_vertex("G");
    auto i = graph1.add_vertex("I");
    auto h = graph1.add_vertex("H");

    graph1.add_edge(f, b);
    graph1.add_edge(b, a);
    graph1.add_edge(b, d);
    graph1.add_edge(d, c);
    graph1.add_edge(d, e);
    graph1.add_edge(f, g);
    graph1.add_edge(g, i);
    graph1.add_edge(i, h);

    sgl::Graph<std::string, sgl::AdjacencyMatrix> graph2;

    for (auto &v : graph1)
        graph2.add_vertex(v);

    // vertex iterator
    for (auto &vertex : graph1)
    // neighbor iterator
    {
        for (auto &neighbor : vertex)
        {
            std::cout << vertex.get_data() << " -> " << neighbor.get_data() << std::endl;
            break;
        }
        for (auto it = graph1.begin(vertex.get_id()); it != graph1.end(vertex.get_id()); ++it)
        {
            std::cout << it->get_data() << " <- " << vertex.get_data() << std::endl;
            break;
        }
    }

    std::cout << std::endl
              << "graph1: " << std::endl
              << sgl::VertexFormat::LONG << graph1 << std::endl;
    std::cout << "graph2: " << std::endl
              << sgl::VertexFormat::LONG << graph2 << std::endl;

    std::cout << "graph1 BFS from f: " << std::endl;
    graph1.traverse(f);
    std::cout << std::endl;

    std::cout << "graph2 BFS: " << std::endl;
    graph2.traverse();
    std::cout << std::endl;

    std::cout << "graph1 DFS from d: " << std::endl;
    graph1.traverse<sgl::DFS>(d);
    std::cout << std::endl;

    std::cout << "graph2 DFS with lambda: " << std::endl;
    graph2.traverse<sgl::DFS>([](auto &v)
                              { std::cout << "num of adjacent: " << v.size() << std::endl; });
    std::cout << std::endl;

    std::cout << "graph2 DFS with predefined lambda: " << std::endl;
    graph2.traverse<sgl::DFS>(sgl::func::add, " X");
    std::cout << sgl::VertexFormat::SHORT << graph2 << std::endl;

    std::cout << "graph1 DFS removed d: " << std::endl;
    graph1.remove_vertex(d);
    graph1.traverse<sgl::DFS>();
    std::cout << std::endl;

    std::cout << "graph1 DFS visit all policy: " << std::endl;
    graph1.traverse<sgl::DFS, sgl::VisitPolicy::ALL>();
    std::cout << std::endl;

    std::cout << "graph1 BFS removed a-b edge: " << std::endl;
    graph1.remove_edge(a, b);
    graph1.traverse<sgl::BFS>(b);
    std::cout << std::endl;

    std::cout << "graph1 BFS remove if with predefined lambda: " << std::endl;
    graph1.remove_if(sgl::func::greater_than, "G");
    graph1.traverse<sgl::BFS>(b);
    std::cout << std::endl;

    std::cout << "graph1 BFS remove if with lambda: " << std::endl;
    graph1.remove_if([](auto &v)
                     { return v.get_data() == "G"; });
    graph1.traverse<sgl::BFS>(b);
    std::cout << std::endl;

    std::cout << "graph1 DFS print with long format: " << std::endl;
    graph1.traverse<sgl::DFS>(sgl::func::print, sgl::VertexFormat::LONG, std::cout);
    std::cout << std::endl;

    graph2.print();
    std::cout << std::endl;

    sgl::Graph<std::string, sgl::AdjacencyMatrix> graph3;

    f = graph3.add_vertex("F");
    b = graph3.add_vertex("B");
    a = graph3.add_vertex("A");
    d = graph3.add_vertex("D");
    c = graph3.add_vertex("C");
    e = graph3.add_vertex("E");
    g = graph3.add_vertex("G");
    i = graph3.add_vertex("I");
    h = graph3.add_vertex("H");

    graph3.add_edge(f, b, 0.5f);
    graph3.add_edge(b, a, 1.4f);
    graph3.add_edge(b, d, 2.2f);
    graph3.add_edge(d, c, 3.1f);
    graph3.add_edge(d, e, 4.3f);
    graph3.add_edge(f, g, 5.2f);
    graph3.add_edge(g, i, 6.1f);
    graph3.add_edge(i, h, 7.3f);

    std::cout << "graph3: " << std::endl
              << sgl::VertexFormat::SHORT << graph3 << std::endl;

    graph3.remove_edge(d, b);

    std::cout << "graph3: " << std::endl
              << sgl::VertexFormat::SHORT << graph3 << std::endl;

    std::cout << graph3.size() << std::endl;

    return EXIT_SUCCESS;
}
