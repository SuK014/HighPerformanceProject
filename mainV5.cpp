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
        if (current_coverage.all() || current_coverage.count() == (size_t)num_nodes) {
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

        
        for (int i = 0; i < num_nodes; i++) {
            if (adj[i].empty()) {
                // current[i] = '1';
                current.set(i);
                initial_coverage.set(i);
                pre_planted_count++;
            } else {
                remaining_order.push_back(i);
            }
        }
        
        sort(remaining_order.begin(), remaining_order.end(), [&](int a, int b) {
            return adj[a].size() > adj[b].size();
        });

        // for (int i = 0; i < num_nodes; i++) {
        //     auto adjacent = adj[i];
        //     if (adj[i].empty() && !initial_coverage.test(i)) {
        //         current[i] = '1';
        //         initial_coverage |= node_masks[i]; 
        //         pre_planted_count++;
        //     }
        //     else if (adj[i].size() == 1 && !initial_coverage.test(i)) {
        //         int neighbor = adj[i][0];
        //         if (current[neighbor] != '1') { 
        //             current[neighbor] = '1';
        //             initial_coverage |= node_masks[neighbor]; 
        //             pre_planted_count++;
        //         }
        //     }
        // }

        // for (int i = 0; i < num_nodes; i++) {
        //     if (!initial_coverage.test(i)) {
        //         remaining_order.push_back(i);
        //     }
        // }
        
      
        // check bottleneck
        cout << "finish sort" <<endl;

        order = remaining_order;

        vector<int> pos_in_order(num_nodes, -1);
        for (int i = 0; i < (int)order.size(); i++) {
            pos_in_order[order[i]] = i;
        }

        for (int i = 0; i < num_nodes; i++) {
            int latest = pos_in_order[i]; 
            for (int neighbor : adj[i]) {
                latest = max(latest, pos_in_order[neighbor]);
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

        // Separate in to 4 Thread Version
        int depth = min(8, (int)order.size());
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
        // int node0 = order[0];
        // int node1 = order[1];

        // vector<future<void>> tasks;

        // // Plant both
        // tasks.push_back(async(launch::async, [this, current, initial_coverage, pre_planted_count, node0, node1]() mutable {
        //     // current[node0] = '1';
        //     // current[node1] = '1';
        //     current.set(node0); 
        //     current.set(node1);
        //     auto cov = initial_coverage | node_masks[node0] | node_masks[node1];
        //     this->backtrack(current, cov, 2, pre_planted_count + 2, (int)cov.count());
        // }));
        // // Plant only first one
        // tasks.push_back(async(launch::async, [this, current, initial_coverage, pre_planted_count, node0, node1]() mutable {
        //     // current[node0] = '1';
        //     // current[node1] = '0';
        //     current.set(node0); 
        //     current.reset(node1);
        //     auto cov = initial_coverage | node_masks[node0];
        //     // Safe check for node1
        //     if (cov.test(node1) || 1 < last_chance[node1]) {
        //         this->backtrack(current, cov, 2, pre_planted_count + 1, (int)cov.count());
        //     }
        // }));
        // // Plant only second one
        // tasks.push_back(async(launch::async, [this, current, initial_coverage, pre_planted_count, node0, node1]() mutable {
        //     // current[node0] = '0';
        //     // current[node1] = '1';
        //     current.reset(node0); 
        //     current.set(node1);
        //     auto cov = initial_coverage | node_masks[node1];
        //     // Safe check for node0
        //     if (initial_coverage.test(node0) || 0 < last_chance[node0]) {
        //         this->backtrack(current, cov, 2, pre_planted_count + 1, (int)cov.count());
        //     }
        // }));
        // // Not Plant neither
        // tasks.push_back(async(launch::async, [this, current, initial_coverage, pre_planted_count, node0, node1]() mutable {
        //     // current[node0] = '0';
        //     // current[node1] = '0';
        //     current.reset(node0); 
        //     current.reset(node1);
        //     // Safe check for both
        //     bool safe0 = initial_coverage.test(node0) || 0 < last_chance[node0];
        //     bool safe1 = initial_coverage.test(node1) || 1 < last_chance[node1];
        //     if (safe0 && safe1) {
        //         this->backtrack(current, initial_coverage, 2, pre_planted_count, (int)initial_coverage.count());
        //     }
        // }));

        for (auto &t : tasks) {
            t.get();
        }

        // Separate in to 2 Thread Version
        // string config1 = current; 
        // config1[order[0]] = '1';

        // // Launch Choice 1 (Plant) in a background thread
        // auto f1 = std::async(std::launch::async, [this, config1, initial_coverage, pre_planted_count]() mutable {
        //     auto next_cov = initial_coverage | node_masks[order[0]];
        //     // Inside this thread, we call backtrack with a REFERENCE to our local config1
        //     this->backtrack(config1, next_cov, 1, pre_planted_count + 1, (int)next_cov.count());
        // });        

        // // Choice 2 (No Plant)
        // string config2 = current; 
        // config2[order[0]] = '0';
        // if (initial_coverage.test(order[0]) || 0 < last_chance[order[0]]) {
        //     backtrack(config2, initial_coverage, 1, pre_planted_count, (int)initial_coverage.count());
        // }

        // f1.get();

        cout << "Final Result : " << result << endl;
    }


};


int main(int argc, char* argv[]) {
    // WARNING : Usage 
    if (argc < 3) {
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
    // vector<vector<int>> adj(n);
    
     
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
    ofstream outfile(argv[2]);
    if (!outfile) {
        cerr << "Could not open output file: " << argv[2] << endl;
        return 1;
    }

    outfile << city.result << "\n"; 
    outfile.close();

    return 0;
}