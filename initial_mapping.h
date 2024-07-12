
#include <utility>                   
#include <algorithm>                
#include <queue>
#include <bitset>
#include <boost/graph/undirected_graph.hpp>
#include <boost/variant/get.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>


using namespace std;
const float ROUNDING_ERROR_f32 = 0.000001f;

inline bool equals(const float a, const float b, const float tolerance = ROUNDING_ERROR_f32);


class ChipTopology{
private:
public:
    ChipTopology();  
    ~ChipTopology();
    pair<std::vector<int>, float> shortest_path_AG(int source, int destination) {std::vector<int> tmp; tmp.push_back(0); tmp.push_back(1);return make_pair(tmp,1.0);};
    std::vector<std::vector<pair<std::vector<int>, float>>> shortest_path_AG_map;
    int num_qubit = 2;
    int diameter = 3;
};


struct CircuitGate{
    std::string gate_name;
    int num_qubit;
    std::vector<int> logic_qubit_list;
    std::vector<int> virtual_qubit_list;
    std::vector<int> physic_qubit_list;
    std::vector<float> experssion_list;
};

class QubitData{
public:
  std::string vertex_name;
  int num;
};


class Gate{
 public:
};

class UnaryGate : public Gate{
 public:
    std::string gate_name;
    int m_qubit;
    std::vector<float> expression_list;
};

class BinaryGate : public Gate{
 public:
    std::string gate_name;
    int circuit_list_index;
    int m_fromqubit;
    int m_toqubit;
    vector<UnaryGate*> m_from_followings;
    vector<UnaryGate*> m_to_followings;
    int circuit_graph_index;
    std::vector<float> expression_list;
};


class EdgeData{
 public:
  std::string edge_name;
  double dist;
};

class CircuitGraph:public boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, BinaryGate, EdgeData>{
 private:
 public:
    CircuitGraph(ChipTopology* AG, std::vector<CircuitGate> circuit_gate_list);
    ~CircuitGraph();
    int num_qubit_logical;
    std::vector<int> first_nodes;
    int node_cnt;
    std::vector<UnaryGate*> single_gates_before_first_layer;
    std::vector<std::vector<int> > gate_for_qubit;
private:
};



class Initial_Mapping{
   public:
      Initial_Mapping(CircuitGraph* CG, ChipTopology* AG);
      virtual ~Initial_Mapping(){}
   
   public:
      std::vector<int> get_initial_mapping();

   protected:
      void compute_initial_mapping();

   private:
      std::vector<int> CANDI;
      static double ALPHA, BETA, GAMMA;

   protected:
      CircuitGraph* m_p_CG;
      ChipTopology* m_p_AG;
      double m_best_h;
      std::vector<int> m_best_map;
      bool m_best_map_available;

   private:
      std::vector<int> m_tmp_log_to_phy;
      std::vector<int> m_tmp_phy_to_log;
      std::vector<int> m_tmp_valid;
      std::vector<std::priority_queue<std::pair<double, int> > > m_tmp_que;
      std::vector<int> m_tmp_c;
      std::vector<int> m_tmp_in_degree;
      std::vector<int> m_tmp_front_layer;

      void dfs(int pos);
      void update();
      void swap_map(int x, int y);
      std::vector<int> find_exe_gate();
      double cost_h();
};



