#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <limits>
#include <string>
#include <algorithm>

using namespace std;

// ANSI color codes for different subway lines.
string getColor(const string &line) {
    if (line == "1") return "\033[31m";         // red
    if (line == "2") return "\033[32m";         // green
    if (line == "3") return "\033[34m";         // blue
    if (line == "Interchange") return "\033[35m"; // magenta
    return "\033[0m";                           // default
}

// Reset color.
const string reset = "\033[0m";

// Represents a connection from one station to another.
struct Edge {
    string destination;
    int cost;
    string line; // subway line (e.g., "1", "2", "3")
};

// Graph class representing the subway system.
class Graph {
public:
    // Adjacency list: station name -> list of edges.
    unordered_map<string, vector<Edge>> adjList;

    // Add a directed edge from 'from' to 'to'.
    void addEdge(const string &from, const string &to, int cost, const string &line) {
        adjList[from].push_back({to, cost, line});
    }

    // Add a bidirectional edge.
    void addBidirectionalEdge(const string &s1, const string &s2, int cost, const string &line) {
        addEdge(s1, s2, cost, line);
        addEdge(s2, s1, cost, line);
    }

    // Finds the minimum-cost path from source to destination using Dijkstra's algorithm.
    // A transfer (switching subway lines) incurs an extra transferCost if the traveling line changes.
    // Returns a pair: {total cost, vector of {station, line used to reach it}}.
    pair<int, vector<pair<string, string>>> dijkstra(const string &source, const string &destination, int transferCost) {
        // Distances from the source.
        unordered_map<string, int> dist;
        // For backtracking: map station -> {parent station, line used to get here}
        unordered_map<string, pair<string, string>> parent;

        // Initialize distances to "infinity".
        for (auto &pair : adjList) {
            dist[pair.first] = numeric_limits<int>::max();
        }
        dist[source] = 0;
        // For source, no incoming line.
        parent[source] = {"", ""};

        // Node structure for the priority queue.
        struct Node {
            int cost;
            string station;
            string line; // current line used to get to this station.
            bool operator>(const Node &other) const {
                return cost > other.cost;
            }
        };

        priority_queue<Node, vector<Node>, greater<Node>> pq;
        pq.push({0, source, ""});

        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();
            if (current.cost > dist[current.station])
                continue;
            if (current.station == destination)
                break;
            for (auto &edge : adjList[current.station]) {
                int extra = 0;
                // If already on a line and the edge's line is different, add transfer cost.
                if (!current.line.empty() && current.line != edge.line)
                    extra = transferCost;
                int newCost = current.cost + edge.cost + extra;
                if (newCost < dist[edge.destination]) {
                    dist[edge.destination] = newCost;
                    parent[edge.destination] = {current.station, edge.line};
                    pq.push({newCost, edge.destination, edge.line});
                }
            }
        }

        vector<pair<string, string>> fullPath;
        if (dist[destination] == numeric_limits<int>::max()) {
            return {-1, fullPath};  // destination unreachable.
        }
        // Reconstruct the path.
        string cur = destination;
        while (cur != source) {
            fullPath.push_back({cur, parent[cur].second});
            cur = parent[cur].first;
        }
        fullPath.push_back({source, ""}); // source has no incoming line.
        reverse(fullPath.begin(), fullPath.end());
        return {dist[destination], fullPath};
    }

    // Display the subway map in a neatly formatted style.
    void displayMap() {
        cout << "\nSubway Map:\n";
        for (const auto &station : adjList) {
            cout << station.first << ":\n";
            for (const auto &edge : station.second) {
                cout << "    -> " << edge.destination << " (" 
                     << getColor(edge.line) << "Line " << edge.line << reset 
                     << ", cost " << edge.cost << ")\n";
            }
            cout << "\n";
        }
    }
};

// Builds an extended sample subway graph with real NYC subway station names.
void buildSampleGraph(Graph &graph) {
    // Line "1"
    graph.addBidirectionalEdge("Times Sq", "42nd St", 4, "1");
    graph.addBidirectionalEdge("42nd St", "34th St", 5, "1");
    graph.addBidirectionalEdge("34th St", "Penn Station", 6, "1");

    // Line "2"
    graph.addBidirectionalEdge("42nd St", "Grand Central", 3, "2");
    graph.addBidirectionalEdge("Grand Central", "14th St", 6, "2");
    graph.addBidirectionalEdge("14th St", "Wall St", 7, "2");

    // Line "3"
    graph.addBidirectionalEdge("34th St", "Union Sq", 4, "3");
    graph.addBidirectionalEdge("Union Sq", "Houston St", 7, "3");
    graph.addBidirectionalEdge("Houston St", "Canal St", 5, "3");

    // Additional interchange scenarios (realistic transfers):
    // "42nd St" is served by Lines 1 and 2.
    // "34th St" is served by Lines 1 and 3.
    // Also, let's assume "Grand Central" and "Union Sq" are close enough to be an interchange.
    graph.addBidirectionalEdge("Grand Central", "Union Sq", 4, "Interchange");
}

int main() {
    Graph graph;
    // Set transfer cost for switching lines (e.g., 2 units).
    int transferCost = 2;
    buildSampleGraph(graph);

    // Display the subway map.
    graph.displayMap();

    // Create a sorted list of available stations for numbered selection.
    vector<string> stationList;
    for (const auto &entry : graph.adjList) {
        stationList.push_back(entry.first);
    }
    sort(stationList.begin(), stationList.end());

    cout << "Welcome to Smart Subway Navigator - NYC\n";
    cout << "Available stations:\n";
    for (size_t i = 0; i < stationList.size(); i++) {
        cout << i+1 << ". " << stationList[i] << "\n";
    }

    int srcIndex = 0, destIndex = 0;
    cout << "\nEnter source station number: ";
    cin >> srcIndex;
    cout << "Enter destination station number: ";
    cin >> destIndex;

    // Validate indices.
    if(srcIndex < 1 || srcIndex > stationList.size() ||
       destIndex < 1 || destIndex > stationList.size()) {
        cout << "Invalid station number(s) entered.\n";
        return 1;
    }
    
    // Map the indices back to station names.
    string src = stationList[srcIndex - 1];
    string dest = stationList[destIndex - 1];

    cin.ignore();  // clear the newline.

    auto result = graph.dijkstra(src, dest, transferCost);
    if (result.first == -1) {
        cout << "No available path from " << src << " to " << dest << "\n";
    } else {
        cout << "\nMinimum cost: " << result.first << "\nRoute Instructions:\n";
        // Print route instructions with line changes.
        // result.second is a vector of pair {station, lineUsed to get here}.
        // The source's associated line is empty.
        cout << "Start at " << result.second[0].first << "\n";
        string currentLine = "";
        for (size_t i = 1; i < result.second.size(); i++) {
            string station = result.second[i].first;
            string usedLine = result.second[i].second;
            if (usedLine != currentLine) {
                if (!currentLine.empty()) {
                    cout << "  -> At " << result.second[i-1].first 
                         << ", transfer to " << getColor(usedLine) 
                         << "Line " << usedLine << reset << "\n";
                } else {
                    cout << "  -> Take " << getColor(usedLine) 
                         << "Line " << usedLine << reset << "\n";
                }
                currentLine = usedLine;
            }
            cout << "  -> Arrive at " << station << "\n";
        }
    }
    return 0;
}