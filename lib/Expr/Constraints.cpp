//===-- Constraints.cpp ---------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Expr/Constraints.h"

#include "klee/Expr/ExprVisitor.h"
#include "klee/Module/KModule.h"
#include "klee/Support/OptionCategories.h"
#include "klee/Support/ErrorHandling.h"

#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"

#include <map>
using namespace klee;

namespace {
llvm::cl::opt<bool> RewriteEqualities(
    "rewrite-equalities",
    llvm::cl::desc("Rewrite existing constraints when an equality with a "
                   "constant is added (default=true)"),
    llvm::cl::init(true),
    llvm::cl::cat(SolvingCat));

llvm::cl::opt<bool> SymSimConstraints(
    "symsim-constraints",
    llvm::cl::desc("Rewrite constraints with a Eq-Select construct and concrete"
                   " values (most commonly from SymSim) (default=false)"),
    llvm::cl::init(false),
    llvm::cl::cat(SolvingCat));
} // namespace

class ExprReplaceVisitor : public ExprVisitor {
private:
  ref<Expr> src, dst;

public:
  ExprReplaceVisitor(const ref<Expr> &_src, const ref<Expr> &_dst)
      : src(_src), dst(_dst) {}

  Action visitExpr(const Expr &e) override {
    if (e == *src) {
      return Action::changeTo(dst);
    }
    return Action::doChildren();
  }

  Action visitExprPost(const Expr &e) override {
    if (e == *src) {
      return Action::changeTo(dst);
    }
    return Action::doChildren();
  }
};

class ExprReplaceVisitor2 : public ExprVisitor {
private:
  const std::map< ref<Expr>, ref<Expr> > &replacements;

public:
  explicit ExprReplaceVisitor2(
      const std::map<ref<Expr>, ref<Expr>> &_replacements)
      : ExprVisitor(true), replacements(_replacements) {}

  Action visitExprPost(const Expr &e) override {
    auto it = replacements.find(ref<Expr>(const_cast<Expr *>(&e)));
    if (it!=replacements.end()) {
      return Action::changeTo(it->second);
    }
    return Action::doChildren();
  }
};

class ExprSymSimVisitor : public ExprVisitor {
private:
  bool valid=false; // have a concrete non-bool eq?
  ref<Expr> eq; // concrete non-bool eq
  bool invert=false; // was there also a concrete bool eq before!! non-bool?

  ref<Expr> select_cond; // condition to replace expression with
  bool found=false; // found the wanted pattern

public:
  ExprSymSimVisitor() {}

