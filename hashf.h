#include "ast.h"
#include <variant>
#include <cassert>
#include <unordered_map>
#include <iostream>

struct proposal_distrib{
    double swap_ops;
    double swap_leaf;
    double swap_leaf_op;
};

struct hashf{

    // current state of the ast
    ast* curtree;
    ast* prevtree;
    std::vector<input> ins;
    proposal_distrib proposals;
    int iterations;
    const double BETA = 2.2f;

    // setup the initial ast
    hashf(std::vector<input> in, proposal_distrib proposals):
    ins(in), proposals(proposals), iterations(0){
        assert(ins.size());
        std::cout << "making hashf" << std::endl;
        for(int8_t v = -20; v < 20; v++){
            const_node* constnode = new const_node();
            constnode->value = v;
            ast::int_choices.push_back(constnode);
        }
        // const_node* constnode = new const_node();
        // constnode->value = INT8_MAX;
        // ast::int_choices.push_back(constnode);

        input first_input = ins.front();
        for(auto [varname, value]: first_input.vars){
            var_node* varnode = new var_node();
            varnode->varname = varname;
            if(std::holds_alternative<std::string>(value)){
                ast::str_choices.push_back(varnode);
            }else{
                ast::int_choices.push_back(varnode);
            }
            assert(!varnode->parent);
        }
        ast init_tree;
        init_tree.init();
        std::cout << "init_tree created" << std::endl;
        curtree = init_tree.copy();
        prevtree = init_tree.copy();
    }

    ~hashf(){
        if(curtree)
            delete curtree;
        if(prevtree)
            delete prevtree;
        for(auto term: ast::int_choices){
            delete term;
        }
        for(auto term: ast::str_choices){
            delete term;
        }
    }


    // scoring function for stationary distribution
    double cost(ast* tree) const;
    double acceptance(double opt_forward, double opt_backward) const;

    // perform iteration of mcmc here
    void iterate();
    inline int get_iterations() const {return iterations;}
    // execute 
    int8_t operator()(input in);


};