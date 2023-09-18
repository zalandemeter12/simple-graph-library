#include "sgl.hxx"

#include <chrono>
#include <numbers>

#define PI std::numbers::pi_v<float>

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

    std::cout << "graph1: " << std::endl
              << sgl::VertexFormat::LONG << graph1 << std::endl;

    for (auto &v : graph1)
        graph2.add_vertex(v);

    // vertex iterator
    for (auto &vertex : graph1)
    // neighbor iterator
    {
        for (auto &neighbor : vertex)
        {
            std::cout << vertex.data() << " -> " << neighbor.data() << std::endl;
            break;
        }
        for (auto it = graph1.begin(vertex.get_id()); it != graph1.end(vertex.get_id()); ++it)
        {
            std::cout << it->data() << " <- " << vertex.data() << std::endl;
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
                     { return v.data() == "G"; });
    graph1.traverse<sgl::BFS>(b);
    std::cout << std::endl;

    std::cout << "graph1 DFS print with long format: " << std::endl;
    graph1.traverse<sgl::DFS>(sgl::func::print, sgl::VertexFormat::LONG, std::cout);
    std::cout << std::endl;

    graph2.print();
    std::cout << std::endl;

    //////////////////////////////////////////////////////////////////////////////
    //
    //       RUNTIME TEST
    //

    [[maybe_unused]] int vertices_count = 100;
    [[maybe_unused]] int edges_count = 500;

    sgl::Graph<std::string, sgl::AdjacencyList> graph_dijkstra2;

    std::vector<sgl::uuid> vertices;

    for (int i = 0; i < vertices_count; ++i)
        vertices.push_back(graph_dijkstra2.add_vertex(std::to_string(i)));

    int created_edges = 0;
    do
    {
        int v1 = rand() % vertices_count;
        int v2 = rand() % vertices_count;
        int weight = rand() % 100;
        try
        {
            graph_dijkstra2.add_edge(vertices[v1], vertices[v2], weight);
            ++created_edges;
        }
        catch (const std::exception &e)
        {
            continue;
        }
    } while (created_edges < edges_count);

    auto start1 = std::chrono::high_resolution_clock::now();
    auto map = dijkstra(graph_dijkstra2, vertices[0]);
    auto end1 = std::chrono::high_resolution_clock::now();

    std::cout << "time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << "ms" << std::endl;
    std::cout << "shortest distance to the 50th vertex: " << map[vertices[50]].first << std::endl;

    //
    //       END OF RUNTIME TEST
    //
    //////////////////////////////////////////////////////////////////////////////

    return EXIT_SUCCESS;
}
