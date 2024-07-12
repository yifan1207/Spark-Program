#include "initial_mapping.h"

double Initial_Mapping::ALPHA = 0.55;
double Initial_Mapping::BETA = 4;
double Initial_Mapping::GAMMA = 1;

Initial_Mapping::Initial_Mapping(CircuitGraph *CG, ChipTopology *AG) {
    m_best_map_available = false;
    CANDI.resize(CG->num_qubit_logical);
    CANDI[0] = AG->num_qubit;
    CANDI[1] = 4;
    for(int i = 2; i < min(8, CG->num_qubit_logical); i++) CANDI[i] = 2;
    for(int i = 8; i < CG->num_qubit_logical; i++) CANDI[i] = 1;

    this->m_p_CG = CG;
    this->m_p_AG = AG;
}

void Initial_Mapping::compute_initial_mapping() {
    // get depth & degree
    std::vector<int> depth;
    std::vector<double> weight;
    depth.resize(m_p_CG->node_cnt);
    m_tmp_in_degree.resize(m_p_CG->node_cnt);
    for(int i = 0, v; i < m_p_CG->node_cnt; i++) {
        m_tmp_in_degree[i] = boost::in_degree(this->m_p_CG->vertex_set()[i], *this->m_p_CG);
        if(!m_tmp_in_degree[i]) m_tmp_front_layer.push_back(i); 
        for (auto e: boost::make_iterator_range(boost::out_edges(m_p_CG->vertex_set()[i], *this->m_p_CG))) {
            v = boost::target(e, *this->m_p_CG);
            depth[v] = std::max(depth[v], depth[i] + 1);
        }
    }
    // get weight
    std::vector<int> pw;
    pw.resize(m_p_CG->node_cnt);
    pw[0] = 1e6;
    for(int i = 1; i < m_p_CG->node_cnt; i++) pw[i] = pw[i-1] * ALPHA;
    weight.resize(m_p_CG->num_qubit_logical);
    for(int x = 0; x < m_p_CG->num_qubit_logical; x++) {
        for(auto v: m_p_CG->gate_for_qubit[x]) {
            weight[x] += pw[depth[v]];
        }
    }
    // for(int x = 0; x < CG->num_qubit_logical; x++) std::cerr << weight[x] << " "; std::cerr << "weight\n";
    // get rank
    m_tmp_c.resize(m_p_CG->num_qubit_logical);
    for(int i = 0; i < m_p_CG->num_qubit_logical; i++) m_tmp_c[i] = i; 
    sort(m_tmp_c.begin(), m_tmp_c.end(), [&](int a, int b) {
        return weight[a] > weight[b];
    });
    // for(int x = 0; x < CG->num_qubit_logical; x++) std::cerr << c[x] << " "; std::cerr << "rank\n";

    // dfs
    m_tmp_valid.resize(m_p_AG->num_qubit);
    m_tmp_log_to_phy.resize(m_p_AG->num_qubit);
    m_tmp_phy_to_log.resize(m_p_AG->num_qubit);
    m_best_map.resize(m_p_AG->num_qubit);
    m_tmp_que.resize(m_p_CG->num_qubit_logical);
    for(int i = 0; i < m_p_AG->num_qubit; i++) {
        m_tmp_valid[i] = 1;
        m_best_map[i] = m_tmp_log_to_phy[i] = m_tmp_phy_to_log[i] = i;
    }
    m_best_h = cost_h();
    for(int i = 0; i < m_p_AG->num_qubit; i++) m_tmp_valid[i] = 0;
    dfs(0);
}

std::vector<int> Initial_Mapping::get_initial_mapping(){
    if(!m_best_map_available) {
        compute_initial_mapping();
        m_best_map_available = true;
    }
    return m_best_map;
}

void Initial_Mapping::update() {
    double ret = cost_h();
    if(ret > m_best_h) {
        m_best_h = ret;
        m_best_map = m_tmp_log_to_phy;
    }
}

void Initial_Mapping::swap_map(int x, int y) { // logical
    if(x == y) return;
    swap(m_tmp_log_to_phy[x], m_tmp_log_to_phy[y]);
    m_tmp_phy_to_log[m_tmp_log_to_phy[x]] = x;
    m_tmp_phy_to_log[m_tmp_log_to_phy[y]] = y;
}

std::vector<int> Initial_Mapping::find_exe_gate(){
    std::vector<int> exe_gate, trash;
    queue<int> q;
    for(auto x: m_tmp_front_layer) q.push(x);
    while(!q.empty()) {
        int x = q.front(), v; q.pop();
        int q_from = m_tmp_log_to_phy[boost::get(&BinaryGate::m_fromqubit, *this->m_p_CG)[x]];
        int q_to = m_tmp_log_to_phy[boost::get(&BinaryGate::m_toqubit, *this->m_p_CG)[x]];
        if(!m_tmp_valid[q_from] || !m_tmp_valid[q_to] || m_p_AG->shortest_path_AG(q_from, q_to).first.size() != 2) continue;
        exe_gate.push_back(x);
        for (auto e : boost::make_iterator_range(boost::out_edges(m_p_CG->vertex_set()[x], *this->m_p_CG))) {
            v = boost::target(e, *this->m_p_CG);
            --m_tmp_in_degree[v];
            trash.push_back(v);
            if(!m_tmp_in_degree[v]) q.push(v);
        }
    }
    for(auto x: trash) ++m_tmp_in_degree[x];
    return exe_gate;
}

double Initial_Mapping::cost_h() {
    double pw, ret = 1.0 * (signed)find_exe_gate().size() * (m_p_AG->num_qubit) * BETA;
    for(int x = 0, q_from, q_to; x < m_p_CG->num_qubit_logical; x++) {
        pw = 1;
        for(auto y: m_p_CG->gate_for_qubit[x]){
            q_from = m_tmp_log_to_phy[boost::get(&BinaryGate::m_fromqubit, *this->m_p_CG)[y]];
            q_to = m_tmp_log_to_phy[boost::get(&BinaryGate::m_toqubit, *this->m_p_CG)[y]];
            ret += pw * (GAMMA - ((!m_tmp_valid[q_from] || !m_tmp_valid[q_to]) ? m_p_AG->diameter : ((signed)m_p_AG->shortest_path_AG(q_from, q_to).first.size() - 1)));
            pw *= ALPHA;
            if(pw < 1e-6) break;
        }
    }
    return ret;
}

void Initial_Mapping::dfs(int pos) {
    if(pos == m_p_CG->num_qubit_logical) {
        update();
        return;
    }
    for(int i = 0; i < m_p_AG->num_qubit; i++) if(!m_tmp_valid[i]) {
        swap_map(m_tmp_c[pos], m_tmp_phy_to_log[i]);
        m_tmp_valid[i] = 1;
        m_tmp_que[pos].push(make_pair(-cost_h(), i));
        while((signed)m_tmp_que[pos].size() > CANDI[pos]) m_tmp_que[pos].pop();
        m_tmp_valid[i] = 0;
    }
    while(!m_tmp_que[pos].empty()){
        int x = m_tmp_que[pos].top().second; m_tmp_que[pos].pop();
        swap_map(m_tmp_c[pos], m_tmp_phy_to_log[x]);
        m_tmp_valid[x]=1;
        dfs(pos+1);
        m_tmp_valid[x]=0;
    }
}

int main() {
    return 0;
}