#include "ast.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <sstream>
std::vector<term_node*> ast::int_choices;
std::vector<term_node*> ast::str_choices;

int8_t ast_node::execute(input in) const{
    switch(ntype){
        case OP:{
            const operation_node* op = dynamic_cast<const operation_node*>(this);
            switch(op->otype){
                case PLUS:{
                    const binary_operation_node* plus  = dynamic_cast<const binary_operation_node*>(op);
                    return plus->left->execute(in) + plus->right->execute(in);
                }
                case MULTIPLY:{
                    const binary_operation_node* mult  = dynamic_cast<const binary_operation_node*>(op);
                    return mult->left->execute(in) * mult->right->execute(in);
                }
                case MINUS:{
                    const binary_operation_node* minus  = dynamic_cast<const binary_operation_node*>(op);
                    return minus->left->execute(in) - minus->right->execute(in);
                }
                case DIVIDE:{
                    const binary_operation_node* div  = dynamic_cast<const binary_operation_node*>(op);
                    int8_t right = div->right->execute(in);
                    if(right == 0)
                        return 0;
                    return div->left->execute(in) / right;
                }
                case MOD:{
                    const binary_operation_node* mod  = dynamic_cast<const binary_operation_node*>(op);
                    int8_t right = mod->right->execute(in);
                    if(right == 0)
                        return 0;
                    return mod->left->execute(in) % right;
                }
                case LEN:{
                    const unary_operation_node* len = dynamic_cast<const unary_operation_node*>(op);
                    term_node* term = dynamic_cast<term_node*>(len->child);

                    // no const strings in this program
                    var_node* var = dynamic_cast<var_node*>(term);
                    return std::get<std::string>(in.vars[var->varname]).length();
                }
                case SUM:{
                    const unary_operation_node* sum = dynamic_cast<const unary_operation_node*>(op);
                    term_node* term = dynamic_cast<term_node*>(sum->child);
                    // no const strings in this program
                    var_node* var = dynamic_cast<var_node*>(term);
                    std::string str = std::get<std::string>(in.vars[var->varname]);
                    int8_t out = 0;
                    for(int i = 0; i < str.length(); i++){
                        out += str[i];
                    }
                    return out;
                }
            }
        }
        case TERM:{
            const term_node* term = dynamic_cast<const term_node*>(this);
            switch(term->ttype){
                case CONST:
                    return dynamic_cast<const const_node*>(term)->value;
                case VAR:{
                    const var_node* var = dynamic_cast<const var_node*>(term);
                    return std::get<int8_t>(in.vars[var->varname]);
                }
            }
        }
    }
}

/** Choose a random op node and change it to another type of op node*/
void ast::swap_op(){
    if(!operations.size()) 
        return;
    
    int op_idx = rand()%operations.size();
    switch(operations[op_idx]->otype){
        case PLUS:
        case MULTIPLY:
        case MINUS:
        case DIVIDE:
        case MOD:{
            // binary operation
            op_type type = operations[op_idx]->otype;
            std::vector<op_type> types = {PLUS, MULTIPLY, MINUS, DIVIDE, MOD};
            while(operations[op_idx]->otype == type){
                operations[op_idx]->otype = types[rand()%types.size()];
            }
            return;
        }
        case SUM:
        case LEN:{
            // unary operation
            if(operations[op_idx]->otype == LEN){
                operations[op_idx]->otype = SUM;
                assert(dynamic_cast<unary_operation_node*>(operations[op_idx])->child);
            }else{
                operations[op_idx]->otype = LEN;
                assert(dynamic_cast<unary_operation_node*>(operations[op_idx])->child);
            }
            return;
        }
    }
}

