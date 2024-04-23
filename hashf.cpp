#include "hashf.h"
#include "unordered_set"
#include <cmath>
#include <iostream>



double hashf::cost(ast* tree) const{
    // cost = range + 2*collisions
    // 1 1 1 1 1
    // cost = 5 (0 + 5), 10
    // 1 1 2 3 4
    // cost = 5 (3 + 2), 7
    // 1 2 3 4 5 (4 + 0), 4 
    // 1 1 2 2 2, 11
    // range = abs(range - n-1)
    // collisions = actual collisions

    int n = ins.size();
    std::vector<int8_t> outs; 
    for(input in: ins){
        int8_t out = tree->execute(in);
        outs.push_back(out);
    }

    int8_t smallest = INT8_MAX;
    int8_t largest = INT8_MIN;
    double negs = 0;

    std::unordered_map<int8_t, size_t> seen;
    for(int8_t out: outs){
        smallest = std::min(smallest, out);
        largest = std::max(largest, out);
        if(seen.count(out)){
            seen[out]++;
        }else{
            seen[out] = 1;
        }
        negs += (out < 0)*4;
    }
    double range_score = abs((largest-smallest) - (n-1));

    double collision_score = 0;
    for(auto [out, count]: seen){
        if(count > 1){
            collision_score+=count;
        }
    }
    collision_score*=4;

    return range_score + collision_score + negs + tree->terms.size();
}

int8_t hashf::operator()(input in){
    return curtree->execute(in);
}

double hashf::acceptance(double opt_forward, double opt_backward) const{
    double prev_cost = cost(prevtree);
    double cur_cost = cost(curtree);
    std::cout << "PREV: " << prev_cost << std::endl;
    std::cout << "CUR: " << cur_cost << std::endl;
    return std::min((double) 1, exp(-1*BETA * cur_cost/prev_cost)* opt_forward/opt_backward);
}

void hashf::iterate(){
    std::cout << iterations << std::endl;
    // std::cout << *curtree << std::endl;
    double prob = ((double)rand())/RAND_MAX;
    double opt_forward = 1;
    double opt_backward = 1;
    delete prevtree;
    prevtree = curtree->copy();
    if(prob <= proposals.swap_ops){
        // std::cout << "swap op" << std::endl;
        curtree->swap_op();
    }else if(prob > proposals.swap_ops &&
    (prob-proposals.swap_ops) <= proposals.swap_leaf){
        // std::cout << "swap leaf" << std::endl;
        curtree->swap_leaf_value();
    }else{
        // std::cout << "swap op/leaf" << std::endl;
        curtree->swap_leaf_op(opt_forward, opt_backward);
    }

    double accept = acceptance(opt_forward, opt_backward);
    std::cout << "a: " << accept << std::endl;
    if(accept < 1){
        double try_prob = ((double)rand())/RAND_MAX;
        if(try_prob > accept){
            curtree = prevtree->copy();
        }
    }
    iterations++;
    if(iterations == 1e5){
        std::cout << curtree->print() << std::endl;
    }
}