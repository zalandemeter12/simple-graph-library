#ifndef SGL_HH
#define SGL_HH

#include <iostream>
#include <memory>
#include <functional>

#include <vector>
#include <set>
#include <queue>
#include <map>
#include <stack>

#include <boost/uuid/uuid.hpp>              // uuid class
#include <boost/uuid/uuid_generators.hpp>   // generators
#include <boost/uuid/uuid_io.hpp>           // streaming operators etc.
//UniformRandomNumberGenerator
#include <boost/random/mersenne_twister.hpp>

namespace sgl 
{

    void version()
    {
        std::cout << R"(
│ ╔═╗╔═╗╦      Simple Graph Library │ 
│ ╚═╗║ ╦║             version 0.1.0 │
│ ╚═╝╚═╝╩═╝        by Zalan Demeter │                                                                  
        )" << std::endl;                                                                             
    }

    template <typename DATA_TYPE>
    class Vertex;

    class VertexPrinter;

    template <typename DATA_TYPE>
    class AdjacencyList;

    template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE>
    class Graph;

    template <typename DATA_STRUCTURE> 
    class BFS;

    template <typename DATA_STRUCTURE> 
    class DFS;

    template <typename DATA_TYPE>
    class DataStructureBase;

    template <typename DATA_TYPE>
    class Vertex
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        public:
            Vertex(const DATA_TYPE&& data) : m_id{s_generator()}, m_data{data} {}

            ~Vertex() = default;

            const boost::uuids::uuid& get_id() const { return m_id; }
            const DATA_TYPE& get_data() const { return m_data; }
            DATA_TYPE& get_data() { return m_data; }

            friend std::ostream& operator<<(std::ostream& os, const VERTEX_TYPE& vertex)
            {
                std::string id = boost::uuids::to_string(vertex.m_id);
                return os << "{" << id.substr(0, 2) << "..." << id.substr(id.size() - 2, 2) << "}: " << vertex.m_data;
            }
            
        private:
            boost::uuids::uuid m_id;
            DATA_TYPE m_data;
            [[maybe_unused]] std::vector<std::shared_ptr<VERTEX_TYPE>> m_adjacent_vertices;

            static inline boost::random::mt19937 rnd = boost::random::mt19937{123456789};
            static inline boost::uuids::basic_random_generator<boost::random::mt19937> s_generator = boost::uuids::basic_random_generator<boost::random::mt19937>(rnd);
    
            friend class VertexPrinter;
            friend class AdjacencyList<DATA_TYPE>;
            friend class Graph<DATA_TYPE, AdjacencyList>;
    };

    enum VertexFormat
    {
        SHORT,
        LONG
    };

    class VertexPrinter
    {
        public:
            VertexPrinter(std::ostream& os, VertexFormat format) : m_os{os}, m_format{format} {}

            template<typename DATA_TYPE>
            std::ostream& operator<< (const Vertex<DATA_TYPE>& vertex) const
            {
                if (m_format == VertexFormat::SHORT)
                {
                    std::string id = boost::uuids::to_string(vertex.m_id);
                    m_os << "{" << id.substr(0, 2) << "..." << id.substr(id.size() - 2, 2) << "}: ";
                }
                else
                {
                    m_os << "{" << vertex.m_id << "}: ";
                }
                m_os << vertex.m_data;
                return m_os;
            }

            template<typename DATA_TYPE, template <typename> typename DATA_STRUCTURE>
            std::ostream& operator<< (const Graph<DATA_TYPE, DATA_STRUCTURE>& graph) const
            {
                if (m_format == VertexFormat::SHORT)
                {
                    VertexPrinter{m_os, VertexFormat::SHORT} << graph.m_data_structure;
                }
                else
                {
                    VertexPrinter{m_os, VertexFormat::LONG} << graph.m_data_structure;
                }
                return m_os;
            }

            template<typename DATA_TYPE>
            std::ostream& operator<< ([[maybe_unused]] const AdjacencyList<DATA_TYPE>& adjacency_list) const
            {
                BFS<AdjacencyList<DATA_TYPE>> algorithm;
                algorithm.template traverse<true>(adjacency_list, [this](const Vertex<DATA_TYPE>& vertex) {
                    if (m_format == VertexFormat::SHORT)
                    {
                        VertexPrinter{m_os, VertexFormat::SHORT} << vertex << std::endl;
                    }
                    else
                    {
                        VertexPrinter{m_os, VertexFormat::LONG} << vertex << std::endl;
                    }
                });

                // for (auto it = adjacency_list.begin(); it != adjacency_list.end(); ++it)
                // {
                //     std::cout << "asd" << std::endl;
                // }

                return m_os;
            }

        private:
            std::ostream& m_os;
            const VertexFormat m_format;
    };

    VertexPrinter operator<< (std::ostream& os, VertexFormat format)
    {
        return VertexPrinter{os, format};
    }

    template <typename DATA_TYPE>
    class DataStructureBase
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        public:
            DataStructureBase() = default;
            virtual ~DataStructureBase() = default;

            virtual boost::uuids::uuid add_vertex(const VERTEX_TYPE&& vertex) = 0;
            virtual boost::uuids::uuid add_vertex(const DATA_TYPE&& data) = 0;
            virtual void add_edge(boost::uuids::uuid vertex1, boost::uuids::uuid vertex2) = 0;
    };

    template <typename DATA_TYPE>
    auto addImpl = [] (Vertex<DATA_TYPE>& vertex, DATA_TYPE a) 
    {
        vertex.get_data() = vertex.get_data() + a;
    };
    
    template <typename DATA_TYPE>
    auto add = std::bind(addImpl<DATA_TYPE>, std::placeholders::_1, std::placeholders::_2);

    template <typename DATA_TYPE>
    auto increment = std::bind(addImpl<DATA_TYPE>, std::placeholders::_1, 1);

    template <typename DATA_TYPE>
    auto decrement = std::bind(addImpl<DATA_TYPE>, std::placeholders::_1, -1);

    template <typename DATA_TYPE>
    auto multiplyImpl = [] (Vertex<DATA_TYPE>& vertex, DATA_TYPE a) 
    {
        vertex.get_data() = vertex.get_data() * a;
    };

    template <typename DATA_TYPE>
    auto multiply = std::bind(multiplyImpl<DATA_TYPE>, std::placeholders::_1, std::placeholders::_2);

    template <typename DATA_TYPE, VertexFormat format = VertexFormat::SHORT, std::ostream& os = std::cout>
    auto printImpl = [] (const Vertex<DATA_TYPE>& vertex) 
    {
        os << format << vertex << std::endl;
    };

    template <typename DATA_TYPE, VertexFormat format = VertexFormat::SHORT, std::ostream& os = std::cout>
    auto print = std::bind(printImpl<DATA_TYPE, format, os>, std::placeholders::_1);

    template <typename DATA_TYPE>
    class AdjacencyList : public DataStructureBase<DATA_TYPE>
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        public:

            AdjacencyList() = default;
            ~AdjacencyList() = default;

            boost::uuids::uuid add_vertex(const VERTEX_TYPE&& vertex) override
            {
                auto new_vertex = std::make_shared<VERTEX_TYPE>(vertex);
                auto id = new_vertex->get_id();
                m_vertices.insert(std::make_pair(id, new_vertex));
                return id;
            }

            boost::uuids::uuid add_vertex(const DATA_TYPE&& data) override
            {
                VERTEX_TYPE vertex{std::move(data)};
                return add_vertex(std::move(vertex));
            }

            void add_edge(boost::uuids::uuid vertex1, boost::uuids::uuid vertex2) override
            {
                m_vertices[vertex1]->m_adjacent_vertices.push_back(m_vertices[vertex2]);
                m_vertices[vertex2]->m_adjacent_vertices.push_back(m_vertices[vertex1]);
            }

            const VERTEX_TYPE& get_vertex(boost::uuids::uuid id) const { return *m_vertices.at(id); }
            VERTEX_TYPE& get_vertex(boost::uuids::uuid id) { return *m_vertices.at(id); }

            auto adj_begin(boost::uuids::uuid id) const
            {
                return m_vertices.at(id)->m_adjacent_vertices.begin();
            }

            auto adj_end(boost::uuids::uuid id) const
            {
                return m_vertices.at(id)->m_adjacent_vertices.end();
            }

            template <template <typename> typename ALGORITHM = BFS, bool VISIT_ALL = false, typename FUNCTION, typename... ARGS>
            void traverse(FUNCTION function, ARGS&&... args)
            {
                ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
                algorithm.template traverse<VISIT_ALL>(*this, function, std::forward<ARGS>(args)...);
            }

            template <template <typename> typename ALGORITHM = BFS, bool VISIT_ALL = false>
            void traverse()
            {
                ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
                algorithm.template traverse<VISIT_ALL>(*this, sgl::print<DATA_TYPE>);
            }

            template <template <typename> typename ALGORITHM = BFS>
            std::ostream& print(std::ostream& os = std::cout) const
            {
                ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
                algorithm.template traverse<true>(*this, [&os](const VERTEX_TYPE& vertex) 
                { 
                    os << vertex << std::endl;
                });
                return os;
            }

            friend std::ostream& operator<<(std::ostream& os, const AdjacencyList& list)
            {
                return list.print(os);
            }


        private:
            std::map<boost::uuids::uuid, std::shared_ptr<VERTEX_TYPE>> m_vertices;

            friend class BFS<AdjacencyList<DATA_TYPE>>;
            friend class DFS<AdjacencyList<DATA_TYPE>>;
        
        public:
           
    
    };

    template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE = AdjacencyList>
    class Graph
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        public:
            Graph() = default;
            ~Graph() = default;

            boost::uuids::uuid add_vertex(const VERTEX_TYPE&& vertex)
            {
                return m_data_structure.add_vertex(std::move(vertex));
            }

            boost::uuids::uuid add_vertex(const DATA_TYPE&& data)
            {
                return m_data_structure.add_vertex(std::move(data));
            }

            void add_edge(boost::uuids::uuid vertex1_id, boost::uuids::uuid vertex2_id)
            {
                m_data_structure.add_edge(vertex1_id, vertex2_id);
            }

            const VERTEX_TYPE& get_vertex(boost::uuids::uuid id) const { return m_data_structure.get_vertex(id); }
            VERTEX_TYPE& get_vertex(boost::uuids::uuid id) { return m_data_structure.get_vertex(id); }

            template <template <typename> typename ALGORITHM = BFS, bool VISIT_ALL = false, typename FUNCTION, typename... ARGS>
            void traverse(FUNCTION function, ARGS&&... args)
            {
                m_data_structure.template traverse<ALGORITHM, VISIT_ALL>(function, std::forward<ARGS>(args)...);
            }

            template <template <typename> typename ALGORITHM = BFS, bool VISIT_ALL = false>
            void traverse()
            {
                m_data_structure.template traverse<ALGORITHM, VISIT_ALL>();
            }

            template <template <typename> typename ALGORITHM = BFS>
            std::ostream& print(std::ostream& os = std::cout) const
            {
                return m_data_structure.template print<ALGORITHM>(os);
            }

            friend std::ostream& operator<<(std::ostream& os, const Graph& graph)
            {
                return os << graph.m_data_structure;
            }

        private:
            DATA_STRUCTURE<DATA_TYPE> m_data_structure;

            friend class VertexPrinter;
    }; 

    // Breadth First Search class
    template <typename DATA_STRUCTURE>
    class BFS
    {
        using VERTEX_TYPE = typename DATA_STRUCTURE::VERTEX_TYPE;

        public:
            BFS() = default;
            ~BFS() = default;

            template <bool VISIT_ALL = false, typename FUNCTION, typename... ARGS>
            void traverse(DATA_STRUCTURE& data_structure, FUNCTION function, ARGS&&... args)
            {
                std::queue<boost::uuids::uuid> queue;
                std::set<boost::uuids::uuid> visited;

                VERTEX_TYPE& start_vertex = data_structure.get_vertex(data_structure.m_vertices.begin()->first);
                queue.push(start_vertex.get_id());
                visited.insert(start_vertex.get_id());

                while (!queue.empty())
                {

                    VERTEX_TYPE& vertex = data_structure.get_vertex(queue.front());
                    queue.pop();

                    function(vertex, std::forward<ARGS>(args)...);
                    
                    for (auto it = data_structure.adj_begin(vertex.get_id()); it != data_structure.adj_end(vertex.get_id()); ++it)
                    {
                        if (visited.find((*it)->get_id()) == visited.end())
                        {
                            queue.push((*it)->get_id());
                            visited.insert((*it)->get_id());
                        }
                    }
                }

                if constexpr (VISIT_ALL)
                {
                    for (auto it = data_structure.m_vertices.begin(); it != data_structure.m_vertices.end(); ++it)
                    {
                        if (visited.find(it->first) == visited.end())
                        {
                            VERTEX_TYPE& vertex = data_structure.get_vertex(it->first);
                            function(vertex, std::forward<ARGS>(args)...);
                        }
                    }
                }
            }

            template <bool VISIT_ALL = false, typename FUNCTION, typename... ARGS>
            void traverse(const DATA_STRUCTURE& data_structure, FUNCTION function, ARGS&&... args) const
            {
                std::queue<boost::uuids::uuid> queue;
                std::set<boost::uuids::uuid> visited;

                const VERTEX_TYPE& start_vertex = data_structure.get_vertex(data_structure.m_vertices.begin()->first);
                queue.push(start_vertex.get_id());
                visited.insert(start_vertex.get_id());

                while (!queue.empty())
                {

                    const VERTEX_TYPE& vertex = data_structure.get_vertex(queue.front());
                    queue.pop();

                    function(vertex, std::forward<ARGS>(args)...);
                    
                    for (auto it = data_structure.adj_begin(vertex.get_id()); it != data_structure.adj_end(vertex.get_id()); ++it)
                    {
                        if (visited.find((*it)->get_id()) == visited.end())
                        {
                            queue.push((*it)->get_id());
                            visited.insert((*it)->get_id());
                        }
                    }
                }

                if constexpr (VISIT_ALL)
                {
                    for (auto it = data_structure.m_vertices.begin(); it != data_structure.m_vertices.end(); ++it)
                    {
                        if (visited.find(it->first) == visited.end())
                        {
                            const VERTEX_TYPE& vertex = data_structure.get_vertex(it->first);
                            function(vertex, std::forward<ARGS>(args)...);
                        }
                    }
                }
            }
    };

    // Depth First Search class
    template <typename DATA_STRUCTURE>
    class DFS
    {
        using VERTEX_TYPE = typename DATA_STRUCTURE::VERTEX_TYPE;

        public:
            DFS() = default;
            ~DFS() = default;

            template <bool VISIT_ALL = false, typename FUNCTION, typename... ARGS>
            void traverse(DATA_STRUCTURE& data_structure, FUNCTION function, ARGS&&... args)
            {
                std::stack<boost::uuids::uuid> stack;
                std::set<boost::uuids::uuid> visited;

                VERTEX_TYPE& start_vertex = data_structure.get_vertex(data_structure.m_vertices.begin()->first);
                stack.push(start_vertex.get_id());
                visited.insert(start_vertex.get_id());

                while (!stack.empty())
                {

                    VERTEX_TYPE& vertex = data_structure.get_vertex(stack.top());
                    stack.pop();

                    function(vertex, std::forward<ARGS>(args)...);
                    
                    for (auto it = data_structure.adj_begin(vertex.get_id()); it != data_structure.adj_end(vertex.get_id()); ++it)
                    {
                        if (visited.find((*it)->get_id()) == visited.end())
                        {
                            stack.push((*it)->get_id());
                            visited.insert((*it)->get_id());
                        }
                    }
                }

                if constexpr (VISIT_ALL)
                {
                    for (auto it = data_structure.m_vertices.begin(); it != data_structure.m_vertices.end(); ++it)
                    {
                        if (visited.find(it->first) == visited.end())
                        {
                            VERTEX_TYPE& vertex = data_structure.get_vertex(it->first);
                            function(vertex, std::forward<ARGS>(args)...);
                        }
                    }
                }
            }

            template <bool VISIT_ALL = false, typename FUNCTION, typename... ARGS>
            void traverse(const DATA_STRUCTURE& data_structure, FUNCTION function, ARGS&&... args) const
            {
                std::stack<boost::uuids::uuid> stack;
                std::set<boost::uuids::uuid> visited;

                const VERTEX_TYPE& start_vertex = data_structure.get_vertex(data_structure.m_vertices.begin()->first);
                stack.push(start_vertex.get_id());
                visited.insert(start_vertex.get_id());

                while (!stack.empty())
                {

                    const VERTEX_TYPE& vertex = data_structure.get_vertex(stack.top());
                    stack.pop();

                    function(vertex, std::forward<ARGS>(args)...);
                    
                    for (auto it = data_structure.adj_begin(vertex.get_id()); it != data_structure.adj_end(vertex.get_id()); ++it)
                    {
                        if (visited.find((*it)->get_id()) == visited.end())
                        {
                            stack.push((*it)->get_id());
                            visited.insert((*it)->get_id());
                        }
                    }
                }
            
                if constexpr (VISIT_ALL)
                {
                    for (auto it = data_structure.m_vertices.begin(); it != data_structure.m_vertices.end(); ++it)
                    {
                        if (visited.find(it->first) == visited.end())
                        {
                            const VERTEX_TYPE& vertex = data_structure.get_vertex(it->first);
                            function(vertex, std::forward<ARGS>(args)...);
                        }
                    }
                }

            }
    };

} // namespace sgl

#endif // SGL_HH  
