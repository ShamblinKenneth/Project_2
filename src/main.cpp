#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <filesystem>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;
using namespace std::chrono;

// ------------------------------------------------------------
// Structure to hold video data
// ------------------------------------------------------------
struct Video {
    string title;
    vector<string> tags;
    double views;
    double likes;
    double ratio;
};

// ------------------------------------------------------------
// Utility: split string by a delimiter
// ------------------------------------------------------------
vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        if (!item.empty())
            elems.push_back(item);
    }
    return elems;
}

// ------------------------------------------------------------
// Parse a CSV line safely (handles quoted commas)
// ------------------------------------------------------------
vector<string> parseCSVLine(const string &line) {
    vector<string> result;
    string current;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

// ------------------------------------------------------------
// Load one dataset
// ------------------------------------------------------------
vector<Video> loadSingleDataset(const string &filename) {
    vector<Video> videos;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        return videos;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> fields = parseCSVLine(line);
        if (fields.size() < 16) continue;

        string title = fields[2];
        string tagsStr = fields[6];
        double views = 0.0, likes = 0.0;

        try {
            views = stod(fields[7]);
            likes = stod(fields[8]);
        } catch (...) {
            continue;
        }

        double ratio = (views == 0.0) ? 0.0 : likes / views;
        vector<string> tags = split(tagsStr, '|');

        videos.push_back({title, tags, views, likes, ratio});
    }

    file.close();
    return videos;
}

// ------------------------------------------------------------
// Load and combine all datasets
// ------------------------------------------------------------
vector<Video> loadAllDatasets(const string &folderPath) {
    vector<Video> allVideos;
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".csv") {
            cout << "Loading: " << entry.path().filename().string() << " ...\n";
            vector<Video> vids = loadSingleDataset(entry.path().string());
            cout << "  -> Loaded " << vids.size() << " videos.\n";
            allVideos.insert(allVideos.end(), vids.begin(), vids.end());
        }
    }
    cout << "\nTotal videos loaded from all datasets: " << allVideos.size() << "\n";
    return allVideos;
}

