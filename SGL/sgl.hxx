#ifndef SGL_HH
#define SGL_HH

#include <functional>
#include <iostream>
#include <memory>

#include <map>
#include <queue>
#include <set>
#include <stack>

#include <random>
#include <sstream>

namespace sgl
{
    //////////////////////////////////////////////////////////////////////////////
    //
    //       FORWARD DECLARATIONS
    //

    class uuid;

    template <typename DATA_TYPE>
    class Vertex;

    class VertexPrinter;

    template <typename DATA_TYPE>
    class DataStructureBase;

    template <typename DATA_TYPE>
    class AdjacencyList;

    template <typename DATA_TYPE>
    class AdjacencyMatrix;

    template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE>
    class Graph;

    template <typename DATA_STRUCTURE>
    class BFS;

    template <typename DATA_STRUCTURE>
    class DFS;

    //
    //       END OF FORWARD DECLARATIONS
    //
    //////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////
    //
    //       TYPE DEFINITIONS
    //

    void version()
    {
        std::cout << R"(
│ ╔═╗╔═╗╦      Simple Graph Library │
│ ╚═╗║ ╦║        Apache License 2.0 │
│ ╚═╝╚═╝╩═╝           version 0.2.1 │
    )" << std::endl;
    }

    enum VertexFormat
    {
        SHORTEST,
        SHORT,
        LONG
    };

    enum VisitPolicy
    {
        RELATED,
        ALL
    };

    class uuid
    {
    public:
        uuid()
        {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<> dis(0, 15);

            std::stringstream ss;
            for (int i = 0; i < 32; ++i)
            {
                int rand_val = dis(gen);
                char hex_digit = rand_val < 10 ? '0' + rand_val : 'a' + (rand_val - 10);
                ss << hex_digit;
                if (i == 7 || i == 11 || i == 15 || i == 19)
                    ss << '-';
            }
            m_uuid = ss.str();
        }

        operator std::string() const { return m_uuid; }

        auto operator<=>(const uuid &other) const
        {
            return m_uuid <=> other.m_uuid;
        }

        auto operator==(const uuid &other) const
        {
            return m_uuid == other.m_uuid;
        }

        friend std::ostream &operator<<(std::ostream &os, const uuid &uuid)
        {
            return os << uuid.m_uuid;
        }

    private:
        std::string m_uuid;
    };

    template <typename DATA_TYPE>
    class Vertex
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        friend class VertexPrinter;
        friend class DataStructureBase<DATA_TYPE>;
        friend class AdjacencyList<DATA_TYPE>;
        friend class AdjacencyMatrix<DATA_TYPE>;

    public:
        Vertex(DATA_TYPE &&data) : m_uuid{}, m_data{data}, m_data_structure{} {}
        Vertex(DATA_TYPE &&data, std::shared_ptr<DataStructureBase<DATA_TYPE>> m_data_structure)
            : m_uuid{}, m_data{data}, m_data_structure{m_data_structure} {}
        Vertex(const Vertex &other) = delete;
        Vertex(Vertex &&other) : m_uuid{std::move(other.m_uuid)}, m_data{std::move(other.m_data)}, m_data_structure{std::move(other.m_data_structure)} {}

        ~Vertex() = default;

        const uuid &get_id() const { return m_uuid; }
        const DATA_TYPE &get_data() const { return m_data; }
        DATA_TYPE &get_data() { return m_data; }

        typename DataStructureBase<DATA_TYPE>::const_iterator cbegin() const { return m_data_structure->cbegin(m_uuid); }
        typename DataStructureBase<DATA_TYPE>::const_iterator cend() const { return m_data_structure->cend(m_uuid); }
        typename DataStructureBase<DATA_TYPE>::iterator begin() { return m_data_structure->begin(m_uuid); }
        typename DataStructureBase<DATA_TYPE>::iterator end() { return m_data_structure->end(m_uuid); }

        size_t size() const { return m_data_structure->size(m_uuid); }

        void remove()
        {
            if (m_data_structure == nullptr)
            {
                throw std::runtime_error("[void sgl::Vertex::remove()] Vertex is not part of a graph");
            }

            m_data_structure->remove_vertex(m_uuid);
            m_data_structure = nullptr;
        }

        void remove_edge(const uuid &id)
        {
            if (m_data_structure == nullptr)
                throw std::runtime_error("[void sgl::Vertex::remove_edge(const uuid &id)] Vertex is not part of a graph");

            m_data_structure->remove_edge(m_uuid, id);
        }

        friend std::ostream &operator<<(std::ostream &os, const VERTEX_TYPE &vertex)
        {
            std::string id = static_cast<std::string>(vertex.m_uuid);
            return os << "[ " << vertex.m_data << " ]";
        }

    private:
        void add_data_structure(const std::shared_ptr<DataStructureBase<DATA_TYPE>> &data_structure)
        {
            m_data_structure = data_structure;
        }

        const uuid m_uuid;
        DATA_TYPE m_data;
        std::shared_ptr<DataStructureBase<DATA_TYPE>> m_data_structure;
    };

    class VertexPrinter
    {
    public:
        VertexPrinter(std::ostream &os, VertexFormat format)
            : m_os{os}, m_format{format} {}

        template <typename DATA_TYPE>
        std::ostream &operator<<(const Vertex<DATA_TYPE> &vertex) const
        {
            if (m_format == VertexFormat::SHORTEST)
            {
                m_os << "[ " << vertex.m_data << " ]";
            }
            else if (m_format == VertexFormat::SHORT)
            {
                std::string id = static_cast<std::string>(vertex.m_uuid);
                m_os << "[ id: {" << id.substr(0, 2) << "..."
                     << id.substr(id.size() - 2, 2) << "}, ";
                m_os << "data: {" << vertex.m_data;
                m_os << "}, neighbors: {" << vertex.size() << "} ]";
            }
            else if (m_format == VertexFormat::LONG)
            {
                m_os << "[ id: {" << vertex.m_uuid << "}, ";
                m_os << "data: {" << vertex.m_data;
                m_os << "}, neighbors: {" << vertex.size() << "} ]";
            }

            return m_os;
        }

        template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE>
        std::ostream &operator<<(const Graph<DATA_TYPE, DATA_STRUCTURE> &graph) const
        {
            if (m_format == VertexFormat::SHORTEST)
            {
                VertexPrinter{m_os, VertexFormat::SHORTEST} << *(graph.m_data_structure);
            }
            else if (m_format == VertexFormat::SHORT)
            {
                VertexPrinter{m_os, VertexFormat::SHORT} << *(graph.m_data_structure);
            }
            else if (m_format == VertexFormat::LONG)
            {
                VertexPrinter{m_os, VertexFormat::LONG} << *(graph.m_data_structure);
            }

            return m_os;
        }

        template <typename DATA_TYPE>
        std::ostream &operator<<(const AdjacencyList<DATA_TYPE> &adjacency_list) const
        {
            for (auto it = adjacency_list.cbegin(); it != adjacency_list.cend(); ++it)
            {
                if (m_format == VertexFormat::SHORTEST)
                {
                    VertexPrinter{m_os, VertexFormat::SHORTEST} << *it << std::endl;
                }
                else if (m_format == VertexFormat::SHORT)
                {
                    VertexPrinter{m_os, VertexFormat::SHORT} << *it << std::endl;
                }
                else if (m_format == VertexFormat::LONG)
                {
                    VertexPrinter{m_os, VertexFormat::LONG} << *it << std::endl;
                }
            }
            return m_os;
        }

        template <typename DATA_TYPE>
        std::ostream &operator<<(const AdjacencyMatrix<DATA_TYPE> &adjacency_matrix) const
        {
            for (auto it = adjacency_matrix.cbegin(); it != adjacency_matrix.cend(); ++it)
            {
                if (m_format == VertexFormat::SHORTEST)
                {
                    VertexPrinter{m_os, VertexFormat::SHORTEST} << *it << std::endl;
                }
                else if (m_format == VertexFormat::SHORT)
                {
                    VertexPrinter{m_os, VertexFormat::SHORT} << *it << std::endl;
                }
                else if (m_format == VertexFormat::LONG)
                {
                    VertexPrinter{m_os, VertexFormat::LONG} << *it << std::endl;
                }
            }
            return m_os;
        }

    private:
        std::ostream &m_os;
        const VertexFormat m_format;
    };

    VertexPrinter operator<<(std::ostream &os, VertexFormat format)
    {
        return VertexPrinter{os, format};
    }

    template <typename DATA_TYPE>
    class DataStructureBase
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        friend class Vertex<DATA_TYPE>;

    public:
        DataStructureBase() = default;
        virtual ~DataStructureBase() = default;

        virtual const uuid &add_vertex(VERTEX_TYPE &&vertex) = 0;
        virtual const uuid &add_vertex(DATA_TYPE &&data) = 0;
        virtual void add_edge(const uuid &vertex1, const uuid &vertex2, const float weight = 0) = 0;

        virtual void remove_vertex(const uuid &vertex) = 0;
        virtual void remove_edge(const uuid &vertex1, const uuid &vertex2) = 0;

        virtual const VERTEX_TYPE &get_vertex(const uuid &id) const = 0;
        virtual VERTEX_TYPE &get_vertex(const uuid &id) = 0;

        virtual size_t size() const = 0;
        virtual size_t size(const uuid &id) const = 0;
        virtual void empty() = 0;

        virtual std::ostream &print(std::ostream &os = std::cout) const = 0;

    protected:
        class const_iterator_impl
        {
        public:
            const_iterator_impl() = default;
            virtual ~const_iterator_impl() = default;

            virtual bool operator==(const const_iterator_impl *other) const = 0;
            virtual bool operator!=(const const_iterator_impl *other) const = 0;
            virtual const_iterator_impl *operator++() = 0;
            virtual const VERTEX_TYPE &operator*() const = 0;
            virtual const VERTEX_TYPE *operator->() const = 0;
        };

        class iterator_impl
        {
        public:
            iterator_impl() = default;
            virtual ~iterator_impl() = default;

            virtual bool operator==(const iterator_impl *other) const = 0;
            virtual bool operator!=(const iterator_impl *other) const = 0;
            virtual iterator_impl *operator++() = 0;
            virtual VERTEX_TYPE &operator*() const = 0;
            virtual VERTEX_TYPE *operator->() const = 0;
        };

    public:
        class const_iterator
        {
        public:
            const_iterator(const std::shared_ptr<const_iterator_impl> &ptr) : m_ptr{ptr} {}
            ~const_iterator() = default;

            bool operator==(const const_iterator &other) const { return *m_ptr == other.m_ptr.get(); }
            bool operator!=(const const_iterator &other) const { return *m_ptr != other.m_ptr.get(); }
            const_iterator &operator++()
            {
                ++(*m_ptr);
                return *this;
            }
            const VERTEX_TYPE &operator*() const { return **m_ptr; }
            const VERTEX_TYPE *operator->() const { return (*m_ptr).operator->(); }

        private:
            std::shared_ptr<const_iterator_impl> m_ptr;
        };

        class iterator
        {
        public:
            iterator(const std::shared_ptr<iterator_impl> &ptr) : m_ptr{ptr} {}
            ~iterator() = default;

            bool operator==(const iterator &other) const { return *m_ptr == other.m_ptr.get(); }
            bool operator!=(const iterator &other) const { return *m_ptr != other.m_ptr.get(); }
            iterator &operator++()
            {
                ++(*m_ptr);
                return *this;
            }
            VERTEX_TYPE &operator*() const { return **m_ptr; }
            VERTEX_TYPE *operator->() const { return (*m_ptr).operator->(); }

        private:
            std::shared_ptr<iterator_impl> m_ptr;
        };

        virtual const_iterator cbegin() const = 0;
        virtual const_iterator cend() const = 0;
        virtual iterator begin() = 0;
        virtual iterator end() = 0;

        virtual const_iterator cbegin(const uuid &id) const = 0;
        virtual const_iterator cend(const uuid &id) const = 0;
        virtual iterator begin(const uuid &id) = 0;
        virtual iterator end(const uuid &id) = 0;
    };

    template <typename DATA_TYPE>
    class AdjacencyList : public DataStructureBase<DATA_TYPE>, public std::enable_shared_from_this<AdjacencyList<DATA_TYPE>>
    {
    public:
        ~AdjacencyList() = default;

    private:
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        using base_iterator_impl =
            typename DataStructureBase<DATA_TYPE>::iterator_impl;

        using base_iterator = typename DataStructureBase<DATA_TYPE>::iterator;

        using base_const_iterator_impl =
            typename DataStructureBase<DATA_TYPE>::const_iterator_impl;

        using base_const_iterator =
            typename DataStructureBase<DATA_TYPE>::const_iterator;

        using const_neighbor_iterator =
            typename std::vector<std::shared_ptr<VERTEX_TYPE>>::const_iterator;

        using neighbor_iterator =
            typename std::vector<std::shared_ptr<VERTEX_TYPE>>::iterator;

        using const_vertex_iterator =
            typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>, std::vector<std::shared_ptr<VERTEX_TYPE>>>>::const_iterator;

        using vertex_iterator =
            typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>, std::vector<std::shared_ptr<VERTEX_TYPE>>>>::iterator;

        friend class BFS<AdjacencyList<DATA_TYPE>>;
        friend class DFS<AdjacencyList<DATA_TYPE>>;
        friend class Graph<DATA_TYPE, AdjacencyList>;
        friend class VertexPrinter;

        std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                 std::vector<std::shared_ptr<VERTEX_TYPE>>>>
            m_vertices;

        AdjacencyList() = default;

        const uuid &add_vertex(VERTEX_TYPE &&vertex) override
        {
            auto new_vertex = std::make_shared<VERTEX_TYPE>(std::forward<VERTEX_TYPE>(vertex));
            new_vertex->add_data_structure(this->shared_from_this());
            const uuid &id = new_vertex->get_id();
            m_vertices.insert(std::make_pair(
                id,
                std::make_pair(std::move(new_vertex),
                               std::vector<std::shared_ptr<VERTEX_TYPE>>())));
            return id;
        }

        const uuid &add_vertex(DATA_TYPE &&data) override
        {
            VERTEX_TYPE vertex{std::forward<DATA_TYPE>(data)};
            return add_vertex(std::move(vertex));
        }

        void add_edge(const uuid &vertex1, const uuid &vertex2, const float weight = 0) override
        {
            try
            {
                m_vertices.at(vertex1).second.push_back(m_vertices.at(vertex2).first);
                m_vertices.at(vertex2).second.push_back(m_vertices.at(vertex1).first);
            }
            catch (const std::out_of_range &e)
            {
                throw std::out_of_range{"[void sgl::AdjacencyList::add_edge(const uuid &vertex1, const uuid &vertex2, const float weight = 0)] Vertex with id " + static_cast<std::string>(vertex1) + " or " + static_cast<std::string>(vertex2) + " not found"};
            }
        }

        void remove_vertex(const uuid &vertex) override
        {
            if (m_vertices.find(vertex) == m_vertices.end())
            {
                throw std::out_of_range{"[void sgl::AdjacencyList::remove_vertex(const uuid &vertex) override] Vertex with id " + static_cast<std::string>(vertex) + " not found"};
            }

            auto &neighbors = m_vertices[vertex].second;
            for (auto &neighbor : neighbors)
            {
                auto &neighbor_neighbors = m_vertices[neighbor->get_id()].second;
                neighbor_neighbors.erase(
                    std::remove_if(
                        neighbor_neighbors.begin(), neighbor_neighbors.end(),
                        [vertex](const std::shared_ptr<VERTEX_TYPE> &neighbor_neighbor)
                        {
                            return neighbor_neighbor->get_id() == vertex;
                        }),
                    neighbor_neighbors.end());
            }
            m_vertices.erase(vertex);
        }

        void remove_edge(const uuid &vertex1, const uuid &vertex2) override
        {
            if (m_vertices.find(vertex1) == m_vertices.end())
            {
                throw std::out_of_range{"[void sgl::AdjacencyList::remove_edge(const uuid &vertex1, const uuid &vertex2)] Vertex with id " + static_cast<std::string>(vertex1) + " not found"};
            }

            if (m_vertices.find(vertex2) == m_vertices.end())
            {
                throw std::out_of_range{"[void sgl::AdjacencyList::remove_edge(const uuid &vertex1, const uuid &vertex2)] Vertex with id " + static_cast<std::string>(vertex2) + " not found"};
            }

            auto &neighbors = m_vertices[vertex1].second;
            neighbors.erase(
                std::remove_if(neighbors.begin(), neighbors.end(),
                               [vertex2](const std::shared_ptr<VERTEX_TYPE> &neighbor)
                               {
                                   return neighbor->get_id() == vertex2;
                               }),
                neighbors.end());

            auto &neighbors2 = m_vertices[vertex2].second;
            neighbors2.erase(
                std::remove_if(neighbors2.begin(), neighbors2.end(),
                               [vertex1](const std::shared_ptr<VERTEX_TYPE> &neighbor)
                               {
                                   return neighbor->get_id() == vertex1;
                               }),
                neighbors2.end());
        }

        template <typename FUNCTION, typename... ARGS>
        void remove_if(FUNCTION &&function, ARGS &&...args)
        {
            if constexpr (!std::is_same_v<
                              bool, std::invoke_result_t<FUNCTION, const VERTEX_TYPE &,
                                                         ARGS...>>)
            {
                throw std::invalid_argument("[void sgl::AdjacencyList::remove_if(FUNCTION &&function, ARGS &&...args)] return type of function must be bool");
            }
            else
            {
                std::vector<uuid> to_remove;
                for (auto it = cbegin(); it != cend(); ++it)
                {
                    if (function(*it, std::forward<ARGS>(args)...))
                    {
                        to_remove.push_back(it->get_id());
                    }
                }
                for (auto &id : to_remove)
                {
                    remove_vertex(id);
                }
            }
        }

        const VERTEX_TYPE &get_vertex(const uuid &id) const override
        {
            return *m_vertices.at(id).first;
        }
        VERTEX_TYPE &get_vertex(const uuid &id) override
        {
            return *m_vertices.at(id).first;
        }

        size_t size() const override { return m_vertices.size(); }
        size_t size(const uuid &id) const override
        {
            return m_vertices.at(id).second.size();
        }
        void empty() override { m_vertices.clear(); }

        std::ostream &print(std::ostream &os = std::cout) const override
        {
            for (auto it = cbegin(); it != cend(); ++it)
            {
                os << *it << std::endl;
            }
            return os;
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(const uuid &id, FUNCTION function, ARGS &&...args)
        {
            ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
            algorithm.template traverse<policy>(*this, id, function,
                                                std::forward<ARGS>(args)...);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(FUNCTION function, ARGS &&...args)
        {
            ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
            const uuid &id = m_vertices.begin()->first;
            algorithm.template traverse<policy>(*this, id, function,
                                                std::forward<ARGS>(args)...);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED>
        void traverse(const uuid &id)
        {
            ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
            algorithm.template traverse<policy>(*this, id,
                                                [](const Vertex<DATA_TYPE> &vertex)
                                                {
                                                    std::cout << vertex << std::endl;
                                                });
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED>
        void traverse()
        {
            ALGORITHM<AdjacencyList<DATA_TYPE>> algorithm;
            const uuid &id = m_vertices.begin()->first;
            algorithm.template traverse<policy>(*this, id,
                                                [](const Vertex<DATA_TYPE> &vertex)
                                                {
                                                    std::cout << vertex << std::endl;
                                                });
        }

        friend std::ostream &operator<<(std::ostream &os, const AdjacencyList &list)
        {
            return list.print(os);
        }

        base_const_iterator cbegin() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cbegin())}; }
        base_const_iterator cend() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cend())}; }
        base_iterator begin() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.begin())}; }
        base_iterator end() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.end())}; }

        base_const_iterator cbegin(const uuid &id) const override { return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(m_vertices.at(id).second.cbegin())}; }
        base_const_iterator cend(const uuid &id) const override { return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(m_vertices.at(id).second.cend())}; }
        base_iterator begin(const uuid &id) override { return base_iterator{std::make_shared<iterator<neighbor_iterator>>(m_vertices.at(id).second.begin())}; }
        base_iterator end(const uuid &id) override { return base_iterator{std::make_shared<iterator<neighbor_iterator>>(m_vertices.at(id).second.end())}; }

        template <typename ITERATOR>
        class iterator : public base_iterator_impl
        {
        public:
            iterator(const ITERATOR &it) : m_it{it} {}
            bool operator==(const base_iterator_impl *other) const override { return m_it == static_cast<const iterator *>(other)->m_it; }
            bool operator!=(const base_iterator_impl *other) const override { return m_it != static_cast<const iterator *>(other)->m_it; }
            base_iterator_impl *operator++() override
            {
                ++m_it;
                return this;
            }

            VERTEX_TYPE &operator*() const override
            {
                if constexpr (std::is_same_v<ITERATOR, vertex_iterator>)
                {
                    return *m_it->second.first;
                }
                else if constexpr (std::is_same_v<ITERATOR, neighbor_iterator>)
                {
                    return **m_it;
                }
            }

            VERTEX_TYPE *operator->() const override
            {
                if constexpr (std::is_same_v<ITERATOR, vertex_iterator>)
                {
                    return m_it->second.first.get();
                }
                else if (std::is_same_v<ITERATOR, neighbor_iterator>)
                {
                    return m_it->get();
                }
            }

        private:
            ITERATOR m_it;
        };

        template <typename ITERATOR>
        class const_iterator : public base_const_iterator_impl
        {
        public:
            const_iterator(const ITERATOR &it) : m_it{it} {}
            bool operator==(const base_const_iterator_impl *other) const override { return m_it == static_cast<const const_iterator *>(other)->m_it; }
            bool operator!=(const base_const_iterator_impl *other) const override { return m_it != static_cast<const const_iterator *>(other)->m_it; }
            base_const_iterator_impl *operator++() override
            {
                ++m_it;
                return this;
            }

            const VERTEX_TYPE &operator*() const override
            {
                if constexpr (std::is_same_v<ITERATOR, const_vertex_iterator>)
                {
                    return *m_it->second.first;
                }
                else if constexpr (std::is_same_v<ITERATOR, const_neighbor_iterator>)
                {
                    return **m_it;
                }
            }

            const VERTEX_TYPE *operator->() const override
            {
                if constexpr (std::is_same_v<ITERATOR, const_vertex_iterator>)
                {
                    return m_it->second.first.get();
                }
                else if constexpr (std::is_same_v<ITERATOR, const_neighbor_iterator>)
                {
                    return m_it->get();
                }
            }

        private:
            ITERATOR m_it;
        };
    };

    template <typename DATA_TYPE>
    class AdjacencyMatrix : public DataStructureBase<DATA_TYPE>, public std::enable_shared_from_this<AdjacencyMatrix<DATA_TYPE>>
    {
    public:
        ~AdjacencyMatrix() = default;

    private:
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        using base_iterator_impl =
            typename DataStructureBase<DATA_TYPE>::iterator_impl;

        using base_iterator = typename DataStructureBase<DATA_TYPE>::iterator;

        using base_const_iterator_impl =
            typename DataStructureBase<DATA_TYPE>::const_iterator_impl;

        using base_const_iterator =
            typename DataStructureBase<DATA_TYPE>::const_iterator;

        using const_neighbor_iterator =
            typename std::map<uuid, float>::const_iterator;

        using neighbor_iterator =
            typename std::map<uuid, float>::iterator;

        using const_vertex_iterator =
            typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>, std::map<uuid, float>>>::const_iterator;

        using vertex_iterator =
            typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>, std::map<uuid, float>>>::iterator;

        friend class BFS<AdjacencyMatrix<DATA_TYPE>>;
        friend class DFS<AdjacencyMatrix<DATA_TYPE>>;
        friend class Graph<DATA_TYPE, AdjacencyMatrix>;
        friend class VertexPrinter;

        std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                 std::map<uuid, float>>>
            m_vertices;

        AdjacencyMatrix() = default;

        const uuid &add_vertex(VERTEX_TYPE &&vertex) override
        {
            auto new_vertex = std::make_shared<VERTEX_TYPE>(std::forward<VERTEX_TYPE>(vertex));
            new_vertex->add_data_structure(this->shared_from_this());
            const uuid &id = new_vertex->get_id();

            m_vertices.insert(std::make_pair(id, std::make_pair(new_vertex, std::map<uuid, float>{})));
            for (auto &v : m_vertices)
            {
                v.second.second.insert(std::make_pair(id, std::nanf("Not adjacent")));
                m_vertices.at(id).second.insert(std::make_pair(v.first, std::nanf("Not adjacent")));
            }

            return id;
        }

        const uuid &add_vertex(DATA_TYPE &&data) override
        {
            VERTEX_TYPE vertex{std::forward<DATA_TYPE>(data)};
            return add_vertex(std::move(vertex));
        }

        void add_edge(const uuid &vertex1, const uuid &vertex2, const float weight = 0) override
        {
            try
            {
                m_vertices.at(vertex1).second.at(vertex2) = weight;
                m_vertices.at(vertex2).second.at(vertex1) = weight;
            }
            catch (const std::out_of_range &e)
            {
                throw std::out_of_range{"[void sgl::AdjacencyMatrix::add_edge(const uuid &vertex1, const uuid &vertex2, const float weight = 0)] Vertex with id " + static_cast<std::string>(vertex1) + " or " + static_cast<std::string>(vertex2) + " not found"};
            }
        }

        void remove_vertex(const uuid &vertex) override
        {
            if (m_vertices.find(vertex) == m_vertices.end())
            {
                throw std::out_of_range{"[void sgl::AdjacencyMatrix::remove_vertex(const uuid &vertex) override] Vertex with id " + static_cast<std::string>(vertex) + " not found"};
            }

            m_vertices.erase(vertex);
            for (auto &v : m_vertices)
            {
                v.second.second.erase(vertex);
            }
        }

        void remove_edge(const uuid &vertex1, const uuid &vertex2) override
        {
            try
            {
                m_vertices.at(vertex1).second.at(vertex2) = std::nanf("Not adjacent");
                m_vertices.at(vertex2).second.at(vertex1) = std::nanf("Not adjacent");
            }
            catch (const std::out_of_range &e)
            {
                throw std::out_of_range{"[void sgl::AdjacencyMatrix::remove_edge(const uuid &vertex1, const uuid &vertex2)] Vertex with id " + static_cast<std::string>(vertex1) + " or " + static_cast<std::string>(vertex2) + " not found"};
            }
        }

        template <typename FUNCTION, typename... ARGS>
        void remove_if(FUNCTION &&function, ARGS &&...args)
        {
            if constexpr (!std::is_same_v<
                              bool, std::invoke_result_t<FUNCTION, const VERTEX_TYPE &,
                                                         ARGS...>>)
            {
                throw std::invalid_argument("[void sgl::AdjacencyMatrix::remove_if(FUNCTION &&function, ARGS &&...args)] return type of function must be bool");
            }
            else
            {
                std::vector<uuid> to_remove;
                for (auto it = cbegin(); it != cend(); ++it)
                {
                    if (function(*it, std::forward<ARGS>(args)...))
                    {
                        to_remove.push_back(it->get_id());
                    }
                }
                for (auto &id : to_remove)
                {
                    remove_vertex(id);
                }
            }
        }

        const VERTEX_TYPE &get_vertex(const uuid &id) const override { return *m_vertices.at(id).first; }
        VERTEX_TYPE &get_vertex(const uuid &id) override { return *m_vertices.at(id).first; }

        size_t size() const override { return m_vertices.size(); }
        size_t size(const uuid &id) const override
        {
            return std::count_if(m_vertices.at(id).second.begin(), m_vertices.at(id).second.end(), [](const auto &pair)
                                 { return !std::isnan(pair.second); });
        }
        void empty() override { m_vertices.clear(); }

        std::ostream &print(std::ostream &os = std::cout) const override
        {
            for (auto it = cbegin(); it != cend(); ++it)
            {
                os << *it << std::endl;
            }
            return os;
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(const uuid &id, FUNCTION function, ARGS &&...args)
        {
            ALGORITHM<AdjacencyMatrix<DATA_TYPE>> algorithm;
            algorithm.template traverse<policy>(*this, id, function,
                                                std::forward<ARGS>(args)...);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(FUNCTION function, ARGS &&...args)
        {
            ALGORITHM<AdjacencyMatrix<DATA_TYPE>> algorithm;
            const uuid &id = m_vertices.begin()->first;
            algorithm.template traverse<policy>(*this, id, function,
                                                std::forward<ARGS>(args)...);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED>
        void traverse(const uuid &id)
        {
            ALGORITHM<AdjacencyMatrix<DATA_TYPE>> algorithm;
            algorithm.template traverse<policy>(*this, id,
                                                [](const Vertex<DATA_TYPE> &vertex)
                                                {
                                                    std::cout << vertex << std::endl;
                                                });
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED>
        void traverse()
        {
            ALGORITHM<AdjacencyMatrix<DATA_TYPE>> algorithm;
            const uuid &id = m_vertices.begin()->first;
            algorithm.template traverse<policy>(*this, id,
                                                [](const Vertex<DATA_TYPE> &vertex)
                                                {
                                                    std::cout << vertex << std::endl;
                                                });
        }

        friend std::ostream &operator<<(std::ostream &os, const AdjacencyMatrix &matrix)
        {
            return matrix.print(os);
        }

        base_const_iterator cbegin() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cbegin(), m_vertices)}; }
        base_const_iterator cend() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cend(), m_vertices)}; }
        base_iterator begin() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.begin(), m_vertices)}; }
        base_iterator end() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.end(), m_vertices)}; }

        base_const_iterator cbegin(const uuid &id) const override
        {
            auto it = m_vertices.at(id).second.cbegin();
            while (it != m_vertices.at(id).second.cend() && !it->second)
            {
                ++it;
            }
            return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(it, m_vertices)};
        }
        base_const_iterator cend(const uuid &id) const override { return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(m_vertices.at(id).second.cend(), m_vertices)}; }
        base_iterator begin(const uuid &id) override
        {
            auto it = m_vertices.at(id).second.begin();
            while (it != m_vertices.at(id).second.end() && !it->second)
            {
                ++it;
            }
            return base_iterator{std::make_shared<iterator<neighbor_iterator>>(it, m_vertices)};
        }
        base_iterator end(const uuid &id) override { return base_iterator{std::make_shared<iterator<neighbor_iterator>>(m_vertices.at(id).second.end(), m_vertices)}; }

        template <typename ITERATOR>
        class iterator : public base_iterator_impl
        {
        public:
            iterator(const ITERATOR &it, std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                                                  std::map<uuid, float>>>
                                             vertices) : m_it{it}, m_vertices{vertices} {}
            bool operator==(const base_iterator_impl *other) const override { return m_it == static_cast<const iterator *>(other)->m_it; }
            bool operator!=(const base_iterator_impl *other) const override { return m_it != static_cast<const iterator *>(other)->m_it; }
            base_iterator_impl *operator++() override
            {
                if constexpr (std::is_same_v<ITERATOR, vertex_iterator>)
                {
                    ++m_it;
                    return this;
                }
                else if constexpr (std::is_same_v<ITERATOR, neighbor_iterator>)
                {
                    if (m_it->first == m_vertices.rbegin()->first)
                    {
                        ++m_it;
                        return this;
                    }
                    else
                    {
                        do
                        {
                            ++m_it;
                        } while (std::isnan(m_it->second));
                        return this;
                    }
                }
            }
            VERTEX_TYPE &operator*() const override
            {
                if constexpr (std::is_same_v<ITERATOR, vertex_iterator>)
                {
                    return *m_it->second.first;
                }
                else if constexpr (std::is_same_v<ITERATOR, neighbor_iterator>)
                {
                    return *m_vertices.at(m_it->first).first;
                }
            }
            VERTEX_TYPE *operator->() const override
            {
                if constexpr (std::is_same_v<ITERATOR, vertex_iterator>)
                {
                    return m_it->second.first.get();
                }
                else if constexpr (std::is_same_v<ITERATOR, neighbor_iterator>)
                {
                    return m_vertices.at(m_it->first).first.get();
                }
            }

        private:
            ITERATOR m_it;
            std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                     std::map<uuid, float>>>
                m_vertices;
        };

        template <typename ITERATOR>
        class const_iterator : public base_const_iterator_impl
        {
        public:
            const_iterator(const ITERATOR &it, std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                                                        std::map<uuid, float>>>
                                                   vertices) : m_it{it}, m_vertices{vertices} {}
            bool operator==(const base_const_iterator_impl *other) const override { return m_it == static_cast<const const_iterator *>(other)->m_it; }
            bool operator!=(const base_const_iterator_impl *other) const override { return m_it != static_cast<const const_iterator *>(other)->m_it; }
            base_const_iterator_impl *operator++() override
            {
                if constexpr (std::is_same_v<ITERATOR, const_vertex_iterator>)
                {
                    ++m_it;
                    return this;
                }
                else if constexpr (std::is_same_v<ITERATOR, const_neighbor_iterator>)
                {
                    if (m_it->first == m_vertices.rbegin()->first)
                    {
                        ++m_it;
                        return this;
                    }
                    else
                    {
                        do
                        {
                            ++m_it;
                        } while (std::isnan(m_it->second));
                        return this;
                    }
                }
            }
            const VERTEX_TYPE &operator*() const override
            {
                if constexpr (std::is_same_v<ITERATOR, const_vertex_iterator>)
                {
                    return *m_it->second.first;
                }
                else if constexpr (std::is_same_v<ITERATOR, const_neighbor_iterator>)
                {
                    return *m_vertices.at(m_it->first).first;
                }
            }
            const VERTEX_TYPE *operator->() const override
            {
                if constexpr (std::is_same_v<ITERATOR, const_vertex_iterator>)
                {
                    return m_it->second.first.get();
                }
                else if constexpr (std::is_same_v<ITERATOR, const_neighbor_iterator>)
                {
                    return m_vertices.at(m_it->first).first.get();
                }
            }

        private:
            ITERATOR m_it;
            std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                     std::map<uuid, float>>>
                m_vertices;
        };
    };

    template <typename DATA_TYPE,
              template <typename> typename DATA_STRUCTURE = AdjacencyList>
    class Graph
    {
        using VERTEX_TYPE = Vertex<DATA_TYPE>;

        using base_const_iterator =
            typename DATA_STRUCTURE<DATA_TYPE>::base_const_iterator;

        using base_iterator = typename DATA_STRUCTURE<DATA_TYPE>::base_iterator;

        friend class VertexPrinter;

    public:
        Graph()
        {
            DATA_STRUCTURE<DATA_TYPE> data_structure;
            m_data_structure = std::make_shared<DATA_STRUCTURE<DATA_TYPE>>(data_structure);
        }
        ~Graph() = default;

        const uuid &add_vertex(VERTEX_TYPE &&vertex)
        {
            return m_data_structure->add_vertex(std::forward<VERTEX_TYPE>(vertex));
        }
        const uuid &add_vertex(VERTEX_TYPE &vertex)
        {
            return m_data_structure->add_vertex(std::move(vertex.get_data()));
        }
        const uuid &add_vertex(DATA_TYPE &&data)
        {
            return m_data_structure->add_vertex(std::forward<DATA_TYPE>(data));
        }
        const uuid &add_vertex(DATA_TYPE &data)
        {
            return m_data_structure->add_vertex(std::move(data));
        }
        void add_edge(const uuid &vertex1_id, const uuid &vertex2_id, const float weight = 0)
        {
            m_data_structure->add_edge(vertex1_id, vertex2_id, weight);
        }
        void remove_edge(const uuid &vertex1_id, const uuid &vertex2_id)
        {
            m_data_structure->remove_edge(vertex1_id, vertex2_id);
        }
        void remove_vertex(const uuid &vertex_id)
        {
            m_data_structure->remove_vertex(vertex_id);
        }

        template <typename FUNCTION, typename... ARGS>
        void remove_if(FUNCTION function, ARGS &&...args)
        {
            m_data_structure->remove_if(function, std::forward<ARGS>(args)...);
        }

        const VERTEX_TYPE &get_vertex(const uuid &id) const
        {
            return m_data_structure->get_vertex(id);
        }
        VERTEX_TYPE &get_vertex(const uuid &id) { return m_data_structure->get_vertex(id); }

        size_t size() const { return m_data_structure->size(); }
        size_t size(const uuid &id) const { return m_data_structure->size(id); }
        void empty() { m_data_structure->empty(); }

        std::ostream &print(std::ostream &os = std::cout) const
        {
            return m_data_structure->print(os);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(const uuid &id, FUNCTION function, ARGS &&...args)
        {
            m_data_structure->template traverse<ALGORITHM, policy>(
                function, id, std::forward<ARGS>(args)...);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(FUNCTION function, ARGS &&...args)
        {
            m_data_structure->template traverse<ALGORITHM, policy>(
                function, std::forward<ARGS>(args)...);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED>
        void traverse(const uuid &id)
        {
            m_data_structure->template traverse<ALGORITHM, policy>(id);
        }

        template <template <typename> typename ALGORITHM = BFS,
                  VisitPolicy policy = VisitPolicy::RELATED>
        void traverse()
        {
            m_data_structure->template traverse<ALGORITHM, policy>();
        }

        friend std::ostream &operator<<(std::ostream &os, const Graph &graph)
        {
            return os << graph.m_data_structure;
        }

    private:
        // DATA_STRUCTURE<DATA_TYPE> m_data_structure;
        std::shared_ptr<DATA_STRUCTURE<DATA_TYPE>> m_data_structure;

    public:
        base_const_iterator cbegin() const { return m_data_structure->cbegin(); }
        base_const_iterator cend() const { return m_data_structure->cend(); }
        base_iterator begin() { return m_data_structure->begin(); }
        base_iterator end() { return m_data_structure->end(); }

        base_const_iterator begin(const uuid &id) const { return m_data_structure->cbegin(id); }
        base_const_iterator end(const uuid &id) const { return m_data_structure->cend(id); }
        base_iterator begin(const uuid &id) { return m_data_structure->begin(id); }
        base_iterator end(const uuid &id) { return m_data_structure->end(id); }
    };

    template <typename DATA_STRUCTURE>
    class BFS
    {
        using VERTEX_TYPE = typename DATA_STRUCTURE::VERTEX_TYPE;

    public:
        BFS() = default;
        ~BFS() = default;

        template <VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(DATA_STRUCTURE &data_structure, const uuid &id,
                      FUNCTION &&function, ARGS &&...args)
        {
            if (data_structure.size() == 0)
            {
                throw std::out_of_range{"[void sgl::BFS::traverse(DATA_STRUCTURE &data_structure, const uuid &id, FUNCTION &&function, ARGS &&...args)] Graph is empty"};
            }

            std::queue<uuid> queue;
            std::set<uuid> visited;

            try
            {
                VERTEX_TYPE &start_vertex = data_structure.get_vertex(id);
                queue.push(start_vertex.get_id());
                visited.insert(start_vertex.get_id());
            }
            catch (const std::out_of_range &e)
            {
                throw std::out_of_range{"[void sgl::BFS::traverse(DATA_STRUCTURE &data_structure, const uuid &id, FUNCTION &&function, ARGS &&...args)] Vertex with id " + static_cast<std::string>(id) + " not found"};
            }

            while (!queue.empty())
            {

                VERTEX_TYPE &vertex = data_structure.get_vertex(queue.front());
                queue.pop();

                function(vertex, std::forward<ARGS>(args)...);

                for (auto it = data_structure.begin(vertex.get_id());
                     it != data_structure.end(vertex.get_id()); ++it)
                {
                    if (visited.find(it->get_id()) == visited.end())
                    {
                        queue.push(it->get_id());
                        visited.insert(it->get_id());
                    }
                }
            }

            if constexpr (policy)
            {
                for (auto it = data_structure.m_vertices.begin();
                     it != data_structure.m_vertices.end(); ++it)
                {
                    if (visited.find(it->first) == visited.end())
                    {
                        VERTEX_TYPE &vertex = data_structure.get_vertex(it->first);
                        function(vertex, std::forward<ARGS>(args)...);
                    }
                }
            }
        }
    };

    template <typename DATA_STRUCTURE>
    class DFS
    {
        using VERTEX_TYPE = typename DATA_STRUCTURE::VERTEX_TYPE;

    public:
        DFS() = default;
        ~DFS() = default;

        template <VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
                  typename... ARGS>
        void traverse(DATA_STRUCTURE &data_structure, const uuid &id,
                      FUNCTION &&function, ARGS &&...args)
        {
            if (data_structure.size() == 0)
            {
                throw std::out_of_range{"[void sgl::DFS::traverse(DATA_STRUCTURE &data_structure, const uuid &id, FUNCTION &&function, ARGS &&...args)] Graph is empty"};
            }

            std::stack<uuid> stack;
            std::set<uuid> visited;

            try
            {
                VERTEX_TYPE &start_vertex = data_structure.get_vertex(id);
                stack.push(start_vertex.get_id());
                visited.insert(start_vertex.get_id());
            }
            catch (const std::out_of_range &e)
            {
                throw std::out_of_range{"[void sgl::DFS::traverse(DATA_STRUCTURE &data_structure, const uuid &id, FUNCTION &&function, ARGS &&...args)] Vertex with id " + static_cast<std::string>(id) + " not found"};
            }

            while (!stack.empty())
            {
                VERTEX_TYPE &vertex = data_structure.get_vertex(stack.top());
                stack.pop();

                function(vertex, std::forward<ARGS>(args)...);

                for (auto it = data_structure.begin(vertex.get_id());
                     it != data_structure.end(vertex.get_id()); ++it)
                {
                    if (visited.find(it->get_id()) == visited.end())
                    {
                        stack.push(it->get_id());
                        visited.insert(it->get_id());
                    }
                }
            }

            if constexpr (policy)
            {
                for (auto it = data_structure.m_vertices.begin();
                     it != data_structure.m_vertices.end(); ++it)
                {
                    if (visited.find(it->first) == visited.end())
                    {
                        VERTEX_TYPE &vertex = data_structure.get_vertex(it->first);
                        function(vertex, std::forward<ARGS>(args)...);
                    }
                }
            }
        }
    };

    //
    //       END OF TYPE DEFINITIONS
    //
    //////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////
    //
    //       FUNCTORS
    //

    namespace func
    {
        auto add = std::bind([](auto &vertex, auto &&a)
                             { vertex.get_data() = vertex.get_data() + a; },
                             std::placeholders::_1, std::placeholders::_2);

        auto increment = std::bind([](auto &vertex)
                                   { vertex.get_data() = vertex.get_data() + 1; },
                                   std::placeholders::_1);

        auto decrement = std::bind([](auto &vertex)
                                   { vertex.get_data() = vertex.get_data() - 1; },
                                   std::placeholders::_1);

        auto multiply = std::bind([](auto &vertex, auto &&a)
                                  { vertex.get_data() = vertex.get_data() * a; },
                                  std::placeholders::_1, std::placeholders::_2);

        auto print = std::bind([](auto &vertex, const VertexFormat &format = VertexFormat::SHORTEST, std::ostream &os = std::cout)
                               { os << format << vertex << std::endl; },
                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        auto less_than = std::bind([](auto &vertex, auto &&a)
                                   { return vertex.get_data() < a; },
                                   std::placeholders::_1, std::placeholders::_2);

        auto greater_than = std::bind([](auto &vertex, auto &&a)
                                      { return vertex.get_data() > a; },
                                      std::placeholders::_1, std::placeholders::_2);

        auto equal_to = std::bind([](auto &vertex, auto &&a)
                                  { return vertex.get_data() == a; },
                                  std::placeholders::_1, std::placeholders::_2);

        auto not_equal_to = std::bind([](auto &vertex, auto &&a)
                                      { return vertex.get_data() != a; },
                                      std::placeholders::_1, std::placeholders::_2);

        auto less_than_or_equal_to = std::bind([](auto &vertex, auto &&a)
                                               { return vertex.get_data() <= a; },
                                               std::placeholders::_1, std::placeholders::_2);

        auto greater_than_or_equal_to = std::bind([](auto &vertex, auto &&a)
                                                  { return vertex.get_data() >= a; },
                                                  std::placeholders::_1, std::placeholders::_2);

        auto and_ = std::bind([](auto &vertex, auto &&a)
                              { return vertex.get_data() && a; },
                              std::placeholders::_1, std::placeholders::_2);

        auto or_ = std::bind([](auto &vertex, auto &&a)
                             { return vertex.get_data() || a; },
                             std::placeholders::_1, std::placeholders::_2);

        auto not_ = std::bind([](auto &vertex)
                              { return !vertex.get_data(); },
                              std::placeholders::_1);

        auto nand_ = std::bind([](auto &vertex, auto &&a)
                               { return !(vertex.get_data() && a); },
                               std::placeholders::_1, std::placeholders::_2);

        auto nor_ = std::bind([](auto &vertex, auto &&a)
                              { return !(vertex.get_data() || a); },
                              std::placeholders::_1, std::placeholders::_2);

        auto xor_ = std::bind([](auto &vertex, auto &&a)
                              { return vertex.get_data() ^ a; },
                              std::placeholders::_1, std::placeholders::_2);

        auto xnor_ = std::bind([](auto &vertex, auto &&a)
                               { return !(vertex.get_data() ^ a); },
                               std::placeholders::_1, std::placeholders::_2);
    }

    //
    //       END OF FUNCTORS
    //
    //////////////////////////////////////////////////////////////////////////////

} // namespace sgl

#endif // SGL_HH
