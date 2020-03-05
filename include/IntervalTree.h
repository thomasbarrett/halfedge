#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

struct IntervalTreeNode {

};

class IntervalTree {
private:
    struct Node {
        double x;
    };

    std::vector<Node> tree_;
public:
    IntervalTree(std::vector<double> min, std::vector<double> max) {
        std::set<double> elementary_intervals;
        elementary_intervals.insert(min.begin(), min.end());
        elementary_intervals.insert(max.begin(), max.end());
        tree_.resize(elementary_intervals.size());
    }

    priva
}

#endif /* INTERVAL_TREE_H */