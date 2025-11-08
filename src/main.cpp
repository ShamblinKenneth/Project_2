#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

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
// Load one dataset (single file)
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
        if (fields.size() < 16) continue; // safety check

        string title = fields[2];
        string tagsStr = fields[6];
        double views = 0.0, likes = 0.0;

        try {
            views = stod(fields[7]);
            likes = stod(fields[8]);
        } catch (...) {
            continue; // skip rows with invalid numeric data
        }

        double ratio = (views == 0.0) ? 0.0 : likes / views;
        vector<string> tags = split(tagsStr, '|');

        videos.push_back({title, tags, views, likes, ratio});
    }

    file.close();
    return videos;
}

// ------------------------------------------------------------
// Load and combine all datasets in the "data/" folder
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
// Heap-based analysis
// ------------------------------------------------------------
void analyzeWithHeap(const vector<Video> &videos, const vector<string> &selectedTags) {
    struct Compare {
        bool operator()(const pair<double, string> &a, const pair<double, string> &b) {
            return a.first < b.first;
        }
    };

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

    cout << "\nTop 10 videos by like/view ratio for selected tags:\n";
    for (int i = 0; i < 10 && !heap.empty(); ++i) {
        auto top = heap.top();
        heap.pop();
        cout << i + 1 << ". " << top.second << " (ratio: " << top.first << ")\n";
    }
}

// ------------------------------------------------------------
// Hash Table-based analysis
// ------------------------------------------------------------
void analyzeWithHashTable(const vector<Video> &videos, const vector<string> &selectedTags) {
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

    cout << "\nAverage like/view ratio for each selected tag:\n";
    for (const auto &selTag : selectedTags) {
        const auto &ratios = tagRatios[selTag];
        if (ratios.empty()) {
            cout << "Tag '" << selTag << "' not found.\n";
            continue;
        }
        double sum = 0;
        for (double r : ratios) sum += r;
        double avg = sum / ratios.size();
        cout << " - " << selTag << ": " << avg << "\n";
    }
}

// ------------------------------------------------------------
// Main console program
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
    if (videos.size() < 100000) {
        cerr << "Warning: Combined dataset has only " << videos.size() << " videos.\n";
        cerr << "Try adding more CSVs to the data/ folder.\n";
    }

    vector<string> selectedTags;
    bool running = true;

    while (running) {
        cout << "\n1. Select tag(s)";
        cout << "\n2. Choose data structure (Heap / Hash Table)";
        cout << "\n3. Exit";
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
                cout << "Choose data structure:\n1. Heap\n2. Hash Table\n> ";
                int dsChoice;
                cin >> dsChoice;
                cin.ignore();

                if (selectedTags.empty()) {
                    cout << "Select tags first.\n";
                    break;
                }

                if (dsChoice == 1)
                    analyzeWithHeap(videos, selectedTags);
                else if (dsChoice == 2)
                    analyzeWithHashTable(videos, selectedTags);
                else
                    cout << "Invalid choice.\n";
                break;
            }
            case 3:
                running = false;
                break;
            default:
                cout << "Invalid input.\n";
        }
    }

    cout << "Exiting... Goodbye!\n";
    return 0;
}