  ref<Expr> visitActual(const ref<Expr> &e) override {
    if (isa<ConstantExpr>(e)) {
      return e;
    } else {
      Expr &ep = *e.get();

      Action res = visitExpr(ep);
      switch(res.kind) {
      case Action::DoChildren:
        // continue with normal action
        break;
      case Action::SkipChildren:
        return e;
      case Action::ChangeTo:
        return res.argument;
      }

      switch(ep.getKind()) {
      case Expr::Select: res = visitSelect(static_cast<SelectExpr&>(ep)); break;
      case Expr::Eq: res = visitEq(static_cast<EqExpr&>(ep)); found=false; break;
      case Expr::NotOptimized: res = visitNotOptimized(static_cast<NotOptimizedExpr&>(ep)); valid=false; found=false; break;
      case Expr::Read: res = visitRead(static_cast<ReadExpr&>(ep)); valid=false; found=false; break;
      case Expr::Concat: res = visitConcat(static_cast<ConcatExpr&>(ep)); valid=false; found=false; break;
      case Expr::Extract: res = visitExtract(static_cast<ExtractExpr&>(ep)); valid=false; found=false; break;
      case Expr::ZExt: res = visitZExt(static_cast<ZExtExpr&>(ep)); valid=false; found=false; break;
      case Expr::SExt: res = visitSExt(static_cast<SExtExpr&>(ep)); valid=false; found=false; break;
      case Expr::Add: res = visitAdd(static_cast<AddExpr&>(ep)); valid=false; found=false; break;
      case Expr::Sub: res = visitSub(static_cast<SubExpr&>(ep)); valid=false; found=false; break;
      case Expr::Mul: res = visitMul(static_cast<MulExpr&>(ep)); valid=false; found=false; break;
      case Expr::UDiv: res = visitUDiv(static_cast<UDivExpr&>(ep)); valid=false; found=false; break;
      case Expr::SDiv: res = visitSDiv(static_cast<SDivExpr&>(ep)); valid=false; found=false; break;
      case Expr::URem: res = visitURem(static_cast<URemExpr&>(ep)); valid=false; found=false; break;
      case Expr::SRem: res = visitSRem(static_cast<SRemExpr&>(ep)); valid=false; found=false; break;
      case Expr::Not: res = visitNot(static_cast<NotExpr&>(ep)); valid=false; found=false; break;
      case Expr::And: res = visitAnd(static_cast<AndExpr&>(ep)); valid=false; found=false; break;
      case Expr::Or: res = visitOr(static_cast<OrExpr&>(ep)); valid=false; found=false; break;
      case Expr::Xor: res = visitXor(static_cast<XorExpr&>(ep)); valid=false; found=false; break;
      case Expr::Shl: res = visitShl(static_cast<ShlExpr&>(ep)); valid=false; found=false; break;
      case Expr::LShr: res = visitLShr(static_cast<LShrExpr&>(ep)); valid=false; found=false; break;
      case Expr::AShr: res = visitAShr(static_cast<AShrExpr&>(ep)); valid=false; found=false; break;
      case Expr::Ne: res = visitNe(static_cast<NeExpr&>(ep)); valid=false; found=false; break;
      case Expr::Ult: res = visitUlt(static_cast<UltExpr&>(ep)); valid=false; found=false; break;
      case Expr::Ule: res = visitUle(static_cast<UleExpr&>(ep)); valid=false; found=false; break;
      case Expr::Ugt: res = visitUgt(static_cast<UgtExpr&>(ep)); valid=false; found=false; break;
      case Expr::Uge: res = visitUge(static_cast<UgeExpr&>(ep)); valid=false; found=false; break;
      case Expr::Slt: res = visitSlt(static_cast<SltExpr&>(ep)); valid=false; found=false; break;
      case Expr::Sle: res = visitSle(static_cast<SleExpr&>(ep)); valid=false; found=false; break;
      case Expr::Sgt: res = visitSgt(static_cast<SgtExpr&>(ep)); valid=false; found=false; break;
      case Expr::Sge: res = visitSge(static_cast<SgeExpr&>(ep)); valid=false; found=false; break;
      case Expr::Constant:
      default:
        assert(0 && "invalid expression kind");
      }

      if(found) {
        return select_cond;
      }

      switch(res.kind) {
      default:
        assert(0 && "invalid kind");
      case Action::DoChildren: {
        bool rebuild = false;
        ref<Expr> e(&ep), kids[8];
        unsigned count = ep.getNumKids();
        for (unsigned i=0; i<count; i++) {
          ref<Expr> kid = ep.getKid(i);
          kids[i] = visit(kid);
          if(found) {
            return select_cond;
          }
          if (kids[i] != kid)
            rebuild = true;
        }
        if (rebuild) {
          e = ep.rebuild(kids);
          if (recursive)
            e = visit(e);
        }
        if (!isa<ConstantExpr>(e)) {
          res = visitExprPost(*e.get());
          if (res.kind==Action::ChangeTo)
            e = res.argument;
        }
        return e;
      }
      case Action::SkipChildren:
        return e;
      case Action::ChangeTo:
        return res.argument;
      }
    }
  }

  Action visitEq(const EqExpr& e) override {
    if(isa<ConstantExpr>(e.left) && e.left->getWidth()!=Expr::Bool) {
      eq = e.left;
      valid = true;
    } else if(isa<ConstantExpr>(e.left) && !valid) {
      invert = e.left->isFalse();
      valid = true;
    } else {
      invert = false;
      valid = false;
    }
    // TODO: what if concrete value is in e.right? would not change much...
    return Action::doChildren();
  }

