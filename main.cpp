#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <bitset>

using namespace std;

struct City{
    int num_nodes;
    vector<vector<int>> adj;
    string result;
    vector<bool> powered;
    // int powered;
    int minPlants;

    // Construction
    // City(int n): num_nodes(n) , adj(n), result(n,'0') , powered(n,false) {}
    City(int n): num_nodes(n) , adj(n), result(n,'0') , powered(0) , minPlants(n+1) {}

    // Add edge in adj
    void addEdge(int u , int v){
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

    void backtrack(string& currentConfig, int index, int currentCount) {
        // Pruning
        if (currentCount >= minPlants) return;

        // Base case
        if (index == num_nodes) {
            if (isPowered(currentConfig)) {
                minPlants = currentCount;
                result = currentConfig;
            }
            return;
        }
        
        // plant
        currentConfig[index] = '1';
        backtrack(currentConfig, index + 1, currentCount + 1);

        // no plant
        currentConfig[index] = '0';
        backtrack(currentConfig, index + 1, currentCount);
    }

    void solveByGreedy(){
        string current = string(num_nodes, '0');
        backtrack(current, 0, 0);
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