/** Choose a leaf value and replace it with another leaf value*/
void ast::swap_leaf_value(){
    int idx = rand()%terms.size();

    switch(terms[idx]->ttype){
        case CONST:{
            // change integer only to var integer or diff integer
            term_node* old_term = terms[idx];
            term_node* new_term = old_term;

            // find a new replacement
            while(*new_term == *old_term){
                new_term = int_choices[rand()%int_choices.size()];
            }

            // copy and create new term/leaf
            switch(new_term->ttype){
                case CONST:{
                    new_term = const_node::cpy(dynamic_cast<const_node*>(new_term));
                    break;
                }
                case VAR:{
                    new_term = var_node::cpy(dynamic_cast<var_node*>(new_term));
                    break;
                }
            }

            // set parent
            if(old_term->parent == nullptr){
                root = new_term;
                new_term->parent = nullptr;
            }else{
                binary_operation_node* parent = dynamic_cast<binary_operation_node*>(old_term->parent);
                if(parent->left == old_term){
                    parent->left = new_term;
                }else{
                    parent->right = new_term;
                }
                new_term->parent = parent;
            }

            // replace term in terms
            terms.erase(remove(terms.begin(), terms.end(), old_term), terms.end());
            terms.push_back(new_term);

            // delete old term/leaf
            delete old_term;
            break;
        }
        case VAR:{
            // if integer, change to var integer or diff integer
            // if string, change to string
            bool isString = false;
            var_node* var = dynamic_cast<var_node*>(terms[idx]);
            for(term_node* term: str_choices){
                var_node* str_var = dynamic_cast<var_node*>(term);
                if(str_var->varname == var->varname){
                    isString = true;
                    break;
                }
            }

            if(isString){
                term_node* old_term = terms[idx];
                term_node* new_term = old_term;

                // can't swap the string
                if(str_choices.size() == 1)
                    return;

                // find a new replacement
                while(*new_term == *old_term){
                    new_term = str_choices[rand()%str_choices.size()];
                }
                new_term = var_node::cpy(dynamic_cast<var_node*>(new_term));

                // set parent
                unary_operation_node* parent = dynamic_cast<unary_operation_node*>(old_term->parent);
                parent->child = new_term;
                new_term->parent = parent;

                // replace term in terms
                terms.erase(remove(terms.begin(), terms.end(), old_term), terms.end());
                terms.push_back(new_term);

                //delete old term/leaf
                delete old_term;
            }else{
                // replace the var int with a const or different var
                term_node* old_term= terms[idx];
                term_node* new_term=old_term;
                
                // find a new replacement
                while(*new_term == *old_term){
                    new_term = int_choices[rand()%int_choices.size()];
                }

                // copy and create new term/leaf
                switch(new_term->ttype){
                    case CONST:
                        new_term = const_node::cpy(dynamic_cast<const_node*>(new_term));
                        break;
                    case VAR:
                        new_term = var_node::cpy(dynamic_cast<var_node*>(new_term));
                        break;
                }

                // set parent
                if(old_term->parent == nullptr){
                    root = new_term;
                    new_term->parent = nullptr;
                }else{
                    binary_operation_node* parent = dynamic_cast<binary_operation_node*>(old_term->parent);
                    if(parent->left == old_term){
                        parent->left = new_term;
                    }else{
                        parent->right = new_term;
                    }
                    new_term->parent = parent;
                }

                // replace term in terms
                terms.erase(remove(terms.begin(), terms.end(), terms[idx]), terms.end());
                terms.push_back(new_term);

                // delete old term/leaf
                delete old_term;
            }
            break;
        }
    }
}

