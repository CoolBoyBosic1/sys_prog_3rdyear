#include "ast_dot.hpp"
#include <typeinfo>
#include <sstream>

static int gid=0;
static std::ostringstream oss;

static int emit(AST* n){ return ++gid; }

static void walk(AST* n, int id){
    if(!n) return;
    auto label = std::string(typeid(*n).name());
    if(auto* t=dynamic_cast<TypeNode*>(n)) label = "Type:"+t->name;
    if(auto* d=dynamic_cast<DeclNode*>(n)) label = "Decl:"+d->name+" :"+d->type;
    if(auto* v=dynamic_cast<VarRefNode*>(n)) label = "Var:"+v->name;
    if(auto* nb=dynamic_cast<NumberNode*>(n)) label = "Num:"+std::to_string(nb->v);
    if(auto* bb=dynamic_cast<BoolNode*>(n)) label = std::string("Bool:")+(bb->v?"true":"false");
    if(auto* bo=dynamic_cast<BinOpNode*>(n)) label = "Bin:"+bo->op;
    if(auto* uo=dynamic_cast<UnaryOpNode*>(n)) label = "Un:"+uo->op;
    if(auto* asn=dynamic_cast<AssignNode*>(n)) label = "Assign:"+asn->name;
    if(auto* fn=dynamic_cast<FuncDefNode*>(n)) label = "Func:"+fn->name+" ->"+fn->retType;
    if(dynamic_cast<BlockNode*>(n)) label = "Block";
    if(dynamic_cast<IfNode*>(n)) label = "If";
    if(dynamic_cast<WhileNode*>(n)) label = "While";
    if(dynamic_cast<ForNode*>(n)) label = "For";
    if(dynamic_cast<ExprStmtNode*>(n)) label = "ExprStmt";
    if(dynamic_cast<Program*>(n)) label = "Program";
    if(auto* call = dynamic_cast<CallNode*>(n)) label = "Call:"+call->name;

    oss << "  n"<<id<<" [label=\""<<label<<"\"];\n";

    auto link=[&](AST* c){ if(!c) return; int cid=emit(c); oss<<"  n"<<id<<" -> n"<<cid<<";\n"; walk(c,cid); };

    if(auto* p=dynamic_cast<Program*>(n)){
        for(auto* it: p->items) link(it);
    } else if(auto* b=dynamic_cast<BlockNode*>(n)){
        for(auto* it: b->stmts) link(it);
    } else if(auto* d=dynamic_cast<DeclNode*>(n)){
        link(d->init);
    } else if(auto* es=dynamic_cast<ExprStmtNode*>(n)){
        link(es->expr);
    } else if(auto* as=dynamic_cast<AssignNode*>(n)){
        link(as->rhs);
    } else if(auto* bo=dynamic_cast<BinOpNode*>(n)){
        link(bo->a); link(bo->b);
    } else if(auto* uo=dynamic_cast<UnaryOpNode*>(n)){
        link(uo->x);
    } else if(auto* iff=dynamic_cast<IfNode*>(n)){
        link(iff->cond); link(iff->thenN); link(iff->elseN);
    } else if(auto* wh=dynamic_cast<WhileNode*>(n)){
        link(wh->cond); link(wh->body);
    } else if(auto* fr=dynamic_cast<ForNode*>(n)){
        link(fr->init); link(fr->cond); link(fr->step); link(fr->body);
    } else if(auto* fd=dynamic_cast<FuncDefNode*>(n)){
        if(fd->params){ for(auto* p: fd->params->params) link(p); }
        link(fd->body);
    } else if(auto* call=dynamic_cast<CallNode*>(n)){
        if(call->args){ for(auto* e: call->args->args) link(e); }
    } else if(auto* pr=dynamic_cast<ParamNode*>(n)){
        (void)pr;
    } else if(auto* rt=dynamic_cast<ReturnNode*>(n)){
        link(rt->expr);
    }
}

std::string ast_to_dot(AST* root){
    gid=0; oss.str(""); oss.clear();
    oss << "digraph AST {\n  node [shape=box, fontname=Courier];\n";
    int id=emit(root); walk(root,id);
    oss << "}\n";
    return oss.str();
}