// ------------------------------------------------------------
// Heap-based analysis (returns runtime)
// ------------------------------------------------------------
long long analyzeWithHeap(const vector<Video> &videos, const vector<string> &selectedTags, bool showOutput = true) {
    struct Compare {
        bool operator()(const pair<double, string> &a, const pair<double, string> &b) {
            return a.first < b.first;
        }
    };

    auto start = high_resolution_clock::now();

    priority_queue<pair<double, string>, vector<pair<double, string>>, Compare> heap;

    for (const auto &v : videos) {
        for (const auto &tag : v.tags) {
            for (const auto &selTag : selectedTags) {
                if (tag.find(selTag) != string::npos) {
                    heap.push({v.ratio, v.title});
                }
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    if (showOutput) {
        cout << "\n[Heap Analysis Completed in " << duration << " ms]\n";
        cout << "Top 10 videos by like/view ratio:\n";
        for (int i = 0; i < 10 && !heap.empty(); ++i) {
            auto top = heap.top();
            heap.pop();
            cout << i + 1 << ". " << top.second << " (ratio: " << top.first << ")\n";
        }
    }

    return duration;
}

// ------------------------------------------------------------
// Hash Table-based analysis (returns runtime)
// ------------------------------------------------------------
long long analyzeWithHashTable(const vector<Video> &videos, const vector<string> &selectedTags, bool showOutput = true) {
    auto start = high_resolution_clock::now();

    unordered_map<string, vector<double>> tagRatios;

    for (const auto &v : videos) {
        for (const auto &tag : v.tags) {
            for (const auto &selTag : selectedTags) {
                if (tag.find(selTag) != string::npos) {
                    tagRatios[selTag].push_back(v.ratio);
                }
            }
        }
    }

    unordered_map<string, double> tagAverages;
    for (const auto &entry : tagRatios) {
        double sum = 0;
        for (double r : entry.second) sum += r;
        tagAverages[entry.first] = (entry.second.empty() ? 0.0 : sum / entry.second.size());
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    if (showOutput) {
        cout << "\n[Hash Table Analysis Completed in " << duration << " ms]\n";
        cout << "Average like/view ratio for selected tags:\n";
        for (const auto &tag : selectedTags) {
            if (tagAverages.find(tag) != tagAverages.end())
                cout << " - " << tag << ": " << tagAverages[tag] << "\n";
            else
                cout << " - " << tag << ": (no data)\n";
        }
    }

    return duration;
}

// ------------------------------------------------------------
// Run both analyses multiple times and compare average time
// ------------------------------------------------------------
void compareDataStructures(const vector<Video> &videos, const vector<string> &selectedTags) {
    const int runs = 3;
    long long totalHeap = 0, totalHash = 0;

    cout << "\nRunning both analyses " << runs << " times each to calculate average runtime...\n";

    for (int i = 1; i <= runs; ++i) {
        cout << "\n--- Run #" << i << " ---\n";
        long long heapTime = analyzeWithHeap(videos, selectedTags, false);
        long long hashTime = analyzeWithHashTable(videos, selectedTags, false);
        cout << "Heap: " << heapTime << " ms | Hash Table: " << hashTime << " ms\n";
        totalHeap += heapTime;
        totalHash += hashTime;
    }

    double avgHeap = totalHeap / static_cast<double>(runs);
    double avgHash = totalHash / static_cast<double>(runs);

    cout << "\n--------------------------------------------------\n";
    cout << "Performance Comparison Summary (Average of " << runs << " runs)\n";
    cout << "--------------------------------------------------\n";
    cout << "Average Heap Time:       " << avgHeap << " ms\n";
    cout << "Average Hash Table Time: " << avgHash << " ms\n";

    if (avgHeap < avgHash)
        cout << "✅ Heap is faster on average.\n";
    else if (avgHash < avgHeap)
        cout << "✅ Hash Table is faster on average.\n";
    else
        cout << "⚖️ Both performed equally on average.\n";

    cout << "--------------------------------------------------\n";
}

// ------------------------------------------------------------
// Main Interactive Console
// ------------------------------------------------------------
int main() {
    cout << "--------------------------------------------------\n";
    cout << "   YouTube Tag Correlation Analyzer (C++)\n";
    cout << "--------------------------------------------------\n";

    string folder = "data";
    if (!fs::exists(folder)) {
        cerr << "Error: 'data/' folder not found.\n";
        return 1;
    }

    vector<Video> videos = loadAllDatasets(folder);
    cout << "Loaded " << videos.size() << " videos total.\n";

    if (videos.empty()) {
        cerr << "No data loaded. Exiting.\n";
        return 1;
    }

    vector<string> selectedTags;
    bool running = true;

    while (running) {
        cout << "\n1. Select tag(s)";
        cout << "\n2. Run Heap Analysis";
        cout << "\n3. Run Hash Table Analysis";
        cout << "\n4. Compare Both (Average Runtime)";
        cout << "\n5. Exit";
        cout << "\n> ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                string tagsInput;
                cout << "Enter tags separated by commas (e.g., music,gaming): ";
                getline(cin, tagsInput);
                selectedTags = split(tagsInput, ',');
                cout << "Tags selected.\n";
                break;
            }
            case 2: {
                if (selectedTags.empty()) {
                    cout << "Select tags first.\n";
                    break;
                }
                analyzeWithHeap(videos, selectedTags);
                break;
            }
            case 3: {
                if (selectedTags.empty()) {
                    cout << "Select tags first.\n";
                    break;
                }
                analyzeWithHashTable(videos, selectedTags);
                break;
            }
            case 4: {
                if (selectedTags.empty()) {
                    cout << "Select tags first.\n";
                    break;
                }
                compareDataStructures(videos, selectedTags);
                break;
            }
            case 5:
                running = false;
                break;
            default:
                cout << "Invalid input.\n";
        }
    }

    cout << "\nExiting... Goodbye!\n";
    return 0;
}

