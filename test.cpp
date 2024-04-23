#include "hashmap.h"
#include <iostream>

int main(){
    std::vector<input> ins;
    input in1;
    in1.vars["a"] = 33;
    in1.vars["b"] = -2;
    in1.vars["c"] = "abc";

    input in2;
    in2.vars["a"] = -25;
    in2.vars["b"] = 13;
    in2.vars["c"] = "";

    input in3;
    in3.vars["a"] = 500;
    in3.vars["b"] = 10;
    in3.vars["c"] = "ajfdaflj";

    ins.push_back(in1);
    ins.push_back(in2);
    ins.push_back(in3);
    std::cout << "inputs: " << ins.size() << std::endl;
    hashmap<int> hm(ins, std::vector{5, 6, 7});

    hm.train();
    std::cout << "results: " << std::endl;
    std::cout << hm[in1] << std::endl;
    std::cout << hm[in2] << std::endl;
    std::cout << hm[in3] << std::endl;
    return 0;
}