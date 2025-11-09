#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cmath>
#include <cstdlib>   // for system()
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// ------------------------------------------------------------
// Video record structure
// ------------------------------------------------------------
struct Video {
    string title;
    vector<string> tags;
    double views;
    double likes;
    double ratio;
};

// ------------------------------------------------------------
// Utility: split string
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
// Load dataset from CSV
// ------------------------------------------------------------
vector<Video> loadDataset(const string &filename) {
    vector<Video> videos;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        return videos;
    }

    string line;
    getline(file, line); // skip header
    while (getline(file, line)) {
        stringstream ss(line);
        string field;
        vector<string> fields;

        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() < 9) continue;

        string title = fields[2];
        string tagsStr = fields[6];
        double views = stod(fields[7]);
        double likes = stod(fields[8]);
        double ratio = (views == 0) ? 0.0 : likes / views;

        vector<string> tags = split(tagsStr, '|');
        videos.push_back({title, tags, views, likes, ratio});
    }
    file.close();
    return videos;
}

// ------------------------------------------------------------
// Heap-based correlation analysis
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
        auto top = heap.top(); heap.pop();
        cout << i + 1 << ". " << top.second << " (ratio: " << top.first << ")\n";
    }
}

// ------------------------------------------------------------
// Hash table-based correlation analysis
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
// Unzip the dataset if needed
// ------------------------------------------------------------
bool ensureDatasetExtracted() {
    fs::path zipPath = "data/archive.zip";
    fs::path unzipDir = "data/unzipped";

    if (!fs::exists(zipPath)) {
        cerr << "Error: archive.zip not found in data/ folder.\n";
        return false;
    }

    if (!fs::exists(unzipDir)) {
        cout << "Extracting dataset...\n";
        fs::create_directories(unzipDir);
        string command = "unzip -o " + zipPath.string() + " -d " + unzipDir.string();
        int result = system(command.c_str());
        if (result != 0) {
            cerr << "Error: Failed to unzip dataset. Ensure 'unzip' is installed.\n";
            return false;
        }
    }

    return true;
}

// ------------------------------------------------------------
// Main menu / wireframe console
// ------------------------------------------------------------
int main() {
    cout << "--------------------------------------------------\n";
    cout << "   YouTube Tag Correlation Analyzer (C++)\n";
    cout << "--------------------------------------------------\n";

    if (!ensureDatasetExtracted()) return 1;

    // Common Kaggle file name (adjust if needed)
    string datasetFile = "data/unzipped/USvideos.csv";
    vector<Video> videos = loadDataset(datasetFile);
    if (videos.empty()) {
        cerr << "Dataset could not be loaded. Check CSV path.\n";
        return 1;
    }
    cout << "Loaded " << videos.size() << " videos.\n";

    vector<string> selectedTags;
    bool running = true;

    while (running) {
        cout << "\n1. Select tag(s)";
        cout << "\n2. Choose data structure";
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
