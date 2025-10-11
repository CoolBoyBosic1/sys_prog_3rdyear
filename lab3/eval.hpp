#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <stdexcept>
#include <variant>
#include "ast.hpp"

struct Value {
    std::variant<double,bool> v;
    static Value num(double x){ return Value{ x }; }
    static Value boolean(bool b){ return Value{ b }; }
    explicit Value(double x): v(x){}
    explicit Value(bool b): v(b){}
    Value() : v(0.0){}
    double as_num() const { if(auto p=std::get_if<double>(&v)) return *p; if(auto q=std::get_if<bool>(&v)) return *q?1.0:0.0; return 0.0; }
    bool as_bool() const { if(auto p=std::get_if<bool>(&v)) return *p; if(auto q=std::get_if<double>(&v)) return *q!=0.0; return false; }
};

struct Env {
    std::vector<std::unordered_map<std::string,Value>> scopes;
    Env(){ scopes.emplace_back(); }
    void push(){ scopes.emplace_back(); }
    void pop(){ scopes.pop_back(); }
    bool decl(const std::string& name, const Value& v){ return scopes.back().emplace(name,v).second; }
    bool set(const std::string& name, const Value& v){
        for(auto it=scopes.rbegin(); it!=scopes.rend(); ++it){
            auto f=it->find(name); if(f!=it->end()){ f->second=v; return true; }
        } return false;
    }
    std::optional<Value> get(const std::string& name){
        for(auto it=scopes.rbegin(); it!=scopes.rend(); ++it){
            auto f=it->find(name); if(f!=it->end()) return f->second;
        } return std::nullopt;
    }
};

struct Func { FuncDefNode* def{}; };

struct World {
    Env env;
    std::unordered_map<std::string,Func> funcs;
    bool has_return=false; Value return_value;
};

/* ОГОЛОШЕННЯ — тепер приймаємо AST* */
void collect_functions(World& w, Program* p);
void exec_node(World& w, AST* n);
Value eval_expr(World& w, ExprNode* e);
Value call_func(World& w, const std::string& name, const std::vector<Value>& args);
