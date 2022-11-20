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

#include <boost/uuid/uuid.hpp>                  // uuid class
#include <boost/uuid/uuid_generators.hpp>       // generators
#include <boost/uuid/uuid_io.hpp>               // streaming operators etc.
#include <boost/random/mersenne_twister.hpp>    //random generator

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

    using uuid = boost::uuids::uuid;

    template <typename DATA_TYPE>
    class Vertex
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        friend class VertexPrinter;

        public:
            Vertex(const DATA_TYPE&& data) : m_id{s_generator()}, m_data{data} {}

            ~Vertex() = default;

            const uuid& get_id() const { return m_id; }
            const DATA_TYPE& get_data() const { return m_data; }
            DATA_TYPE& get_data() { return m_data; }

            friend std::ostream& operator<<(std::ostream& os, const VERTEX_TYPE& vertex)
            {
                std::string id = boost::uuids::to_string(vertex.m_id);
                return os << "{" << id.substr(0, 2) << "..." << id.substr(id.size() - 2, 2) << "}: " << vertex.m_data;
            }
            
        private:
            uuid m_id;
            DATA_TYPE m_data;

            static inline boost::random::mt19937 rnd = boost::random::mt19937{0};
            static inline boost::uuids::basic_random_generator<boost::random::mt19937> s_generator = boost::uuids::basic_random_generator<boost::random::mt19937>{rnd};
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

            virtual uuid add_vertex(const VERTEX_TYPE&& vertex) = 0;
            virtual uuid add_vertex(const DATA_TYPE&& data) = 0;
            virtual void add_edge(uuid vertex1, uuid vertex2) = 0;

            virtual void remove_vertex(uuid vertex) = 0;
            virtual void remove_edge(uuid vertex1, uuid vertex2) = 0;

        protected:
            class const_iterator_impl
            {
                public:
                    const_iterator_impl() = default;
                    virtual ~const_iterator_impl() = default;

                    virtual bool operator==(const const_iterator_impl* other) const = 0;
                    virtual bool operator!=(const const_iterator_impl* other) const = 0;
                    virtual const_iterator_impl* operator++() = 0;
                    virtual const VERTEX_TYPE& operator*() const = 0;
                    virtual const VERTEX_TYPE* operator->() const = 0;
            };

            class iterator_impl
            {
                public:
                    iterator_impl() = default;
                    virtual ~iterator_impl() = default;

                    virtual bool operator==(const iterator_impl* other) const = 0;
                    virtual bool operator!=(const iterator_impl* other) const = 0;
                    virtual iterator_impl* operator++() = 0;
                    virtual VERTEX_TYPE& operator*() const = 0;
                    virtual VERTEX_TYPE* operator->() const = 0;
            };
            
        public:
            class const_iterator
            {
                public:
                    const_iterator(const_iterator_impl* ptr) : m_ptr{ptr} {}
                    ~const_iterator() { delete m_ptr; }

                    bool operator==(const const_iterator& other) const { return *m_ptr == other.m_ptr; }
                    bool operator!=(const const_iterator& other) const { return *m_ptr != other.m_ptr; }
                    const_iterator& operator++() { ++(*m_ptr); return *this; }
                    const VERTEX_TYPE& operator*() const { return **m_ptr; }
                    const VERTEX_TYPE* operator->() const { return (*m_ptr).operator->(); }

                private:
                    const_iterator_impl* m_ptr;
            };

            class iterator
            {
                public:
                    iterator(iterator_impl* ptr) : m_ptr{ptr} {}
                    ~iterator() { delete m_ptr; }

                    bool operator==(const iterator& other) const { return *m_ptr == other.m_ptr; }
                    bool operator!=(const iterator& other) const { return *m_ptr != other.m_ptr; }
                    iterator& operator++() { ++(*m_ptr); return *this; }
                    VERTEX_TYPE& operator*() const { return **m_ptr; }
                    VERTEX_TYPE* operator->() const { return (*m_ptr).operator->(); }

                private:
                    iterator_impl* m_ptr;
            };

            virtual const_iterator cbegin() const = 0;
            virtual const_iterator cend() const = 0;
            virtual iterator begin() = 0;
            virtual iterator end() = 0;
    };

    template <typename DATA_TYPE>
    class AdjacencyList : public DataStructureBase<DATA_TYPE>
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        using base_const_iterator_impl = typename DataStructureBase<DATA_TYPE>::const_iterator_impl;
        using base_const_iterator = typename DataStructureBase<DATA_TYPE>::const_iterator;
        using base_iterator_impl = typename DataStructureBase<DATA_TYPE>::iterator_impl;
        using base_iterator = typename DataStructureBase<DATA_TYPE>::iterator;

        friend class BFS<AdjacencyList<DATA_TYPE>>;
        friend class DFS<AdjacencyList<DATA_TYPE>>;
        friend class Graph<DATA_TYPE,AdjacencyList>;

        public:

            AdjacencyList() = default;
            ~AdjacencyList() = default;

            uuid add_vertex(const VERTEX_TYPE&& vertex) override
            {
                auto new_vertex = std::make_shared<VERTEX_TYPE>(vertex);
                auto id = new_vertex->get_id();
                m_vertices.insert(std::make_pair(id, std::make_pair(new_vertex, std::vector<std::shared_ptr<VERTEX_TYPE>>())));
                return id;
            }

            uuid add_vertex(const DATA_TYPE&& data) override
            {
                VERTEX_TYPE vertex{std::move(data)};
                return add_vertex(std::move(vertex));
            }

            void add_edge(uuid vertex1, uuid vertex2) override
            {
                m_vertices[vertex1].second.push_back(m_vertices[vertex2].first);
                m_vertices[vertex2].second.push_back(m_vertices[vertex1].first);
            }

            void remove_vertex(uuid vertex) override
            {
                auto& neighbors = m_vertices[vertex].second;
                for (auto& neighbor : neighbors)
                {
                    auto& neighbor_neighbors = m_vertices[neighbor->get_id()].second;
                    neighbor_neighbors.erase(std::remove_if(neighbor_neighbors.begin(), neighbor_neighbors.end(), [vertex](const std::shared_ptr<VERTEX_TYPE>& neighbor_neighbor) {
                        return neighbor_neighbor->get_id() == vertex;
                    }), neighbor_neighbors.end());
                }
                m_vertices.erase(vertex);
            }

            void remove_edge(uuid vertex1, uuid vertex2) override
            {
                auto& neighbors = m_vertices[vertex1].second;
                neighbors.erase(std::remove_if(neighbors.begin(), neighbors.end(), [vertex2](const std::shared_ptr<VERTEX_TYPE>& neighbor) {
                    return neighbor->get_id() == vertex2;
                }), neighbors.end());
                auto& neighbors2 = m_vertices[vertex2].second;
                neighbors2.erase(std::remove_if(neighbors2.begin(), neighbors2.end(), [vertex1](const std::shared_ptr<VERTEX_TYPE>& neighbor) {
                    return neighbor->get_id() == vertex1;
                }), neighbors2.end());
            }

            const VERTEX_TYPE& get_vertex(uuid id) const { return *m_vertices.at(id).first; }
            VERTEX_TYPE& get_vertex(uuid id) { return *m_vertices.at(id).first; }

            auto adj_begin(uuid id) const
            {
                return m_vertices.at(id).second.begin();
            }

            auto adj_end(uuid id) const
            {
                return m_vertices.at(id).second.end();
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
                algorithm.template traverse<VISIT_ALL>(*this, [] (const Vertex<DATA_TYPE>& vertex) { std::cout << vertex << std::endl; });
            }

            std::ostream& print(std::ostream& os = std::cout) const
            {
                for (auto it = cbegin(); it != cend(); ++it)
                {
                    os << *it << std::endl;
                }
                return os;
            }

            friend std::ostream& operator<<(std::ostream& os, const AdjacencyList& list)
            {
                return list.print(os);
            }


        private:
            std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,std::vector<std::shared_ptr<VERTEX_TYPE>>>> m_vertices;
        
        public:
            virtual base_const_iterator cbegin() const override { return base_const_iterator(new const_iterator(m_vertices.cbegin())); }
            virtual base_const_iterator cend() const override { return base_const_iterator(new const_iterator(m_vertices.cend())); }
            virtual base_iterator begin() override { return base_iterator(new iterator(m_vertices.begin())); }
            virtual base_iterator end() override { return base_iterator(new iterator(m_vertices.end())); }

        private:
            class const_iterator : public base_const_iterator_impl
            {
                public:
                    const_iterator(const typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,std::vector<std::shared_ptr<VERTEX_TYPE>>>>::const_iterator& it) : m_it(it) {}

                    virtual bool operator==(const base_const_iterator_impl* other) const override { return m_it == static_cast<const const_iterator*>(other)->m_it; }
                    virtual bool operator!=(const base_const_iterator_impl* other) const override { return m_it != static_cast<const const_iterator*>(other)->m_it; }
                    virtual base_const_iterator_impl* operator++() override { ++m_it; return this; }
                    virtual const VERTEX_TYPE& operator*() const override { return *m_it->second.first; }
                    virtual const VERTEX_TYPE* operator->() const override { return m_it->second.first.get(); }

                private:
                    typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,std::vector<std::shared_ptr<VERTEX_TYPE>>>>::const_iterator m_it;
            };
            
            class iterator : public base_iterator_impl
            {
                public:
                    iterator(const typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,std::vector<std::shared_ptr<VERTEX_TYPE>>>>::iterator& it) : m_it(it) {}

                    virtual bool operator==(const base_iterator_impl* other) const override { return m_it == static_cast<const iterator*>(other)->m_it; }
                    virtual bool operator!=(const base_iterator_impl* other) const override { return m_it != static_cast<const iterator*>(other)->m_it; }
                    virtual base_iterator_impl* operator++() override { ++m_it; return this; }
                    virtual VERTEX_TYPE& operator*() const override { return *m_it->second.first; }
                    virtual VERTEX_TYPE* operator->() const override { return m_it->second.first.get(); }

                private:
                    typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,std::vector<std::shared_ptr<VERTEX_TYPE>>>>::iterator m_it;
            };

    };

    template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE = AdjacencyList>
    class Graph
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        public:
            Graph() = default;
            ~Graph() = default;

            uuid add_vertex(const VERTEX_TYPE&& vertex)
            {
                return m_data_structure.add_vertex(std::move(vertex));
            }

            uuid add_vertex(const DATA_TYPE&& data)
            {
                return m_data_structure.add_vertex(std::move(data));
            }

            void add_edge(uuid vertex1_id, uuid vertex2_id)
            {
                m_data_structure.add_edge(vertex1_id, vertex2_id);
            }

            void remove_edge(uuid vertex1_id, uuid vertex2_id)
            {
                m_data_structure.remove_edge(vertex1_id, vertex2_id);
            }

            void remove_vertex(uuid vertex_id)
            {
                m_data_structure.remove_vertex(vertex_id);
            }

            const VERTEX_TYPE& get_vertex(uuid id) const { return m_data_structure.get_vertex(id); }
            VERTEX_TYPE& get_vertex(uuid id) { return m_data_structure.get_vertex(id); }

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

            std::ostream& print(std::ostream& os = std::cout) const
            {
                return m_data_structure.print(os);
            }

            friend std::ostream& operator<<(std::ostream& os, const Graph& graph)
            {
                return os << graph.m_data_structure;
            }

        private:
            DATA_STRUCTURE<DATA_TYPE> m_data_structure;

            friend class VertexPrinter;
            
        public:
            using base_const_iterator = typename DATA_STRUCTURE<DATA_TYPE>::base_const_iterator;
            using base_iterator = typename DATA_STRUCTURE<DATA_TYPE>::base_iterator;
            base_const_iterator cbegin() const { return m_data_structure.cbegin(); }
            base_const_iterator cend() const { return m_data_structure.cend(); }
            base_iterator begin() { return m_data_structure.begin(); }
            base_iterator end() { return m_data_structure.end(); }    
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
                std::queue<uuid> queue;
                std::set<uuid> visited;

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
                std::queue<uuid> queue;
                std::set<uuid> visited;

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
                std::stack<uuid> stack;
                std::set<uuid> visited;

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
                std::stack<uuid> stack;
                std::set<uuid> visited;

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

} // namespace sgl

#endif // SGL_HH  
