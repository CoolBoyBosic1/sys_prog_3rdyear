#include "gen_c.hpp"
#include <sstream>
#include <typeinfo>

static void emit_node(std::ostringstream& out, AST* n, int ind);
static void emit_block(std::ostringstream& out, BlockNode* b, int ind);

static void indn(std::ostringstream& out, int n){ while(n--) out << "  "; }

static void emit_expr(std::ostringstream& out, AST* n){
  if(auto* k = dynamic_cast<NumberNode*>(n)){ out << k->v; return; }
  if(auto* b = dynamic_cast<BoolNode*>(n)){ out << (b->v ? "1" : "0"); return; }
  if(auto* v = dynamic_cast<VarRefNode*>(n)){ out << v->name; return; }
  if(auto* u = dynamic_cast<UnaryOpNode*>(n)){
    if(u->op == "neg"){ out << "-("; emit_expr(out, u->x); out << ")"; }
    else { out << u->op << "("; emit_expr(out, u->x); out << ")"; }
    return;
  }
  if(auto* a = dynamic_cast<AssignNode*>(n)){
    out << a->name << " = "; emit_expr(out, a->rhs); return;
  }
  if(auto* b = dynamic_cast<BinOpNode*>(n)){
    out << "("; emit_expr(out, b->a); out << " " << b->op << " "; emit_expr(out, b->b); out << ")"; return;
  }
  if(auto* c = dynamic_cast<CallNode*>(n)){
    out << c->name << "(";
    if(c->args){
      for(size_t i=0;i<c->args->args.size();++i){
        if(i) out << ", ";
        emit_expr(out, c->args->args[i]);
      }
    }
    out << ")"; return;
  }
  out << "0"; // fallback
}

static std::string c_type(const std::string& t){
  if(t=="int") return "int";
  if(t=="double") return "double";
  if(t=="bool") return "int"; // bool -> int
  return "int";
}

static void emit_block(std::ostringstream& out, BlockNode* b, int ind){
  indn(out,ind); out << "{\n";
  for(auto* s : b->stmts) emit_node(out, s, ind+1);
  indn(out,ind); out << "}\n";
}

static void emit_node(std::ostringstream& out, AST* n, int ind){
  if(auto* d = dynamic_cast<DeclNode*>(n)){
    indn(out,ind); out << c_type(d->type) << " " << d->name;
    if(d->init){ out << " = "; emit_expr(out, d->init); }
    out << ";\n"; return;
  }
  if(auto* es = dynamic_cast<ExprStmtNode*>(n)){
    indn(out,ind); emit_expr(out, es->expr); out << ";\n"; return;
  }
  if(auto* r = dynamic_cast<ReturnNode*>(n)){
    indn(out,ind); out << "return "; if(r->expr) emit_expr(out, r->expr); else out << "0"; out << ";\n"; return;
  }
  if(auto* iff = dynamic_cast<IfNode*>(n)){
    indn(out,ind); out << "if ("; emit_expr(out, iff->cond); out << ")\n";
    emit_node(out, iff->thenN, ind);
    if(iff->elseN){ indn(out,ind); out << "else\n"; emit_node(out, iff->elseN, ind); }
    return;
  }
  if(auto* wh = dynamic_cast<WhileNode*>(n)){
    indn(out,ind); out << "while ("; emit_expr(out, wh->cond); out << ")\n";
    emit_node(out, wh->body, ind); return;
  }
  if(auto* fr = dynamic_cast<ForNode*>(n)){
    indn(out,ind); out << "for (";
    if(fr->init) emit_expr(out, fr->init); out << "; ";
    if(fr->cond) emit_expr(out, fr->cond); out << "; ";
    if(fr->step) emit_expr(out, fr->step); out << ")\n";
    emit_node(out, fr->body, ind); return;
  }
  if(auto* bl = dynamic_cast<BlockNode*>(n)){ emit_block(out, bl, ind); return; }

  indn(out,ind); emit_expr(out, n); out << ";\n";
}

std::string gen_c_code(Program* p){
  std::ostringstream out;
  out << "#include <stdio.h>\n\n";

  // forward-декларації
  for(auto* it : p->items){
    if(auto* f = dynamic_cast<FuncDefNode*>(it)){
      out << c_type(f->retType) << " " << f->name << "(";
      size_t n = f->params? f->params->params.size() : 0;
      for(size_t i=0;i<n;++i){
        if(i) out << ", ";
        auto* pr = f->params->params[i];
        out << c_type(pr->type) << " " << pr->name;
      }
      out << ");\n";
    }
  }
  out << "\n";

  // тіла функцій
  for(auto* it : p->items){
    if(auto* f = dynamic_cast<FuncDefNode*>(it)){
      out << c_type(f->retType) << " " << f->name << "(";
      size_t n = f->params? f->params->params.size() : 0;
      for(size_t i=0;i<n;++i){
        if(i) out << ", ";
        auto* pr = f->params->params[i];
        out << c_type(pr->type) << " " << pr->name;
      }
      out << ")\n";
      emit_block(out, f->body, 0);
      out << "\n";
    }
  }
  return out.str();
}