  Action visitSelect(const SelectExpr& e) override {
    // TODO: what if only one is concrete (can still be helpful)
    if(valid && isa<ConstantExpr>(e.trueExpr) && isa<ConstantExpr>(e.falseExpr)) {
      ref<Expr> res;
      if(eq == e.trueExpr) {
        if(!invert) {
          select_cond = e.cond;
          res = e.cond;
        } else {
          select_cond = EqExpr::createIsZero(e.cond);
        }
      } else if(eq == e.falseExpr) {
        if(!invert) {
          select_cond = EqExpr::createIsZero(e.cond);
        } else {
          select_cond = e.cond;
        }
        found = true;
      } else {
        found = false;
      }
    } else {
      found = false;
    }
    // reset Eq-found data, bc already processed that
    invert = false;
    valid = false;
    return Action::doChildren();
  }
};

bool ConstraintManager::rewriteConstraints(ExprVisitor &visitor) {
  ConstraintSet old;
  bool success = true;

  std::swap(constraints, old);
  for (auto &ce : old) {
    ref<Expr> e = visitor.visit(ce);

    if (e!=ce) {
      success = success && addConstraintInternal(e); // enable further reductions
    } else {
      constraints.push_back(ce);
    }
  }

  return success;
}

ref<Expr> ConstraintManager::simplifyExpr(const ConstraintSet &constraints,
                                          const ref<Expr> &e) {

  if (isa<ConstantExpr>(e))
    return e;

  std::map< ref<Expr>, ref<Expr> > equalities;

  for (auto &constraint : constraints) {
    if (const EqExpr *ee = dyn_cast<EqExpr>(constraint)) {
      if (isa<ConstantExpr>(ee->left)) {
        equalities.insert(std::make_pair(ee->right,
                                         ee->left));
      } else {
        equalities.insert(
            std::make_pair(constraint, ConstantExpr::alloc(1, Expr::Bool)));
      }
    } else {
      equalities.insert(
          std::make_pair(constraint, ConstantExpr::alloc(1, Expr::Bool)));
    }
  }

  return ExprReplaceVisitor2(equalities).visit(e);
}

bool ConstraintManager::addConstraintInternal(const ref<Expr> &e) {
  // rewrite any known equalities and split Ands into different conjuncts
  bool success = true;
  switch (e->getKind()) {
  case Expr::Constant:
    if(!cast<ConstantExpr>(e)->isTrue()) {
//      klee_warning("attempt to add invalid (false) constraint");
      success = false;
    }
    break;

    // split to enable finer grained independence and other optimizations
  case Expr::And: {
    BinaryExpr *be = cast<BinaryExpr>(e);
    success = addConstraintInternal(be->left);
    if(success)
      success = addConstraintInternal(be->right);
    break;
  }

  case Expr::Eq: {
    if (RewriteEqualities) {
      // XXX: should profile the effects of this and the overhead.
      // traversing the constraints looking for equalities is hardly the
      // slowest thing we do, but it is probably nicer to have a
      // ConstraintSet ADT which efficiently remembers obvious patterns
      // (byte-constant comparison).
      BinaryExpr *be = cast<BinaryExpr>(e);
      if (isa<ConstantExpr>(be->left)) {
	ExprReplaceVisitor visitor(be->right, be->left);
	success = rewriteConstraints(visitor);
      }
    }
    if(success)
      constraints.push_back(e);
    break;
  }

  default:
    constraints.push_back(e);
    break;
  }
  return success;
}

bool ConstraintManager::addConstraint(const ref<Expr> &e) {
  ref<Expr> symsimFolding = ExprSymSimVisitor().visit(e);
  ref<Expr> simplified = simplifyExpr(constraints, symsimFolding);
  return addConstraintInternal(simplified);
}

ConstraintManager::ConstraintManager(ConstraintSet &_constraints)
    : constraints(_constraints) {}

bool ConstraintSet::empty() const { return constraints.empty(); }

klee::ConstraintSet::constraint_iterator ConstraintSet::begin() const {
  return constraints.begin();
}

klee::ConstraintSet::constraint_iterator ConstraintSet::end() const {
  return constraints.end();
}

size_t ConstraintSet::size() const noexcept { return constraints.size(); }

void ConstraintSet::push_back(const ref<Expr> &e) { constraints.push_back(e); }
