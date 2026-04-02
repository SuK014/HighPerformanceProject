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
    
    // int powered;
    int minPlants;

    // Bitset (Vectorization)
    bitset<MAX_NODES> node_masks[MAX_NODES];    

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
    void backtrack(string& currentConfig, bitset<MAX_NODES> current_coverage, int index, int currentCount) {
        // Pruning
        if (currentCount >= minPlants) return;

        // Base case
        if (current_coverage.count() == (size_t)num_nodes) {
            if (currentCount < minPlants) {
                minPlants = currentCount;
                result = currentConfig; 
            }
            return;
        }

        if (index == order.size()) return;

        int node = order[index];


        // Choice 1 : Plant
        currentConfig[node] = '1';
        // Using OR in bitmask (instead of the for loop update coverage) (Vectorization)
        backtrack(currentConfig, current_coverage | node_masks[node], index + 1, currentCount + 1);

        // Choice 2 : NOT Plant
        currentConfig[node] = '0';
        // Safe Check : if there aren't any future neighbor to be plant it will not go this way
        if (current_coverage.test(node) || index < last_chance[node]) {
            backtrack(currentConfig, current_coverage, index + 1, currentCount);
        }
        // if (current_coverage.test(node)) {
        //     backtrack(currentConfig, current_coverage, index + 1, currentCount);
        // }
        // Safe Check : if there aren't any future neighbor to be plant it will not go this way
        // else {
        //     bool has_future_neighbor = false;
        //     for (int neighbor : adj[node]) {
        //         auto it = find(order.begin() + index + 1, order.end(), neighbor);
        //         if (it != order.end()) {
        //             has_future_neighbor = true;
        //             break;
        //         }
        //     }
        //     if (has_future_neighbor) {
        //         backtrack(currentConfig, current_coverage, index + 1, currentCount);
        //     }
        // }
    }


    void solveByGreedy(){
        // turn adj into coverage bitmask
        precomputeMasks();
        // check bottleneck
        // cout << "finish percompute" <<endl;

        string current = string(num_nodes, '0');
        // (Vectorization)
        bitset<MAX_NODES> initial_coverage;
        int pre_planted_count = 0;
        vector<int> remaining_order;

        for (int i = 0; i < num_nodes; i++) {
            if (adj[i].empty()) {
                current[i] = '1';
                initial_coverage.set(i);
                pre_planted_count++;
            } else {
                remaining_order.push_back(i);
            }
        }
        
        sort(remaining_order.begin(), remaining_order.end(), [&](int a, int b) {
            return adj[a].size() > adj[b].size();
        });

        // check bottleneck
        // cout << "finish sort" <<endl;

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
        // cout << "finish prepare graph" <<endl;

        backtrack(current, initial_coverage, 0, pre_planted_count);

        cout << result << endl;
    }


};


int main(int argc, char* argv[]) {
    // WARNING : Usage 
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
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
    vector<vector<int>> adj(n);
    
     
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

    return 0;
}