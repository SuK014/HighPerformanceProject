#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <bitset>
#include <numeric>
#include <bitset>
#include <future>

using namespace std;

// Maximum number of nodes
const int MAX_NODES = 100;

struct City{
    int num_nodes; // number of nodes
    vector<vector<int>> adj; 
    string result;
    // vector<bool> powered;
    vector<int> order;
    // The last adj for Safe check
    int last_chance[MAX_NODES];
    
    // optimal;
    atomic<int> minPlants;
    mutex mtx;
    int max_reach;

    // Bitset (Vectorization)
    bitset<MAX_NODES> node_masks[MAX_NODES];    
    bitset<MAX_NODES> suffix_coverage[MAX_NODES + 1];
    vector<int> max_potential_coverage;

    // Construction
    // City(int n): num_nodes(n) , adj(n), result(n,'0') , powered(0) , minPlants(n+1) {}
    City(int n): num_nodes(n) , adj(n), result(n,'0') , minPlants(n+1) {}

    // function : Add edge in adj
    void addEdge(int u , int v){
        if (u >= num_nodes || v >= num_nodes) return;
        adj[u].push_back(v); 
        adj[v].push_back(u);
    }

    // Pre-calculate the bitmask : insted of for loop the adj every time (Vectorization)
    void precomputeMasks() {
        for (int i = 0; i < num_nodes; ++i) {
            node_masks[i].reset();
            node_masks[i].set(i); // Power itself
            for (int neighbor : adj[i]) {
                node_masks[i].set(neighbor); // Power neighbor
            }
            
        }
    }
    


