#ifndef PROGRESS_H
#define PROGRESS_H

#include <iostream>

class ProgressBar {
public:
    void update(float progress) {
        int barWidth = 70;
        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
    }

    void finish() {
        int barWidth = 70;
        std::cout << "[";
        for (int i = 0; i < barWidth; ++i) {
            std::cout << "=";
        }
        std::cout << "] " << "100%\n";
        std::cout.flush();
    }
};

#endif