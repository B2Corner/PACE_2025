/*
   Copyright 2025 Viacheslav Khrushchev

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <queue>
#include <cassert>
#include <sstream>
#include <signal.h>

volatile sig_atomic_t need_to_stop = 0;

void sigterm_handler(int dummy) {
    need_to_stop = 1;
}

std::vector<int32_t> get_upper_bound(int32_t n, std::vector<int32_t>* graph, int32_t seed) {
    bool* can_be_in_ans = new bool[n];
    for(int32_t i = 0; i < n; i++)
        can_be_in_ans[i] = true;
    bool* in_graph = new bool[n];
    for(int32_t i = 0; i < n; i++)
        in_graph[i] = false;
    for(int32_t i = 0; i < n; i++) {
        if(!can_be_in_ans[i])
            continue;

        for(int32_t k = 0; k < graph[i].size(); k++)
            in_graph[graph[i][k]] = true;
        in_graph[i] = true;
        for(int32_t v : graph[i]) {
            if(graph[v].size() > graph[i].size())
                continue;
            bool all_in_i = true;
            for(int32_t k = 0; k < graph[v].size(); k++)
                if(!in_graph[graph[v][k]]) {
                    all_in_i = false;
                    break;
                }
            if(all_in_i)
                can_be_in_ans[v] = false;
        }
        for(int32_t k = 0; k < graph[i].size(); k++)
            in_graph[graph[i][k]] = false;
        in_graph[i] = false;
    }
    
    bool* in_ans = new bool[n];
    for(int32_t i = 0; i < n; i++)
        in_ans[i] = can_be_in_ans[i];

    int32_t* selected_neighbors = new int32_t[n];
    int64_t* sum_selected_neighbors = new int64_t[n];
    for(int32_t i = 0; i < n; i++) {
        selected_neighbors[i] = in_ans[i] ? 1 : 0;
        sum_selected_neighbors[i] = in_ans[i] ? i : 0;
        for(int32_t j = 0; j < graph[i].size(); j++)
            if(in_ans[graph[i][j]]) {
                selected_neighbors[i]++;
                sum_selected_neighbors[i] += graph[i][j];
            }
    }

    int32_t* selected_neighbors_dominated_once = new int32_t[n];
    for(int32_t i = 0; i < n; i++) {
        if(!in_ans[i]) {
            selected_neighbors_dominated_once[i] = -1;
            continue;
        }
        selected_neighbors_dominated_once[i] = (selected_neighbors[i] == 1) ? 1 : 0;
        for(int32_t v : graph[i])
            if(selected_neighbors[v] == 1)
                selected_neighbors_dominated_once[i]++;
    }

    int32_t ans_size = 0;
    for(int32_t i = 0; i < n; i++)
        if(in_ans[i])
            ans_size++;
    int32_t best_ans_size = ans_size;
    
    int32_t till_reset = 3;
    std::queue<int64_t> last_improvements;
    int64_t since_last_divisor_increase = 0;
    int32_t divisor = 100;
    
    std::vector<int32_t> new_removable;
    for(int32_t i = 0; i < n; i++)
        if(in_ans[i])
            new_removable.push_back(i);

    auto remove_vertex = [&](int32_t v) {
        in_ans[v] = false;
        ans_size--;
        selected_neighbors_dominated_once[v] = -1;
        for(int32_t i = 0; i < graph[v].size(); i++) {
            selected_neighbors[graph[v][i]]--;
            sum_selected_neighbors[graph[v][i]] -= v;
            if(selected_neighbors[graph[v][i]] == 1)
                selected_neighbors_dominated_once[sum_selected_neighbors[graph[v][i]]]++;
        }
        selected_neighbors[v]--;
        sum_selected_neighbors[v] -= v;
        if(selected_neighbors[v] == 1)
            selected_neighbors_dominated_once[sum_selected_neighbors[v]]++;
    };

    std::vector<int32_t> actions_wrt_best;
    auto undo_actions_wrt_best = [&]() {
        for(int32_t j = actions_wrt_best.size() - 1; j >= 0; j--)
            if(actions_wrt_best[j] > 0) {
                int32_t v2 = actions_wrt_best[j] - 1;
                remove_vertex(v2);
            } else {
                int32_t v2 = -actions_wrt_best[j] - 1;
                in_ans[v2] = true;
                ans_size++;
                selected_neighbors_dominated_once[v2] = 0;
                for(int32_t i = 0; i < graph[v2].size(); i++) {
                    selected_neighbors[graph[v2][i]]++;
                    sum_selected_neighbors[graph[v2][i]] += v2;
                    if(selected_neighbors[graph[v2][i]] == 2)
                        selected_neighbors_dominated_once[sum_selected_neighbors[graph[v2][i]] - v2]--;
                }
                selected_neighbors[v2]++;
                sum_selected_neighbors[v2] += v2;
                if(selected_neighbors[v2] == 2)
                    selected_neighbors_dominated_once[sum_selected_neighbors[v2] - v2]--;
            }
        actions_wrt_best.clear();
    };

    std::mt19937 rng(seed);
    for(int64_t t = 0; ; t++) {
        if(need_to_stop > 0)
            break;

        if(t % 5 == 0)
            std::shuffle(new_removable.begin(), new_removable.end(), rng);
        for(int32_t v_ind = 0; v_ind < new_removable.size(); v_ind++) {
            int32_t v = new_removable[v_ind];
            assert(in_ans[v]);

            bool can_remove = (selected_neighbors_dominated_once[v] == 0);
            if(can_remove) {
                remove_vertex(v);
                actions_wrt_best.push_back(-(v + 1));
                continue;
            }
        }
        new_removable.clear();

        if(ans_size <= best_ans_size) {
            best_ans_size = ans_size;
            actions_wrt_best.clear();
            till_reset = 3;
            last_improvements.push(t);
        } else if((--till_reset) == 0) {
            undo_actions_wrt_best();
            till_reset = 3;
        }

        since_last_divisor_increase++;
        if(!last_improvements.empty() && last_improvements.front() < t - 100)
            last_improvements.pop();
        if(divisor < 2000 && since_last_divisor_increase > 100 && last_improvements.size() < 50) {
            divisor += 25;
            since_last_divisor_increase = 0;
        }
        
        for(int32_t i = 0; i < std::max(n / divisor, 10); i++) {
            int32_t v2 = rng() % n;
            if(in_ans[v2] || !can_be_in_ans[v2])
                continue;
            
            in_ans[v2] = true;
            ans_size++;
            for(int32_t i = 0; i < graph[v2].size(); i++) {
                selected_neighbors[graph[v2][i]]++;
                sum_selected_neighbors[graph[v2][i]] += v2;
                if(selected_neighbors[graph[v2][i]] == 2) {
                    selected_neighbors_dominated_once[sum_selected_neighbors[graph[v2][i]] - v2]--;
                    if(selected_neighbors_dominated_once[sum_selected_neighbors[graph[v2][i]] - v2] == 0)
                        new_removable.push_back(sum_selected_neighbors[graph[v2][i]] - v2);
                }
            }
            selected_neighbors[v2]++;
            sum_selected_neighbors[v2] += v2;
            if(selected_neighbors[v2] == 2) {
                selected_neighbors_dominated_once[sum_selected_neighbors[v2] - v2]--;
                if(selected_neighbors_dominated_once[sum_selected_neighbors[v2] - v2] == 0)
                    new_removable.push_back(sum_selected_neighbors[v2] - v2);
            }
            selected_neighbors_dominated_once[v2] = 0;
            assert(selected_neighbors[v2] >= 2);
            new_removable.push_back(v2);
            actions_wrt_best.push_back(v2 + 1);
        }
    }
    
    undo_actions_wrt_best();

    // Check the answer
    for(int32_t i = 0; i < n; i++) {
        bool ok = in_ans[i];
        for(int32_t x : graph[i])
            if(in_ans[x])
                ok = true;
        if(!ok) {
            std::cerr << "Upper bound verification failed" << std::endl;
            exit(1);
        }
    }

    std::vector<int32_t> res;
    for(int32_t i = 0; i < n; i++)
        if(in_ans[i])
            res.push_back(i);
    assert(res.size() == best_ans_size);
    return res;
}

int main(int argc, char** argv) {
    struct sigaction sigaction_struct;
    sigaction(SIGTERM, NULL, &sigaction_struct);
    sigaction_struct.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &sigaction_struct, NULL);
    
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);

    int32_t n, m;
    std::string str;
    while(std::getline(std::cin, str)) {
        if(str[0] == 'c')
            continue;

        std::stringstream string_stream(str);
        std::string dummy;
        string_stream >> dummy;
        string_stream >> dummy >> n >> m;
        break;
    }
            
    std::vector<int32_t>* graph = new std::vector<int32_t>[n];
    for(int32_t i = 0; i < m; i++) {
        int32_t v1, v2;
        std::cin >> v1 >> v2;
        v1--;
        v2--;
        
        graph[v1].push_back(v2);
        graph[v2].push_back(v1);
    }

    std::vector<int32_t> solution = get_upper_bound(n, graph, 0);
    std::cout << solution.size() << "\n";
    for(int32_t i = 0; i < solution.size(); i++)
        std::cout << solution[i] + 1 << "\n";
}

