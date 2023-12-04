#include <iostream>
#include <fstream>
#include <Windows.h>
#include <mmsystem.h>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <algorithm>

#define BPM 100
#define DORIAN false

namespace gtup
{
    enum class VirtualKeyCode : int
    {
        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4A,
        K = 0x4B,
        L = 0x4C,
        M = 0x4D,
        N = 0x4E,
        O = 0x4F,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5A
    };

    std::unordered_map<std::string, VirtualKeyCode> noteMap = {
        {"C1", VirtualKeyCode::Z},
        {"D1", VirtualKeyCode::X},
        {"E1", VirtualKeyCode::C},
        {"F1", VirtualKeyCode::V},
        {"G1", VirtualKeyCode::B},
        {"A1", VirtualKeyCode::N},
        {"B1", VirtualKeyCode::M},

        {"C2", VirtualKeyCode::A},
        {"D2", VirtualKeyCode::S},
        {"E2", VirtualKeyCode::D},
        {"F2", VirtualKeyCode::F},
        {"G2", VirtualKeyCode::G},
        {"A2", VirtualKeyCode::H},
        {"B2", VirtualKeyCode::J},

        {"C3", VirtualKeyCode::Q},
        {"D3", VirtualKeyCode::W},
        {"E3", VirtualKeyCode::E},
        {"F3", VirtualKeyCode::R},
        {"G3", VirtualKeyCode::T},
        {"A3", VirtualKeyCode::Y},
        {"B3", VirtualKeyCode::U}};

    std::unordered_map<std::string, std::string> toDorian = {
        {"C1", "B0"},
        {"C2", "B1"},
        {"C3", "B2"},

        {"D1", "C1"},
        {"D2", "C2"},
        {"D3", "C3"},

        {"E1", "D1"},
        {"E2", "D2"},
        {"E3", "D0"},

        {"F1", "E1"},
        {"F2", "E2"},
        {"F3", "E3"},

        {"G1", "F1"},
        {"G2", "F2"},
        {"G3", "F3"},

        {"A1", "G1"},
        {"A2", "G2"},
        {"A3", "G3"},

        {"B1", "A1"},
        {"B2", "A2"},
        {"B3", "A0"}};

    void KeyPress(gtup::VirtualKeyCode keyCode)
    {
        INPUT input;
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = (int)keyCode;
        input.ki.dwFlags = 0; // 0 表示按下按键

        SendInput(1, &input, sizeof(INPUT));
    }

    void KeyRelease(gtup::VirtualKeyCode keyCode)
    {
        INPUT input;
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = (int)keyCode;
        input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP 表示释放按键

        SendInput(1, &input, sizeof(INPUT));
    }
    std::vector<std::pair<std::string, int>> Input(std::string path)
    {
        std::vector<std::pair<std::string, int>> res;
        std::fstream f;

        f.open(path);
        while (!f.eof())
        {
            std::string note;
            int time;
            f >> note >> time;
            res.emplace_back(std::pair<std::string, int>(note, time));
        }
        return res;
    }
}

std::vector<std::thread> threads;
std::mutex mtx;
std::condition_variable cv;
auto clk = std::chrono::steady_clock::now();
std::atomic<int> ready(0);

std::function<void()> readyClearer = []()
{
    std::this_thread::sleep_for(std::chrono::microseconds(60000000 / BPM) / 2);
    ready = 0;
};

int gcd(int x, int y)
{
    while (y ^= x ^= y ^= x %= y)
        ;
    return x;
}

