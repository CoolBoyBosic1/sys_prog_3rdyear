#pragma once
#include <string>
#include <vector>
#include <memory>

struct AST { virtual ~AST() = default; };

template<typename T> inline T* as(AST* n){ return static_cast<T*>(n); }

struct Node : AST {};
struct ExprNode : Node {};

struct TypeNode : Node {
    std::string name; explicit TypeNode(std::string n): name(std::move(n)){}
};

struct VecNode : Node { std::vector<AST*> items; };

struct Program : Node { std::vector<AST*> items; };

struct ParamNode : Node {
    std::string type, name;
    ParamNode(std::string t, std::string n): type(std::move(t)), name(std::move(n)){}
};

struct ParamListNode : Node { std::vector<ParamNode*> params; };

struct BlockNode : Node {
    std::vector<AST*> stmts; explicit BlockNode(VecNode* v){ if(v){ stmts = std::move(v->items);} }
};

struct DeclNode : Node {
    std::string type, name; ExprNode* init;
    DeclNode(std::string t, std::string n, ExprNode* i): type(std::move(t)), name(std::move(n)), init(i){}
};

struct ExprStmtNode : Node { ExprNode* expr; explicit ExprStmtNode(ExprNode* e): expr(e){} };

struct AssignNode : ExprNode { std::string name; ExprNode* rhs; AssignNode(std::string n, ExprNode* r): name(std::move(n)), rhs(r){} };

struct BinOpNode : ExprNode { std::string op; ExprNode* a; ExprNode* b; BinOpNode(std::string o, ExprNode* A, ExprNode* B): op(std::move(o)), a(A), b(B){} };

struct UnaryOpNode : ExprNode { std::string op; ExprNode* x; UnaryOpNode(std::string o, ExprNode* X): op(std::move(o)), x(X){} };

struct NumberNode : ExprNode { double v; explicit NumberNode(double V): v(V){} };

struct BoolNode : ExprNode { bool v; explicit BoolNode(bool V): v(V){} };

struct VarRefNode : ExprNode { std::string name; explicit VarRefNode(std::string n): name(std::move(n)){} };

struct ReturnNode : Node { ExprNode* expr; explicit ReturnNode(ExprNode* e): expr(e){} };

struct IfNode : Node { ExprNode* cond; Node* thenN; Node* elseN; IfNode(ExprNode* c, Node* t, Node* e): cond(c), thenN(t), elseN(e){} };

struct WhileNode : Node { ExprNode* cond; Node* body; WhileNode(ExprNode* c, Node* b): cond(c), body(b){} };

struct ForNode : Node { ExprNode* init; ExprNode* cond; ExprNode* step; Node* body; ForNode(ExprNode* i, ExprNode* c, ExprNode* s, Node* b): init(i), cond(c), step(s), body(b){} };

struct ArgListNode : Node { std::vector<ExprNode*> args; };

struct CallNode : ExprNode {
    std::string name; ArgListNode* args;
    CallNode(std::string n, ArgListNode* a): name(std::move(n)), args(a){}
};

struct FuncDefNode : Node {
    std::string retType, name; ParamListNode* params; BlockNode* body;
    FuncDefNode(std::string r, std::string n, ParamListNode* p, BlockNode* b): retType(std::move(r)), name(std::move(n)), params(p), body(b){}
};