    // Backtracking
    void backtrack(bitset<MAX_NODES>& currentConfig, bitset<MAX_NODES>& current_coverage, int index, int currentCount, int powered_count) {
        // Pruning
        if (currentCount >= minPlants) return;

        // Lower bound
        // int uncovered = num_nodes - (int)current_coverage.count();
        // int uncovered = num_nodes - powered_count;
        // int allowed_more = minPlants - currentCount - 1; 
        
        // if (uncovered > allowed_more * max_reach) {
        //     return; 
        // }

        if ((current_coverage | suffix_coverage[index]).count() < (size_t)num_nodes) {
            return; 
        }

        int allowed_more = minPlants - currentCount - 1;
        if (allowed_more >= 0) {
            int max_index = min((int)order.size(), index + allowed_more);
            int max_possible_new_coverage = max_potential_coverage[max_index] - max_potential_coverage[index];
            if (num_nodes - powered_count > max_possible_new_coverage) return;
        }
        // Base case
        if ((int)current_coverage.count() == num_nodes) {
            // Due to using thread this make it not write at the same time
            lock_guard<mutex> lock(mtx);
            if (currentCount < minPlants) {
                minPlants = currentCount;
                // result = currentConfig; 
                string temp_result(num_nodes, '0');
                for (int i = 0; i < num_nodes; ++i) {
                    if (currentConfig.test(i)) {
                        temp_result[i] = '1';
                    }
                }

                result = temp_result;
                // cout << "Candidate Result : " << result << " | With number of nodes : " << minPlants << endl;
            }
            return;
        }

        if (index == order.size()) return;

        int node = order[index];


        // Choice 1 : Plant
        // currentConfig[node] = '1';
        currentConfig.set(node);
        auto next_coverage = current_coverage | node_masks[node];  // Using OR in bitmask (instead of the for loop update coverage) (Vectorization)
        if (next_coverage != current_coverage) { // check if the plant help cover more city or not
            // currentConfig[node] = '1';
            backtrack(currentConfig, next_coverage, index + 1, currentCount + 1, (int)next_coverage.count());
        }
        // Check progression
        // cout << "Current Progression (Plant):" << currentConfig << endl;

        // Choice 2 : NOT Plant
        // currentConfig[node] = '0';
        currentConfig.reset(node);
        // Safe Check : if there aren't any future neighbor to be plant it will not go this way
        if (current_coverage.test(node) || index < last_chance[node]) {
            backtrack(currentConfig, current_coverage, index + 1, currentCount, powered_count);
            // Check progression
            // cout << "Current Progression (Not plant):" << currentConfig << endl;
        }


    }
    void forceLeaves(bitset<MAX_NODES>& current, bitset<MAX_NODES>& coverage, int& count) {
        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 0; i < num_nodes; i++) {
                // If leaf and not yet covered
                if (adj[i].size() == 1 && !coverage.test(i)) {
                    int parent = adj[i][0];
                    if (!current.test(parent)) {
                        current.set(parent);
                        coverage |= node_masks[parent];
                        count++;
                        changed = true;
                    }
                }
            }
        }
    }

    


    void solveByGreedy(){
        // turn adj into coverage bitmask
        precomputeMasks();
        // check bottleneck
        cout << "finish percompute" <<endl;

        int max_deg = 0;
        for(int i=0; i<num_nodes; i++) max_deg = max(max_deg, (int)adj[i].size());
        max_reach = max_deg + 1;

        // string current = string(num_nodes, '0');
        // (Vectorization)
        bitset<MAX_NODES> current;
        bitset<MAX_NODES> initial_coverage;
        int pre_planted_count = 0;
        vector<int> remaining_order;

        
        // for (int i = 0; i < num_nodes; i++) {
        //     if (adj[i].empty()) {
        //         // current[i] = '1';
        //         current.set(i);
        //         initial_coverage.set(i);
        //         pre_planted_count++;
        //     } else {
        //         remaining_order.push_back(i);
        //     }
        // }
        // NEW
        for (int i = 0; i < num_nodes; i++) {
            if (adj[i].empty()) {
                current.set(i);
                initial_coverage.set(i);
                pre_planted_count++;
            }
        }

        // Force plant parents of all leaves
        forceLeaves(current, initial_coverage, pre_planted_count);

        // Only backtrack on nodes not yet decided
        for (int i = 0; i < num_nodes; i++) {
            if (!adj[i].empty() && !current.test(i)) {
                remaining_order.push_back(i);
            }
        }
        
        sort(remaining_order.begin(), remaining_order.end(), [&](int a, int b) {
            return adj[a].size() > adj[b].size();
        });

        // check bottleneck
        cout << "finish sort" <<endl;

        // Upper bound
        bitset<MAX_NODES> greedy_cov = initial_coverage;
        bitset<MAX_NODES> greedy_planted = current;
        int greedy_count = pre_planted_count;

        while (greedy_cov.count() < (size_t)num_nodes) {
            int best = -1, best_gain = 0;
            for (int node : remaining_order) {
                if (greedy_planted.test(node)) continue;
                int gain = (node_masks[node] & ~greedy_cov).count();
                if (gain > best_gain) {
                    best_gain = gain;
                    best = node;
                }
            }
            if (best == -1) break;
            greedy_planted.set(best);
            greedy_cov |= node_masks[best];
            greedy_count++;
        }

        minPlants = greedy_count;
        result = string(num_nodes, '0');
        for (int i = 0; i < num_nodes; i++)
            if (greedy_planted.test(i)) result[i] = '1';
        // if (initial_coverage.count() == (size_t)num_nodes) {
        //     result = string(num_nodes, '0');
        //     for (int i = 0; i < num_nodes; i++)
        //         if (current.test(i)) result[i] = '1';
        //     minPlants = pre_planted_count;
        //     cout << "Final Result : " << result << endl;
        //     return;
        // }

        // minPlants = pre_planted_count + (int)remaining_order.size();
      
        
        order = remaining_order;

        vector<int> pos_in_order(num_nodes, -1);
        for (int i = 0; i < (int)order.size(); i++) {
            pos_in_order[order[i]] = i;
        }

        for (int i = 0; i < num_nodes; i++) {
            int latest = pos_in_order[i]; 
            for (int neighbor : adj[i]) {
                // latest = max(latest, pos_in_order[neighbor]);
                int npos = pos_in_order[neighbor];
                if (npos != -1)  // only consider undecided neighbors
                    latest = max(latest, npos);
            }
            last_chance[i] = latest;
        }

        // check bottleneck
        cout << "finish prepare graph" <<endl;

        max_potential_coverage.assign(order.size() + 1, 0);
        for (size_t i = 0; i < order.size(); i++) {
            max_potential_coverage[i + 1] = max_potential_coverage[i] + adj[order[i]].size() + 1;
        }

        suffix_coverage[order.size()].reset(); 
        for (int i = (int)order.size() - 1; i >= 0; --i) {
            suffix_coverage[i] = suffix_coverage[i + 1] | node_masks[order[i]];
        }

        // check bottleneck
        cout << "finish pre compute" <<endl;

        if (order.empty()) {
            // result = current;
            string temp_result(num_nodes, '0');
            for(int i=0; i<num_nodes; ++i) if(current.test(i)) temp_result[i] = '1';
            result = temp_result;
            minPlants = pre_planted_count;
            cout << "Final Result : " << result << endl;
            return; // Exit early, no need for threads!
        }

        // Separate in to 8 Thread Version
        int depth = min(3, (int)order.size());
        int num_tasks = 1 << depth;
        vector<future<void>> tasks;

        for (int mask = 0; mask < num_tasks; ++mask) {
            tasks.push_back(async(launch::async, [this, mask, depth, current, initial_coverage, pre_planted_count]() mutable {
                int local_count = pre_planted_count;
                bitset<MAX_NODES> local_current = current;
                bitset<MAX_NODES> local_cov = initial_coverage;
                
                bool valid = true;
                for (int i = 0; i < depth; ++i) {
                    int node = order[i];
                    // Bitwise check: is the i-th bit 1 (Plant) or 0 (Not Plant)?
                    bool plant = (mask & (1 << (depth - 1 - i))) != 0;
                    
                    if (plant) {
                        local_current.set(node);
                        local_cov |= node_masks[node];
                        local_count++;
                    } else {
                        local_current.reset(node);
                        // Safe check simulation
                        if (!local_cov.test(node) && i >= last_chance[node]) {
                            valid = false; // Invalid branch, abort early
                            break;
                        }
                    }
                }
                
                if (valid) {
                    // Pass the thread's local bitsets by reference into the deep recursion
                    this->backtrack(local_current, local_cov, depth, local_count, (int)local_cov.count());
                }
            }));
        }

        for (auto &t : tasks) {
            t.get();
        }

        cout << "Final Result : " << result << endl;
    }


};


int main(int argc, char* argv[]) {
    // WARNING : Usage 
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }
    
    // Start Timer
    auto start_time = chrono::high_resolution_clock::now();

    // WARNING : file path 
    ifstream infile(argv[1]);
    if (!infile) {
        cerr << "Could not open file: " << argv[1] << endl;
        return 1;
    }

    // Declare number of nodes and edges of graph 
    int n, e;
    infile >> n >> e;
    City city(n);


    // Declare edges connection
    for (int i = 0; i < e; ++i) {
        int u, v;
        infile >> u >> v;
        city.addEdge(u, v);
    }

    // Close file
    infile.close();
    
    // execute Logic
    city.solveByGreedy();
    
    // Print out result
    cout << "Number of Plants :" << city.minPlants << endl;

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end_time - start_time;

    cerr << "Execution time: " << diff.count() << " seconds" << endl;

    // Write in output file
    if (argc >= 3) {
        ofstream outfile(argv[2]);
        outfile << city.result << "\n";
        outfile.close();
    }

    cout << "RESULT:" << city.result << "\n";

    return 0;
}