void ast::swap_leaf_op(double& opt_forward, double& opt_backward){
    size_t total_choices = leaved_operations.size() + terms.size();
    size_t idx = rand() % total_choices;
    if(idx < leaved_operations.size()){
        // op to leaf
        opt_forward = 1.0f/((double)leaved_operations.size());
        opt_backward = 1.0f/((double)int_choices.size());
        // get op
        operation_node* op_node = leaved_operations[idx];

        // remove children from terms
        switch(op_node->otype){
            case PLUS:
            case MINUS:
            case MULTIPLY:
            case DIVIDE:
            case MOD:{
                binary_operation_node* binop_node = dynamic_cast<binary_operation_node*>(op_node);
                terms.erase(remove(terms.begin(), terms.end(), binop_node->left), terms.end());
                terms.erase(remove(terms.begin(), terms.end(), binop_node->right), terms.end());
                delete binop_node->left;
                delete binop_node->right;
                break;
            }
            case LEN:
            case SUM:{
                unary_operation_node* unop_node = dynamic_cast<unary_operation_node*>(op_node);
                terms.erase(remove(terms.begin(), terms.end(), unop_node->child), terms.end());
                delete unop_node->child;
                break;
            }
        }

        // remove node from leaved_operations
        leaved_operations.erase(remove(leaved_operations.begin(), leaved_operations.end(), op_node), leaved_operations.end());
        operations.erase(remove(operations.begin(), operations.end(), op_node), operations.end());

        // get new term
        size_t term_idx = rand()%int_choices.size();
        term_node* new_term;
        switch(int_choices[term_idx]->ttype){
            case CONST:
                new_term = const_node::cpy(dynamic_cast<const_node*>(int_choices[term_idx]));
                break;
            case VAR:
                new_term = var_node::cpy(dynamic_cast<var_node*>(int_choices[term_idx]));
                break;
        }


        // set new term to op's parent's child if not null
        // if null, set root to term
        if(op_node->parent == nullptr){
            root = new_term;
            new_term->parent = nullptr;
        }else{
            binary_operation_node* parent = dynamic_cast<binary_operation_node*>(op_node->parent);
            if(parent->left == op_node){
                parent->left = new_term;
                if(parent->right->ntype == TERM){
                    leaved_operations.push_back(parent);
                }
            }else{
                parent->right = new_term;
                if(parent->left->ntype == TERM){
                    leaved_operations.push_back(parent);    
                }
            }
            new_term->parent = parent;
        }

        delete op_node;

        // add to leaves
        terms.push_back(new_term);
    }else{
        // leaf to op
        

        // get all nodes we can convert
        std::vector<term_node*> int_nodes;
        for(term_node* term: terms){
            if(term->ttype == CONST){
                int_nodes.push_back(term);
            }else{
                bool isString = false;
                var_node* var = dynamic_cast<var_node*>(term);
                for(term_node* str_term: str_choices){
                    var_node* str_var = dynamic_cast<var_node*>(str_term);
                    if(str_var->varname == var->varname){
                        isString = true;
                        break;
                    }
                }

                if(!isString){
                    int_nodes.push_back(term);
                }
                    
            }
        }

        // no leaves to change
        if(!int_nodes.size()){
            // no change
            opt_forward = 1;
            opt_backward = 1;
            return;
        }

        // get our leaf node
        term_node* term = int_nodes[rand()%int_nodes.size()];
        terms.erase(remove(terms.begin(), terms.end(), term), terms.end());

        std::vector<op_type> ops = {PLUS, MINUS, MULTIPLY, DIVIDE, MOD, LEN, SUM};
        op_type new_op_type = ops[rand()%ops.size()];
        while((new_op_type == LEN || new_op_type == SUM) && !str_choices.size()){
            new_op_type = ops[rand()%ops.size()];
        }

        opt_forward = 1.0f/((double)int_nodes.size());
        opt_backward = 1.0f/((double)ops.size());

        operation_node* op_node;
        switch(new_op_type){
            case PLUS:
            case MULTIPLY:
            case MINUS:
            case DIVIDE:
            case MOD:{
                op_node = new binary_operation_node();
                op_node->otype = new_op_type;

                // create left child: (var or const)
                term_node* left_term = int_choices[rand()%int_choices.size()];
                if(left_term->ttype == CONST)
                    left_term = const_node::cpy(dynamic_cast<const_node*>(left_term));
                else
                    left_term = var_node::cpy(dynamic_cast<var_node*>(left_term));

                // create right child: (var or const)
                term_node* right_term = int_choices[rand()%int_choices.size()];
                if(right_term->ttype == CONST)
                    right_term = const_node::cpy(dynamic_cast<const_node*>(right_term));
                else
                    right_term = var_node::cpy(dynamic_cast<var_node*>(right_term));

                dynamic_cast<binary_operation_node*>(op_node)->right = right_term;
                dynamic_cast<binary_operation_node*>(op_node)->left = left_term;
                right_term->parent = op_node;
                left_term->parent = op_node;

                // add children to terms
                terms.push_back(left_term);
                terms.push_back(right_term);
                assert(dynamic_cast<binary_operation_node*>(op_node)->right);
                assert(dynamic_cast<binary_operation_node*>(op_node)->left);

                break;
            }
            case LEN:
            case SUM:{
                op_node = new unary_operation_node();
                op_node->otype = new_op_type;

                var_node* str_var = var_node::cpy(dynamic_cast<var_node*>(str_choices[rand()%str_choices.size()]));
                dynamic_cast<unary_operation_node*>(op_node)->child = str_var;
                str_var->parent = op_node;
                terms.push_back(str_var);

                assert(dynamic_cast<unary_operation_node*>(op_node)->child);
                break;
            }
        }


        // set the parent
        if(term->parent == nullptr){
            root = op_node;
            op_node->parent = nullptr;
        }else{
            binary_operation_node* parent = dynamic_cast<binary_operation_node*>(term->parent);
            if(parent->left == term){
                parent->left = op_node;
                if(parent->right->ntype == TERM){
                    leaved_operations.erase(remove(leaved_operations.begin(), leaved_operations.end(), parent), leaved_operations.end());
                }
            }else{
                parent->right = op_node;
                if(parent->left->ntype == TERM){
                    leaved_operations.erase(remove(leaved_operations.begin(), leaved_operations.end(), parent), leaved_operations.end());
                }
            }
            op_node->parent = parent;
        }


        delete term;
        operations.push_back(op_node);
        leaved_operations.push_back(op_node);

    }
}


ast::ast(){
}

void ast::init(){
    term_node* root_candidate = int_choices.back();
    if(root_candidate->ttype == CONST)
        root = const_node::cpy(dynamic_cast<const_node*>(root_candidate));
    else
        root = var_node::cpy(dynamic_cast<var_node*>(root_candidate));
    terms.push_back(dynamic_cast<term_node*>(root));
    assert(terms.size() == 1);
}

