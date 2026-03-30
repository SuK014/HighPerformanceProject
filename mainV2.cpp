#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <bitset>
#include <numeric>

using namespace std;

struct City{
    int num_nodes;
    vector<vector<int>> adj;
    string result;
    vector<bool> powered;
    vector<int> order;

    // int powered;
    int minPlants;

    // Construction
    // City(int n): num_nodes(n) , adj(n), result(n,'0') , powered(n,false) {}
    City(int n): num_nodes(n) , adj(n), result(n,'0') , powered(0) , minPlants(n+1) {}

    // Add edge in adj
    void addEdge(int u , int v){
        if (u >= num_nodes || v >= num_nodes) return;
        adj[u].push_back(v); 
        adj[v].push_back(u);
    }
    
    // Check if all city is powered
    bool isPowered(const string& config) {
        vector<bool> covered(num_nodes, false);
        for (int i = 0; i < num_nodes; ++i) {
            if (config[i] == '1') {
                covered[i] = true;
                for (int neighbor : adj[i]) {
                    covered[neighbor] = true;
                }
            }
        }
        for (bool c : covered) if (!c) return false;
        return true;
    }

    // Backtracking
    void backtrack(string& currentConfig, vector<int>& coverage, int index, int currentCount, int &powered_count, int total_to_process) {
        // Pruning
        if (currentCount >= minPlants) return;

        // Base case
        if (powered_count == num_nodes) {
            minPlants = currentCount;
            result = currentConfig;
            return;
        }

        if (index == total_to_process) return;

        int node = order[index];

        // plant
        int added_coverage = 0;
    
        // Update coverage for the node itself
        if (coverage[node]++ == 0) added_coverage++;
        
        // Update coverage for neighbors
        for (int neighbor : adj[node]) {
            if (coverage[neighbor]++ == 0) added_coverage++;
        }

        currentConfig[node] = '1';
        powered_count += added_coverage;
        backtrack(currentConfig, coverage, index + 1, currentCount + 1, powered_count,total_to_process);

        // no plant
        currentConfig[node] = '0';
        powered_count -= added_coverage;
        if (--coverage[node] == 0); 
        for (int neighbor : adj[node]) {
            --coverage[neighbor];
        }
        
        int temp_powered = 0;
        for(int i=0; i<num_nodes; i++) if(coverage[i] > 0) temp_powered++;
        powered_count = temp_powered;
        backtrack(currentConfig, coverage, index + 1, currentCount, powered_count,total_to_process);
    }

    // void solveByGreedy(){
    //     order.resize(num_nodes);
    //     iota(order.begin(), order.end(), 0);

    //     sort(order.begin(), order.end(), [&](int a, int b) {
    //         return adj[a].size() > adj[b].size();
    //     });

    //     cout << "--- Node Processing Order (Degree-based) ---" << endl;
    //     for (int node : order) {
    //         cout << "Node: " << node << " | Adjacency Count: " << adj[node].size() << endl;
    //     }
    //     cout << "--------------------------------------------" << endl;

    //     string current = string(num_nodes, '0');
    //     vector<int> coverage(num_nodes, 0);
    //     int powered_count = 0;
    //     backtrack(current, coverage, 0, 0, powered_count);

    //     cout << result << endl;
    // }

    void solveByGreedy(){
        string current = string(num_nodes, '0');
        vector<int> coverage(num_nodes, 0);
        int powered_count = 0;
        int pre_planted_count = 0;
        vector<int> remaining_order;

        for (int i = 0; i < num_nodes; i++) {
            if (adj[i].empty()) {
                current[i] = '1';
                coverage[i] = 1;
                powered_count++;
                pre_planted_count++;
            } else {
                remaining_order.push_back(i);
            }
        }

        sort(remaining_order.begin(), remaining_order.end(), [&](int a, int b) {
            return adj[a].size() > adj[b].size();
        });

        cout << "--- Node Processing Order (Degree-based) ---" << endl;
        for (int node : remaining_order) {
            cout << "Node: " << node << " | Adjacency Count: " << adj[node].size() << endl;
        }
        cout << "--------------------------------------------" << endl;

        order = remaining_order;
        int remaining_nodes = order.size();
        backtrack(current, coverage, 0, pre_planted_count, powered_count, remaining_nodes);

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