#include "hashf.h"
#include <list>
#include <utility>
#include <cassert>
#include <cmath>
#include <iostream>

bool equals(input& mine, input& other){
    if(other.vars.size() != mine.vars.size()) return false;
    for(auto [varname, val]: other.vars){
        if(mine.vars[varname] != val){
            return false;
        }
    }
    return true;
}

template<typename V>
struct hashmap{
    hashf function;
    std::vector<input> ins;
    std::vector<V> values;
    std::vector<std::list<std::pair<input, V>>> storage;
    size_t sz = 1;

    hashmap(std::vector<input> ins, std::vector<V> values):
    ins(ins), values(values),
    function(ins, {0.33, 0.33, 0.34})
    {
        assert(values.size() == ins.size());
        std::cout << "created hashmap" << std::endl;
    }

    void train(){
        std::cout << "training" << std::endl;
        for(int i = 0; i < 1e5; i++){
            function.iterate();
        }
        for(int i = 0; i < ins.size(); i++){
            int8_t idx = function(ins[i]);
            std::cout << "idx: " << (int) idx << std::endl;
            if(idx >= storage.size()){
                storage.resize(idx+1);
            }
            storage[idx].push_front({ins[i], values[i]});
        }
    }

    V& operator[] (input& key){
        int idx = function(key);
        assert(storage[idx].size());
        if(storage[idx].size() == 1){
            return storage[idx].front().second;
        }else{
            for(auto [in, val]: storage[idx]){
                if(equals(in,key)){
                    return val;
                }
            }
        }
        return storage[idx].front().second;
    }
};