ast::~ast(){
    delete root;
}

binary_operation_node::~binary_operation_node(){
    delete left;
    delete right;
}

unary_operation_node::~unary_operation_node(){
    delete child;
}

ast* ast::copy() const{
    ast* cpy = new ast();
    cpy->root = root->copy(cpy->operations, cpy->leaved_operations, cpy->terms);
    return cpy;
}


ast_node* ast_node::copy(
    std::vector<operation_node*>& operations,
    std::vector<operation_node*>& leaved_operations,
    std::vector<term_node*>& terms
) const{
    switch(ntype){
        case OP:{
            const operation_node* opnode = dynamic_cast<const operation_node*>(this);
            switch(opnode->otype){
                case PLUS:
                case MINUS:
                case MULTIPLY:
                case DIVIDE:
                case MOD:{
                    const binary_operation_node* binnode = dynamic_cast<const binary_operation_node*>(this);
                    binary_operation_node* cpy = new binary_operation_node();
                    ast_node* left = binnode->left->copy(operations, leaved_operations, terms);
                    ast_node* right = binnode->right->copy(operations, leaved_operations, terms);
                    left->parent = cpy;
                    right->parent = cpy;
                    cpy->left = left;
                    cpy->right = right;
                    cpy->otype = binnode->otype;
                    cpy->ntype = binnode->ntype;
                    operations.push_back(cpy);
                    if(left->ntype == TERM && right->ntype == TERM){
                        leaved_operations.push_back(cpy);
                    }
                    return cpy;
                }
                case LEN:
                case SUM:{
                    const unary_operation_node* unnode = dynamic_cast<const unary_operation_node*>(this);
                    unary_operation_node* cpy = new unary_operation_node();
                    ast_node* child = unnode->child->copy(operations, leaved_operations, terms);
                    child->parent = cpy;
                    cpy->child = child;
                    cpy->otype = unnode->otype;
                    cpy->ntype = unnode->ntype;
                    operations.push_back(cpy);
                    leaved_operations.push_back(cpy);
                    return cpy;
                }
            }
        }
        case TERM:{
            const term_node* tnode = dynamic_cast<const term_node*>(this);
            switch(tnode->ttype){
                case CONST:{
                    const const_node* cnode = dynamic_cast<const const_node*>(tnode);
                    const_node* cpy = new const_node();
                    cpy->value = cnode->value;
                    cpy->ttype = cnode->ttype;
                    cpy->ntype = cnode->ntype;
                    terms.push_back(cpy);
                    return cpy;
                }
                case VAR:{
                    const var_node* varnode = dynamic_cast<const var_node*>(tnode);
                    var_node* cpy = new var_node();
                    cpy->varname = varnode->varname;
                    cpy->ttype = varnode->ttype;
                    cpy->ntype = varnode->ntype;
                    terms.push_back(cpy);
                    return cpy;
                }
            }
        }
    }
}

std::string ast_node::print(){
    std::stringstream os;
    switch(ntype){
        case OP:{
            operation_node& on = dynamic_cast<operation_node&>(*this);
            switch(on.otype){
                case LEN:
                case SUM:{
                    unary_operation_node& node = dynamic_cast<unary_operation_node&>(on);
                    os << "UN  NODE: " << node.child;
                    os << " parent: " << node.parent;
                    os << " ntype: " << node.ntype;
                    os << " otype: " << node.otype;
                    os << std::endl;
                    os << node.child->print();
                    break;
                }
                case PLUS:
                case MULTIPLY:
                case DIVIDE:
                case MINUS:
                case MOD:{
                    binary_operation_node& node = dynamic_cast<binary_operation_node&>(on);
                    os << "BIN  NODE: " << node.left << ", " << node.right;
                    os << " parent: " << node.parent;
                    os << " ntype: " << node.ntype;
                    os << " otype: " << node.otype;
                    os << std::endl;
                    os << node.left->print();
                    os << node.right->print();
                    break;
                }
            }
            break;
        }
        case TERM:{
            term_node& tn = dynamic_cast<term_node&>(*this);
            switch(tn.ttype){
                case VAR:{
                    var_node& node = dynamic_cast<var_node&>(tn);
                    os << "VAR: " << node.ttype;
                    os << " parent: " << node.parent;
                    os << " ntype: " << node.ntype;
                    os << " varname: " << node.varname;
                    os << std::endl;
                    break;
                }
                case CONST:{
                    const_node& node = dynamic_cast<const_node&>(tn);
                    os << "CONST: " << node.ttype;
                    os << " parent: " << node.parent;
                    os << " ntype: " << node.ntype;
                    os << " value: " << (int64_t) node.value;
                    os << std::endl;
                    break;
                }
            }
            break;
        }
    }
    return os.str();
}