void notesMerge(std::vector<std::pair<std::string, int>> &notes)
{
    std::pair<std::string, int> pre = notes.front();
    bool flag = true;
    while (flag)
    {
        flag = false;
        for (std::size_t i = 1; i < notes.size(); i++)
        {
            if (notes[i].second == 0 || pre.second == 0)
            {
                pre = notes[i];
                continue;
            }
            if (notes[i].first == "H0")
            {
                if (notes[i].second != pre.second)
                {
                    // different time
                    int big = std::max(notes[i].second, pre.second);
                    int sml = std::min(notes[i].second, pre.second);
                    if (big % sml != 0)
                    {
                        pre = notes[i];
                        continue;
                    }
                    else if (big % (1 + big / sml) != 0)
                    {
                        pre = notes[i];
                        continue;
                    }
                    else
                    {
                        flag = true;
                        notes[i - 1].second = big / (1 + big / sml);
                        notes.erase(notes.begin() + i);
                    }
                }
                else
                {
                    // same time
                    if (pre.first == "H0")
                    {
                        if (notes[i].first == "H0")
                        {
                            flag = true;
                            notes.erase(notes.begin() + i);
                            notes[i - 1].second /= 2;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
            pre = notes[i];
        }
    }
}

std::vector<std::pair<std::string, int>> trackMerge(std::vector<std::vector<std::pair<std::string, int>>> tracks)
{
    std::vector<std::pair<std::string, int>> res;
    std::vector<int> gcb;
    int dt = 1;
    int nl = 0;
    std::vector<int> pass(tracks.size(), 0);
    std::vector<std::size_t> index(tracks.size(), 0);
    for (auto track : tracks)
    {
        for (auto [note, time] : track)
        {
            if (note == "LL")
            {
                nl = nl > time ? nl : time;
                continue;
            }
            if (time == 0)
            {
                continue;
            }
            dt = dt * time / gcd(dt, time);
        }
    }

    for (int i = 0; i < dt * nl; i++)
    {
        std::vector<std::pair<std::string, int>> notes;
        for (std::size_t t = 0; t < tracks.size(); t++)
        {
            if (index[t] >= tracks[t].size())
            {
                continue;
            }
            if (pass[t] == i)
            {
                if (tracks[t][index[t]].first == "LL")
                {
                    index[t]++;
                }
                while (tracks[t][index[t]].second == 0)
                {
                    notes.emplace_back(std::pair<std ::string, int>(tracks[t][index[t]]));
                    index[t]++;
                }
                notes.emplace_back(tracks[t][index[t]]);
                pass[t] += dt / tracks[t][index[t]].second;
                index[t]++;
            }
        }
        notes.emplace_back(std::pair<std ::string, int>("H0", dt));

        for (int j = 0; j < (int)notes.size() - 1; j++)
        {
            res.emplace_back(std::pair<std::string, int>(notes[j].first, 0));
        }
        res.emplace_back(std::pair<std::string, int>(notes.back().first, dt));
    }
    notesMerge(res);
    return res;
}

int main()
{

    std::vector<std::vector<std::pair<std::string, int>>> tracks;
    tracks.emplace_back(gtup::Input("./song/flowerdance/melody.txt"));
    tracks.emplace_back(gtup::Input("./song/flowerdance/chord.txt"));

    std::cout << "Input finished, press 's' for single-thread, 'm' for muti-thread! " << std::endl;
    char c = getchar();

    std::function<void(std::vector<std::pair<std::string, int>>, int)> trackPlayer;
    trackPlayer = [](std::vector<std::pair<std::string, int>> track, int size)
    {
        clk = std::chrono::steady_clock::now();
        std::this_thread::sleep_until(clk + std::chrono::microseconds(5000000));
        clk = std::chrono::steady_clock::now();

        std::vector<std::string> mutiNotes;
        for (auto &[name, time] : track)
        {
            if (name == "LL")
            {
                std::unique_lock<std::mutex> lock(mtx);
                ready++;
                if (ready < size)
                {
                    cv.wait(lock, [&]()
                            { return ready >= size; });
                }
                else
                {
                    cv.notify_all();
                }
                std::thread t(readyClearer);
                t.detach();
                continue;
            }
            int last = 0;
            mutiNotes.emplace_back(name);
            if (time == 0)
            {
                continue;
            }
            else
            {
                last = 4 * 60000000 / (BPM * time);
                for (auto note : mutiNotes)
                {
                    std::string dn = note;
                    if (DORIAN)
                    {
                        dn = gtup::toDorian[note];
                    }
                    gtup::KeyPress(gtup::noteMap[dn]);
                    gtup::KeyRelease(gtup::noteMap[dn]);
                }
                std::this_thread::sleep_until(clk + std::chrono::microseconds(last));
                mutiNotes.clear();
            }
            clk = std::chrono::steady_clock::now();
        }
    };

    if (c == 'm')
    {
        // muti-thread
        std::cout << "The program will start in 5 seconds! " << std::endl;
        for (const auto &track : tracks)
        {
            threads.emplace_back(trackPlayer, track, tracks.size());
        }
        for (auto &th : threads)
        {
            th.join();
        }
    }
    else if (c == 's')
    {
        // single thread
        std::cout << "Merging tracks..." << std::endl;
        std::vector<std::pair<std::string, int>> mergedTrack = trackMerge(tracks);
        std::cout << "Merging finished, the program will start in 5 seconds! " << std::endl;
        std::thread player(trackPlayer, mergedTrack, 0);
        player.join();
    }

    system("pause");
}
