#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdint.h>
#include <iostream>
#include <cassert>

struct input{
    std::map<std::string, std::variant<int8_t, std::string>> vars;
};

enum node_type{OP, TERM};

struct operation_node;
struct term_node;

struct ast_node{
    node_type ntype;
    ast_node* parent;

    ast_node* copy(
        std::vector<operation_node*>& operations,
        std::vector<operation_node*>& leaved_operations,
        std::vector<term_node*>& terms
    ) const; 
    virtual int8_t execute(const input in) const;
    std::string print();
};

enum op_type{PLUS, MINUS, MULTIPLY, DIVIDE, MOD, LEN, SUM};

struct operation_node: ast_node{
    op_type otype;

    operation_node(){
        ntype = OP;
    }
};

struct binary_operation_node: operation_node{
    ast_node* left;
    ast_node* right;
    ~binary_operation_node();
};

struct unary_operation_node: operation_node{
    ast_node* child;
    ~unary_operation_node();
};

enum term_type {CONST, VAR};

struct term_node: ast_node{
    term_type ttype;
    virtual bool operator==(const term_node& right) const = 0;
     
};

struct const_node: term_node{
    int8_t value;

    bool operator==(const term_node& right) const{
        return right.ttype == ttype && dynamic_cast<const const_node&>(right).value == value;
    }

    const_node(){
        ttype = CONST;
        ntype = TERM;
        parent = nullptr;
    }

    static const_node* cpy(const_node* og){
        const_node* cpy = new const_node();
        cpy->value = og->value;
        cpy->ttype = og->ttype;
        cpy->parent = og->parent;
        cpy->ntype = og->ntype;
        assert(og->ttype == CONST);
        return cpy;
    }

    
};

struct var_node: term_node{
    std::string varname;

    var_node(){
        ttype = VAR;
        ntype = TERM;
        parent = nullptr;
    }

    bool operator==(const term_node& right) const{
        return right.ttype == ttype && dynamic_cast<const var_node&>(right).varname == varname;
    }

    static var_node* cpy(var_node* og){
        var_node* cpy = new var_node();
        cpy->varname = og->varname;
        cpy->ttype = og->ttype;
        cpy->parent = og->parent;
        cpy->ntype = og->ntype;
        return cpy;
    }
};


struct ast{
    ast_node* root;
    std::vector<operation_node*> operations; // swap op
    std::vector<operation_node*> leaved_operations; // op 2 leaf
    std::vector<term_node*> terms; // swap leaf

    // pick terms from these choices
    static std::vector<term_node*> int_choices;
    static std::vector<term_node*> str_choices;

    ast();
    ~ast();

    void swap_op();
    void swap_leaf_value();
    void swap_leaf_op(double& opt_forward, double& opt_backward);

    void init();
    ast* copy() const;
    int8_t execute(const input in) const{
        return root->execute(in);
    }

    std::string print(){
        return root->print();
    }
    
};



