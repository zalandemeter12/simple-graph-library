#ifndef SGL_HH
#define SGL_HH

#include <any>
#include <functional>
#include <iostream>
#include <memory>

#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>

#include <boost/random/mersenne_twister.hpp> //random generator
#include <boost/uuid/uuid.hpp>               // uuid class
#include <boost/uuid/uuid_generators.hpp>    // generators
#include <boost/uuid/uuid_io.hpp>            // streaming operators etc.

namespace sgl
{

  template <template <typename...> typename T, typename U>
  inline constexpr bool is_specialization_of_v = false;

  template <template <typename...> typename T, typename... P>
  inline constexpr bool is_specialization_of_v<T, T<P...>> = true;

  void version()
  {
    std::cout << R"(
│ ╔═╗╔═╗╦      Simple Graph Library │ 
│ ╚═╗║ ╦║        Apache License 2.0 │
│ ╚═╝╚═╝╩═╝           version 0.1.0 │                            
        )" << std::endl;
  }

  template <typename DATA_TYPE>
  class Vertex;

  enum VertexFormat
  {
    SHORTEST,
    SHORT,
    LONG
  };

  class VertexPrinter;

  template <typename DATA_TYPE>
  class AdjacencyList;

  template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE>
  class Graph;

  enum VisitPolicy
  {
    RELATED,
    ALL
  };

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
    friend class DataStructureBase<DATA_TYPE>;

  public:
    Vertex(const DATA_TYPE &&data) : m_id{s_generator()}, m_data{data} {}
    Vertex(const DATA_TYPE &&data, DataStructureBase<DATA_TYPE> *m_data_structure)
        : m_id{s_generator()}, m_data{data}, m_data_structure{m_data_structure} {}

    ~Vertex() = default;

    const uuid &get_id() const { return m_id; }
    const DATA_TYPE &get_data() const { return m_data; }
    DATA_TYPE &get_data() { return m_data; }

    typename DataStructureBase<DATA_TYPE>::const_iterator cbegin() const { return m_data_structure->cbegin(m_id); }
    typename DataStructureBase<DATA_TYPE>::const_iterator cend() const { return m_data_structure->cend(m_id); }
    typename DataStructureBase<DATA_TYPE>::iterator begin() { return m_data_structure->begin(m_id); }
    typename DataStructureBase<DATA_TYPE>::iterator end() { return m_data_structure->end(m_id); }

    size_t size() const { return m_data_structure->size(m_id); }

    void remove()
    {
      if (m_data_structure == nullptr)
        throw std::runtime_error("SGL: Vertex is not part of a graph");

      m_data_structure->remove_vertex(m_id);
      m_data_structure = nullptr;
    }

    void remove_edge(const uuid &id)
    {
      if (m_data_structure == nullptr)
        throw std::runtime_error("SGL: Vertex is not part of a graph");

      m_data_structure->remove_edge(m_id, id);
    }

    friend std::ostream &operator<<(std::ostream &os, const VERTEX_TYPE &vertex)
    {
      std::string id = boost::uuids::to_string(vertex.m_id);
      return os << "[ " << vertex.m_data << " ]";
    }

  private:
    void add_data_structure(DataStructureBase<DATA_TYPE> *data_structure)
    {
      m_data_structure = data_structure;
    }

    const uuid m_id;
    DATA_TYPE m_data;
    DataStructureBase<DATA_TYPE> *m_data_structure = nullptr;

    static inline boost::random::mt19937 rnd = boost::random::mt19937{0};
    static inline boost::uuids::basic_random_generator<boost::random::mt19937>
        s_generator =
            boost::uuids::basic_random_generator<boost::random::mt19937>{rnd};
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
        std::string id = boost::uuids::to_string(vertex.m_id);
        m_os << "[ id: {" << id.substr(0, 2) << "..."
             << id.substr(id.size() - 2, 2) << "}, ";
        m_os << "data: {" << vertex.m_data;
        m_os << "}, size: {" << vertex.size() << "} ]";
      }
      else if (m_format == VertexFormat::LONG)
      {
        m_os << "[ id: {" << vertex.m_id << "}, ";
        m_os << "data: {" << vertex.m_data;
        m_os << "}, size: {" << vertex.size() << "} ]";
      }

      return m_os;
    }

    template <typename DATA_TYPE, template <typename> typename DATA_STRUCTURE>
    std::ostream &
    operator<<(const Graph<DATA_TYPE, DATA_STRUCTURE> &graph) const
    {
      if (m_format == VertexFormat::SHORTEST)
      {
        VertexPrinter{m_os, VertexFormat::SHORTEST} << graph.m_data_structure;
      }
      else if (m_format == VertexFormat::SHORT)
      {
        VertexPrinter{m_os, VertexFormat::SHORT} << graph.m_data_structure;
      }
      else if (m_format == VertexFormat::LONG)
      {
        VertexPrinter{m_os, VertexFormat::LONG} << graph.m_data_structure;
      }

      return m_os;
    }

    template <typename DATA_TYPE>
    std::ostream &
    operator<<(const AdjacencyList<DATA_TYPE> &adjacency_list) const
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

    virtual uuid add_vertex(VERTEX_TYPE &&vertex) = 0;
    virtual uuid add_vertex(const DATA_TYPE &&data) = 0;
    virtual void add_edge(const uuid &vertex1, const uuid &vertex2) = 0;

    virtual void remove_vertex(const uuid &vertex) = 0;
    virtual void remove_edge(const uuid &vertex1, const uuid &vertex2) = 0;

    virtual const VERTEX_TYPE &get_vertex(const uuid &id) const = 0;
    virtual VERTEX_TYPE &get_vertex(const uuid &id) = 0;

    virtual size_t size() const = 0;
    virtual size_t size(const uuid &id) const = 0;
    virtual void empty() = 0;

    virtual std::ostream &print(std::ostream &os = std::cout) const = 0;

  protected:
    virtual DataStructureBase<DATA_TYPE> *instance() { return this; }
    void add_data_structure(VERTEX_TYPE &vertex)
    {
      vertex.add_data_structure(instance());
    }

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

    virtual const_iterator cbegin(uuid id) const = 0;
    virtual const_iterator cend(uuid id) const = 0;
    virtual iterator begin(uuid id) = 0;
    virtual iterator end(uuid id) = 0;
  };

  template <typename DATA_TYPE>
  class AdjacencyList : public DataStructureBase<DATA_TYPE>
  {
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

  public:
    AdjacencyList() = default;
    ~AdjacencyList() = default;

    uuid add_vertex(VERTEX_TYPE &&vertex) override
    {
      this->add_data_structure(vertex);
      auto new_vertex = std::make_shared<VERTEX_TYPE>(std::move(vertex));
      auto id = new_vertex->get_id();
      m_vertices.insert(std::make_pair(
          std::move(id),
          std::make_pair(std::move(new_vertex),
                         std::vector<std::shared_ptr<VERTEX_TYPE>>())));
      return id;
    }

    uuid add_vertex(const DATA_TYPE &&data) override
    {
      VERTEX_TYPE vertex{std::move(data)};
      return add_vertex(std::move(vertex));
    }

    void add_edge(const uuid &vertex1, const uuid &vertex2) override
    {
      m_vertices[vertex1].second.push_back(m_vertices[vertex2].first);
      m_vertices[vertex2].second.push_back(m_vertices[vertex1].first);
    }

    void remove_vertex(const uuid &vertex) override
    {
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
        throw std::invalid_argument("SGL: return type of function must be bool");
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
      uuid id = m_vertices.begin()->first;
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
      uuid id = m_vertices.begin()->first;
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

  private:
    std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                             std::vector<std::shared_ptr<VERTEX_TYPE>>>>
        m_vertices;

  public:
    base_const_iterator cbegin() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cbegin())}; }
    base_const_iterator cend() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cend())}; }
    base_iterator begin() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.begin())}; }
    base_iterator end() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.end())}; }

    base_const_iterator cbegin(uuid id) const override { return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(m_vertices.at(id).second.cbegin())}; }
    base_const_iterator cend(uuid id) const override { return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(m_vertices.at(id).second.cend())}; }
    base_iterator begin(uuid id) override { return base_iterator{std::make_shared<iterator<neighbor_iterator>>(m_vertices.at(id).second.begin())}; }
    base_iterator end(uuid id) override { return base_iterator{std::make_shared<iterator<neighbor_iterator>>(m_vertices.at(id).second.end())}; }

  private:
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
  class AdjacencyMatrix : public DataStructureBase<DATA_TYPE>
  {
    using VERTEX_TYPE = Vertex<DATA_TYPE>;

    using base_iterator_impl =
        typename DataStructureBase<DATA_TYPE>::iterator_impl;

    using base_iterator = typename DataStructureBase<DATA_TYPE>::iterator;

    using base_const_iterator_impl =
        typename DataStructureBase<DATA_TYPE>::const_iterator_impl;

    using base_const_iterator =
        typename DataStructureBase<DATA_TYPE>::const_iterator;

    using const_neighbor_iterator =
        typename std::map<uuid, bool>::const_iterator;

    using neighbor_iterator =
        typename std::map<uuid, bool>::iterator;

    using const_vertex_iterator =
        typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>, std::map<uuid, bool>>>::const_iterator;

    using vertex_iterator =
        typename std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>, std::map<uuid, bool>>>::iterator;

    friend class BFS<AdjacencyMatrix<DATA_TYPE>>;
    friend class DFS<AdjacencyMatrix<DATA_TYPE>>;
    friend class Graph<DATA_TYPE, AdjacencyMatrix>;

  public:
    AdjacencyMatrix() = default;
    ~AdjacencyMatrix() = default;

    uuid add_vertex(VERTEX_TYPE &&vertex) override
    {
      this->add_data_structure(vertex);
      auto new_vertex = std::make_shared<VERTEX_TYPE>(std::move(vertex));
      auto id = new_vertex->get_id();

      m_vertices.insert(std::make_pair(id, std::make_pair(new_vertex, std::map<uuid, bool>{})));
      for (auto &v : m_vertices)
      {
        v.second.second.insert(std::make_pair(id, false));
        m_vertices.at(id).second.insert(std::make_pair(v.first, false));
      }

      return id;
    }

    uuid add_vertex(const DATA_TYPE &&data) override
    {
      VERTEX_TYPE vertex{std::move(data)};
      return add_vertex(std::move(vertex));
    }

    void add_edge(const uuid &vertex1, const uuid &vertex2) override
    {
      m_vertices.at(vertex1).second.at(vertex2) = true;
      m_vertices.at(vertex2).second.at(vertex1) = true;
    }

    void remove_vertex(const uuid &vertex) override
    {
      m_vertices.erase(vertex);
      for (auto &v : m_vertices)
      {
        v.second.second.erase(vertex);
      }
    }

    void remove_edge(const uuid &vertex1, const uuid &vertex2) override
    {
      m_vertices.at(vertex1).second.at(vertex2) = false;
      m_vertices.at(vertex2).second.at(vertex1) = false;
    }

    template <typename FUNCTION, typename... ARGS>
    void remove_if(FUNCTION &&function, ARGS &&...args)
    {
      if constexpr (!std::is_same_v<
                        bool, std::invoke_result_t<FUNCTION, const VERTEX_TYPE &,
                                                   ARGS...>>)
      {
        throw std::invalid_argument("SGL: return type of function must be bool");
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
                           { return pair.second; });
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
      uuid id = m_vertices.begin()->first;
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
      uuid id = m_vertices.begin()->first;
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

  private:
    std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                             std::map<uuid, bool>>>
        m_vertices;

  public:
    base_const_iterator cbegin() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cbegin(), m_vertices)}; }
    base_const_iterator cend() const override { return base_const_iterator{std::make_shared<const_iterator<const_vertex_iterator>>(m_vertices.cend(), m_vertices)}; }
    base_iterator begin() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.begin(), m_vertices)}; }
    base_iterator end() override { return base_iterator{std::make_shared<iterator<vertex_iterator>>(m_vertices.end(), m_vertices)}; }

    base_const_iterator cbegin(uuid id) const override
    {
      auto it = m_vertices.at(id).second.cbegin();
      while (it != m_vertices.at(id).second.cend() && !it->second)
      {
        ++it;
      }
      return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(it, m_vertices)};
    }
    base_const_iterator cend(uuid id) const override { return base_const_iterator{std::make_shared<const_iterator<const_neighbor_iterator>>(m_vertices.at(id).second.cend(), m_vertices)}; }
    base_iterator begin(uuid id) override
    {
      auto it = m_vertices.at(id).second.begin();
      while (it != m_vertices.at(id).second.end() && !it->second)
      {
        ++it;
      }
      return base_iterator{std::make_shared<iterator<neighbor_iterator>>(it, m_vertices)};
    }
    base_iterator end(uuid id) override { return base_iterator{std::make_shared<iterator<neighbor_iterator>>(m_vertices.at(id).second.end(), m_vertices)}; }

  private:
    template <typename ITERATOR>
    class iterator : public base_iterator_impl
    {
    public:
      iterator(const ITERATOR &it, std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                                            std::map<uuid, bool>>>
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
            } while (!m_it->second);
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
                               std::map<uuid, bool>>>
          m_vertices;
    };

    template <typename ITERATOR>
    class const_iterator : public base_const_iterator_impl
    {
    public:
      const_iterator(const ITERATOR &it, std::map<uuid, std::pair<std::shared_ptr<VERTEX_TYPE>,
                                                                  std::map<uuid, bool>>>
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
            } while (!m_it->second);
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
                               std::map<uuid, bool>>>
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
    Graph() = default;
    ~Graph() = default;

    uuid add_vertex(const VERTEX_TYPE &&vertex)
    {
      return m_data_structure.add_vertex(std::move(vertex));
    }
    uuid add_vertex(const DATA_TYPE &&data)
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

    template <typename FUNCTION, typename... ARGS>
    void remove_if(FUNCTION function, ARGS &&...args)
    {
      m_data_structure.remove_if(function, std::forward<ARGS>(args)...);
    }

    const VERTEX_TYPE &get_vertex(uuid id) const
    {
      return m_data_structure.get_vertex(id);
    }
    VERTEX_TYPE &get_vertex(uuid id) { return m_data_structure.get_vertex(id); }

    size_t size() const { return m_data_structure.size(); }
    size_t size(uuid id) const { return m_data_structure.size(id); }
    void empty() { m_data_structure.empty(); }

    std::ostream &print(std::ostream &os = std::cout) const
    {
      return m_data_structure.print(os);
    }

    template <template <typename> typename ALGORITHM = BFS,
              VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
              typename... ARGS>
    void traverse(const uuid &id, FUNCTION function, ARGS &&...args)
    {
      m_data_structure.template traverse<ALGORITHM, policy>(
          function, id, std::forward<ARGS>(args)...);
    }

    template <template <typename> typename ALGORITHM = BFS,
              VisitPolicy policy = VisitPolicy::RELATED, typename FUNCTION,
              typename... ARGS>
    void traverse(FUNCTION function, ARGS &&...args)
    {
      m_data_structure.template traverse<ALGORITHM, policy>(
          function, std::forward<ARGS>(args)...);
    }

    template <template <typename> typename ALGORITHM = BFS,
              VisitPolicy policy = VisitPolicy::RELATED>
    void traverse(const uuid &id)
    {
      m_data_structure.template traverse<ALGORITHM, policy>(id);
    }

    template <template <typename> typename ALGORITHM = BFS,
              VisitPolicy policy = VisitPolicy::RELATED>
    void traverse()
    {
      m_data_structure.template traverse<ALGORITHM, policy>();
    }

    friend std::ostream &operator<<(std::ostream &os, const Graph &graph)
    {
      return os << graph.m_data_structure;
    }

  private:
    DATA_STRUCTURE<DATA_TYPE> m_data_structure;

  public:
    base_const_iterator cbegin() const { return m_data_structure.cbegin(); }
    base_const_iterator cend() const { return m_data_structure.cend(); }
    base_iterator begin() { return m_data_structure.begin(); }
    base_iterator end() { return m_data_structure.end(); }

    base_const_iterator begin(uuid id) const { return m_data_structure.cbegin(id); }
    base_const_iterator end(uuid id) const { return m_data_structure.cend(id); }
    base_iterator begin(uuid id) { return m_data_structure.begin(id); }
    base_iterator end(uuid id) { return m_data_structure.end(id); }
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
        throw std::out_of_range{"SGL: Graph is empty"};
      }

      std::queue<uuid> queue;
      std::set<uuid> visited;

      try
      {
        VERTEX_TYPE start_vertex = data_structure.get_vertex(id);
        queue.push(start_vertex.get_id());
        visited.insert(start_vertex.get_id());
      }
      catch (const std::out_of_range &e)
      {
        throw std::out_of_range{"SGL: Vertex with id " +
                                boost::uuids::to_string(id) + " not found"};
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
        throw std::out_of_range{"SGL: Graph is empty"};
      }

      std::stack<uuid> stack;
      std::set<uuid> visited;

      try
      {
        VERTEX_TYPE start_vertex = data_structure.get_vertex(id);
        stack.push(start_vertex.get_id());
        visited.insert(start_vertex.get_id());
      }
      catch (const std::out_of_range &e)
      {
        throw std::out_of_range{"SGL: Vertex with id " +
                                boost::uuids::to_string(id) + " not found"};
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

  class Point2D
  {
  public:
    Point2D() = default;
    Point2D(const float x, const float y) : m_x{x}, m_y{y} {}
    Point2D(const float x, const float y, const std::any &payload)
        : m_x{x}, m_y{y}, m_payload{payload} {}
    ~Point2D() = default;

    float distanceFrom(Point2D p)
    {
      return sqrt(pow((abs(m_x - p.m_x)), 2) + pow(abs(m_y - p.m_y), 2));
    }
    void set_payload(const std::any &payload) { m_payload = payload; }

  private:
    float m_x;
    float m_y;
    std::any m_payload;
  };

  // travelling salesman problem solver with simulated annealing
  template <typename DATA_STRUCTURE>
  class TSP
  {
    using VERTEX_TYPE = typename DATA_STRUCTURE::VERTEX_TYPE;

  public:
    TSP() = default;
    ~TSP() = default;

    void solve(float T0, int nsteps)
    {
      auto swapping = [this](unsigned int idx1, unsigned int idx2)
      {
        std::vector<Point2D> tmp;
        for (unsigned int i = idx1 + 1; i < idx2 + 1; ++i)
          tmp.push_back(nodes.at(i));
        for (unsigned int i = idx2; i > idx1; --i)
          nodes.at(i) = tmp.at(-i + idx2);
      };

      auto E = [this]()
      {
        float dist_trav = 0;
        for (unsigned int i = 0; i < nodes.size() - 1; ++i)
          dist_trav += nodes.at(i).distanceFrom(nodes.at(i + 1));
        dist_trav += nodes.at(nodes.size() - 1).distanceFrom(nodes.at(0));
        return dist_trav;
      };

      if (nodes.size() == 0)
        return;

      srand(time(NULL));

      float oldE = E();

      std::vector<float> array;
      for (int i = 0; i < nsteps; ++i)
        array.push_back(T0 - (T0 / nsteps * i));

      for (float T : array)
      {
        unsigned int idx1 = rand() % nodes.size() - 2;
        unsigned int idx2 = rand() % (nodes.size() - (idx1 + 2)) +
                            idx1 + 2;
        swapping(idx1, idx2);
        float newE = E();
        if (newE > oldE)
        {
          if (static_cast<float>(rand()) / (static_cast<float>(RAND_MAX)) > exp(-(newE - oldE) / T))
          {
            swapping(idx1, idx2);
            newE = oldE;
          }
        }
        oldE = newE;
      }
    }

  private:
    std::vector<Point2D> nodes;
  };

  template <typename DATA_TYPE, typename PARAM_TYPE = DATA_TYPE>
  auto add = std::bind(
      [](Vertex<DATA_TYPE> &vertex, const PARAM_TYPE &&a)
      {
        vertex.get_data() = vertex.get_data() + a;
      },
      std::placeholders::_1, std::placeholders::_2);

  template <typename DATA_TYPE>
  auto increment = std::bind(
      [](Vertex<DATA_TYPE> &vertex)
      {
        vertex.get_data() = vertex.get_data() + 1;
      },
      std::placeholders::_1);

  template <typename DATA_TYPE>
  auto decrement = std::bind(
      [](Vertex<DATA_TYPE> &vertex)
      {
        vertex.get_data() = vertex.get_data() - 1;
      },
      std::placeholders::_1);

  template <typename DATA_TYPE, typename PARAM_TYPE = DATA_TYPE>
  auto multiply = std::bind(
      [](Vertex<DATA_TYPE> &vertex, const PARAM_TYPE &&a)
      {
        vertex.get_data() = vertex.get_data() * a;
      },
      std::placeholders::_1, std::placeholders::_2);

  template <typename DATA_TYPE, VertexFormat format = VertexFormat::SHORTEST,
            std::ostream &os = std::cout>
  auto print = std::bind(
      [](const Vertex<DATA_TYPE> &vertex)
      {
        os << format << vertex << std::endl;
      },
      std::placeholders::_1);

  template <typename DATA_TYPE, typename PARAM_TYPE = DATA_TYPE>
  auto less_than =
      std::bind([](const Vertex<DATA_TYPE> &vertex,
                   const PARAM_TYPE &&a)
                { return vertex.get_data() < a; },
                std::placeholders::_1, std::placeholders::_2);

  template <typename DATA_TYPE, typename PARAM_TYPE = DATA_TYPE>
  auto greater_than =
      std::bind([](const Vertex<DATA_TYPE> &vertex,
                   const PARAM_TYPE &&a)
                { return vertex.get_data() > a; },
                std::placeholders::_1, std::placeholders::_2);

  template <typename DATA_TYPE, typename PARAM_TYPE = DATA_TYPE>
  auto equal_to =
      std::bind([](const Vertex<DATA_TYPE> &vertex,
                   const PARAM_TYPE &&a)
                { return vertex.get_data() == a; },
                std::placeholders::_1, std::placeholders::_2);

} // namespace sgl

#endif // SGL_HH
