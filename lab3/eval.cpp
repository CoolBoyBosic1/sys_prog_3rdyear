#include "eval.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

void collect_functions(World& w, Program* p){
    for(auto* it : p->items){
        if(auto* f = dynamic_cast<FuncDefNode*>(it)){
            w.funcs[f->name] = Func{ f };
        }
    }
}

/* ГОЛОВНЕ: приймаємо AST* */
void exec_node(World& w, AST* n){
    if(w.has_return) return;

    if(auto* d = dynamic_cast<DeclNode*>(n)){
        Value init = d->init ? eval_expr(w, d->init) : Value::num(0);
        if(!w.env.decl(d->name, init)) throw std::runtime_error("Redeclaration: "+d->name);
        return;
    }
    if(auto* es = dynamic_cast<ExprStmtNode*>(n)){ eval_expr(w, es->expr); return; }
    if(auto* r = dynamic_cast<ReturnNode*>(n)){
        w.return_value = r->expr? eval_expr(w, r->expr): Value::num(0);
        w.has_return = true;
        return;
    }
    if(auto* iff = dynamic_cast<IfNode*>(n)){
        if(eval_expr(w, iff->cond).as_bool()){
            exec_node(w, iff->thenN);
        } else if(iff->elseN){
            exec_node(w, iff->elseN);
        }
        return;
    }
    if(auto* wh = dynamic_cast<WhileNode*>(n)){
        while(!w.has_return && eval_expr(w, wh->cond).as_bool()){
            exec_node(w, wh->body);
        }
        return;
    }
    if(auto* fr = dynamic_cast<ForNode*>(n)){
        if(fr->init) eval_expr(w, fr->init);
        while(!w.has_return && (!fr->cond || eval_expr(w, fr->cond).as_bool())){
            exec_node(w, fr->body);
            if(fr->step) eval_expr(w, fr->step);
        }
        return;
    }
    if(auto* bl = dynamic_cast<BlockNode*>(n)){
        w.env.push();
        for(auto* s: bl->stmts){
            exec_node(w, s);
            if(w.has_return) break;
        }
        w.env.pop();
        return;
    }
}

static Value apply_bin(const std::string& op, const Value& A, const Value& B){
    if(op=="+") return Value::num(A.as_num()+B.as_num());
    if(op=="-") return Value::num(A.as_num()-B.as_num());
    if(op=="*") return Value::num(A.as_num()*B.as_num());
    if(op=="/") return Value::num(A.as_num()/B.as_num());
    if(op=="%") return Value::num(std::fmod(A.as_num(),B.as_num()));
    if(op=="<") return Value::boolean(A.as_num()<B.as_num());
    if(op==">") return Value::boolean(A.as_num()>B.as_num());
    if(op=="<=") return Value::boolean(A.as_num()<=B.as_num());
    if(op==">=") return Value::boolean(A.as_num()>=B.as_num());
    if(op=="==") return Value::boolean(A.as_num()==B.as_num());
    if(op=="!=") return Value::boolean(A.as_num()!=B.as_num());
    if(op=="&&") return Value::boolean(A.as_bool() && B.as_bool());
    if(op=="||") return Value::boolean(A.as_bool() || B.as_bool());
    throw std::runtime_error("Unknown binop: "+op);
}

static Value apply_un(const std::string& op, const Value& X){
    if(op=="!") return Value::boolean(!X.as_bool());
    if(op=="neg") return Value::num(-X.as_num());
    throw std::runtime_error("Unknown unop: "+op);
}

Value eval_expr(World& w, ExprNode* e){
    if(auto* a = dynamic_cast<AssignNode*>(e)){
        Value v = eval_expr(w, a->rhs);
        if(!w.env.set(a->name, v)) throw std::runtime_error("Undeclared: "+a->name);
        return v;
    }
    if(auto* b = dynamic_cast<BinOpNode*>(e)){
        auto L = eval_expr(w, b->a);
        if(b->op=="&&"){
            if(!L.as_bool()) return Value::boolean(false);
            return Value::boolean(eval_expr(w,b->b).as_bool());
        }
        if(b->op=="||"){
            if(L.as_bool()) return Value::boolean(true);
            return Value::boolean(eval_expr(w,b->b).as_bool());
        }
        auto R = eval_expr(w, b->b);
        return apply_bin(b->op, L, R);
    }
    if(auto* u = dynamic_cast<UnaryOpNode*>(e)) return apply_un(u->op, eval_expr(w,u->x));
    if(auto* n = dynamic_cast<NumberNode*>(e)) return Value::num(n->v);
    if(auto* b = dynamic_cast<BoolNode*>(e)) return Value::boolean(b->v);
    if(auto* v = dynamic_cast<VarRefNode*>(e)){
        auto got = w.env.get(v->name);
        if(!got) throw std::runtime_error("Undeclared: "+v->name);
        return *got;
    }
    if(auto* c = dynamic_cast<CallNode*>(e)){
        std::vector<Value> args;
        if(c->args){
            for(auto* ex: c->args->args) args.push_back(eval_expr(w, ex));
        }
        return call_func(w, c->name, args);
    }
    throw std::runtime_error("Unknown expr node");
}

Value call_func(World& w, const std::string& name, const std::vector<Value>& args){
    auto it = w.funcs.find(name);
    if(it==w.funcs.end()) throw std::runtime_error("Unknown function: "+name);
    FuncDefNode* f = it->second.def;
    size_t n = f->params? f->params->params.size() : 0;
    if(n != args.size()) throw std::runtime_error("Arity mismatch in "+name);
    w.env.push();
    for(size_t i=0;i<n;++i){ w.env.decl(f->params->params[i]->name, args[i]); }
    bool prev_ret = w.has_return; Value prev_val = w.return_value; w.has_return=false;
    exec_node(w, f->body);
    Value ret = w.return_value; bool had = w.has_return;
    w.has_return = prev_ret; w.return_value = prev_val;
    w.env.pop();
    if(had) return ret;
    return Value::num(0);
}
