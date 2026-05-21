#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <deque>
#include <fstream>

using namespace std;

struct Item {
    int id;           
    int current_type; 
};

struct Machine {
    int id;
    bool is_busy = false;
    deque<Item> queue;
};

enum EventType { FINISH = 0, START = 1, WAIT = 2, READY = 3 };

struct Event {
    long long time;
    EventType type;
    int item_id;
    int type_id;
    int machine_id;
    int p_size; // для wait

    bool operator>(const Event& other) const {
        if (time != other.time) return time > other.time;
        return type > other.type;
    }
};

int M, N;
vector<vector<int>> T;
vector<Machine> machines;
vector<Item> items;
priority_queue<Event, vector<Event>, greater<Event>> event_queue;
int finished_items_count = 0;
ofstream output_file;

void report_error(const string& line) {
    cout << line << endl;
    exit(0);
}

long long calculate_wait_time(int m_idx) {
    long long wait = 0;
    for (const auto& it : machines[m_idx].queue) {
        wait += T[it.current_type][m_idx];
    }
    return wait;
}

bool is_number(const string& s) {
    if (s.empty()) return false;
    for (char const& c : s) if (!isdigit(c)) return false;
    return true;
}

void parse_input(int argc, char* argv[]) {
    if (argc < 2) exit(0);

    string filename = argv[1], line;
    freopen(argv[1], "r", stdin);

    if (!getline(cin, line)) exit(0);

    stringstream ss(line);
    string sm, sn;

    if (!(ss >> sm >> sn) || !is_number(sm) || !is_number(sn)) report_error(line);

    M = stoi(sm);
    N = stoi(sn);

    if (M < 1 || N < 1 || M > 100 || N > 100) report_error(line);

    T.assign(M - 1, vector<int>(N));
    for (int i = 0; i < M - 1; i++) {
        if (!getline(cin, line)) report_error("");
        stringstream ss_t(line);

        for (int j = 0; j < N; j++) {
            string st;
            if (!(ss_t >> st) || !is_number(st)) report_error(line);
            
            T[i][j] = stoi(st);

            if (T[i][j] < 0 || T[i][j] > 10000) report_error(line);
        }

        string z; if (ss_t >> z) report_error(line);
    }

    int cur_id = 0;
    for (int j = 0; j < N; ++j) {
        if (!getline(cin, line)) report_error("");

        stringstream ss_q(line);
        string sq;
        
        if (!(ss_q >> sq) || !is_number(sq)) report_error(line);
        int q_j = stoi(sq);
        machines.push_back({ j, false, {} });
        
        for (int p = 0; p < q_j; ++p) {
            string stype;
            if (!(ss_q >> stype) || !is_number(stype)) report_error(line);

            int type = stoi(stype);

            if (type < 0 || type > M - 2) report_error(line);

            Item it = { cur_id++, type };
            items.push_back(it);
            machines[j].queue.push_back(it);
        }

        string z; if (ss_q >> z) report_error(line);
    }

}

void schedule_next_machine(int item_idx, long long current_time) {
    int item_type = items[item_idx].current_type;
    int best_m = 0;
    long long min_wait = -1;

    for (int j = 0; j < N; ++j) {
        long long w = calculate_wait_time(j);
        if (min_wait == -1 || w < min_wait) {
            min_wait = w;
            best_m = j;
        }
    }

    machines[best_m].queue.push_back(items[item_idx]);

    if (machines[best_m].is_busy) {
        // Станок занят – изделие становится в очередь
        event_queue.push({ current_time, WAIT, item_idx, item_type, best_m,
                           (int)machines[best_m].queue.size() - 1 });
    }
    else {
        // Станок свободен – начинаем обработку немедленно
        event_queue.push({ current_time, START, item_idx, item_type, best_m, 0 });
        machines[best_m].is_busy = true;
    }
}

int main(int argc, char* argv[]) {
    output_file.open("example.txt");
    if (!output_file.is_open()) {
        cout << "Error: Cannot open example.txt for writing" << endl;
        return 1;
    }

    // Перенаправляем cout в файл
    streambuf* cout_buf = cout.rdbuf();
    cout.rdbuf(output_file.rdbuf());

    parse_input(argc, argv);

    for (int j = 0; j < N; ++j) {
        if (!machines[j].queue.empty()) {
            Item first = machines[j].queue.front();
            event_queue.push({ 0, START, first.id , first.current_type, j, 0 });
            machines[j].is_busy = true;
        }
    }

    long long last_time = 0;
    while (!event_queue.empty()) {
        Event e = event_queue.top();
        event_queue.pop();
        last_time = e.time;

        switch (e.type) {
        case FINISH:
            cout << "finish " << e.time << " " << e.item_id << " " << e.type_id << " " << e.machine_id << endl;
            machines[e.machine_id].queue.pop_front();

            if (!machines[e.machine_id].queue.empty()) {
                Item next = machines[e.machine_id].queue.front();
                event_queue.push({ e.time, START, next.id, next.current_type, e.machine_id, 0 });
            }
            else {
                machines[e.machine_id].is_busy = false;
            }

            items[e.item_id].current_type++;
            if (items[e.item_id].current_type == M - 1) {
                event_queue.push({ e.time, READY, e.item_id, M - 1, e.machine_id, 0 });
            }
            else {
                schedule_next_machine(e.item_id, e.time);
            }

            break;

        case START:
            cout << "start " << e.time << " " << e.item_id << " " << e.type_id << " " << e.machine_id << endl;
            event_queue.push({ e.time + T[e.type_id][e.machine_id], FINISH, e.item_id, e.type_id, e.machine_id, 0 });
            break;

        case WAIT:
            cout << "wait " << e.time << " " << e.item_id << " " << e.type_id << " " << e.machine_id << " " << e.p_size << endl;
            break;

        case READY:
            cout << "ready " << e.time << " " << e.item_id << " " << e.machine_id << endl;
            finished_items_count++;
            break;
        }
    }

    cout << "stop " << last_time << endl;

    cout.rdbuf(cout_buf);
    output_file.close();

    return 0;
}