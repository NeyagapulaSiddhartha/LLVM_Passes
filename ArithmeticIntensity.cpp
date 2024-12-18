#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include <map>
#include <stack>
#include <vector>
#include <string>
#include <cstdlib> // for integral types
#include <cmath>  
using namespace llvm;

using namespace std;
namespace {
    struct ArithmeticIntensityPass : public PassInfoMixin<ArithmeticIntensityPass> {
        int arthemeticcount;
        int memorycount;
        
        std::pair<int, int> get_count(BasicBlock *BB) {
            int memorycount = 0;
            int arthemeticcount = 0;
            for (Instruction &I : *BB) {
                int flag = 0;
                for (Use &U : I.operands()) {
                    Value *Operand = U.get();

                    if (Operand->getType()->isFloatingPointTy()) {
                        flag = 1;
                        break; // No need to check other operands if one of them is a floating-point type
                    }
                }
                if (isa<StoreInst>(&I)) 
                    {
                        memorycount++;
                    } 
                if (isa<LoadInst>(&I)) 
                    {
                        memorycount++;
                    }
                if (flag) {
                    if (I.getOpcode() == Instruction::FAdd ||
                            I.getOpcode() == Instruction::FMul ||
                            I.getOpcode() == Instruction::FDiv ||
                            I.getOpcode() == Instruction::FSub  ||
                            I.getOpcode() == Instruction::FRem) {
                            arthemeticcount++;
}
                }
            }
            return std::make_pair(arthemeticcount, memorycount);
        }

        int extract_cmp(BasicBlock* b, Function &F) {
            int x = 0;
            for (BasicBlock *Successor : successors(b))
                x++;
            if (x == 1)
                return 1;
            for (Instruction &I : *b) {
                if (CmpInst *cm = dyn_cast<CmpInst>(&I)) 
                {
                    Value  *Op1 = cm->getOperand(0);
                    Value *cnst = cm->getOperand(1);
                    int hbound = 0;
                    ConstantInt *hboundptr = dyn_cast<ConstantInt>(cnst);
                     hbound = hboundptr->getSExtValue();
                    if (Instruction *InstPtr = dyn_cast<Instruction>(Op1)) {
                        Value  *Op11 = InstPtr->getOperand(0);
                        if (Instruction *InstPtr2 = dyn_cast<Instruction>(Op11)) {
                            for (BasicBlock &BB : F) {
                                for (Instruction &I : BB) {
                                    if (isa<StoreInst>(&I)) {
                                        Value  *Op111 = I.getOperand(1);
                                        if (InstPtr2 == Op111) {
                                            Value  *ivalue = I.getOperand(0);
                                            if (ConstantInt *ivalueptr = dyn_cast<ConstantInt>(ivalue)) {
                                                return std::abs(ivalueptr->getSExtValue() - hbound);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                 else {
                    Value *Op2 = cm->getOperand(0);
                    ConstantInt *ConstOp2 = dyn_cast<ConstantInt>(Op2);
                    return std::abs(ConstOp2->getSExtValue() - hbound);
              
                   }  
                }
            }
            return 1;
        }

        int bound2(int i, Function &F, std::map<BasicBlock*, int> &lowkey) {
            int bound = 1;
            for (auto &Pair : lowkey) {
                BasicBlock *a = Pair.first;
                if (Pair.second == i) {
                    int x = lowkey[a];
                    for (BasicBlock *pred : successors(a)) {
                        if (x != lowkey[pred]) {
                            bound = extract_cmp(a, F);
                        }
                    }
                }
            }
            return bound;
        }

        void obtainList2(Function &F, std::map<BasicBlock*, int> &lowkey, int &indexcount) {
            int perfunarthemetic=0;
            int perfunmemaccess=0;
            for (int i = 0; i <= indexcount; i++) 
            {
                int x = 0;
                int y = 0;
                int bound = bound2(i, F, lowkey);
                for (auto &Pair : lowkey) {
                    if (Pair.second == i) {
                        std::pair<int,int> s = get_count(Pair.first);
                        x += s.first;
                        y += s.second;
                    }
                }
                //errs() << bound * x << "the values of the basic for loops are" << bound * y << "\n";
                perfunarthemetic =perfunarthemetic + bound * x;
                perfunmemaccess = perfunmemaccess + bound *y;
                
            }
            errs()<< "the Athremetic Intencity of the function is \t--->  "<<((float)perfunarthemetic)/((float)perfunmemaccess)<<"\n";
        }

        void dfs(BasicBlock *BB, std::map<BasicBlock*, int> &vmap, std::map<BasicBlock*, int> &index,
                std::map<BasicBlock*, int> &lowkey, int &indexcount) {
            if (vmap[BB] != 1) {
                vmap[BB] = 1;
                index[BB] = indexcount++;
                lowkey[BB] = index[BB];
            }
            for (BasicBlock *Successor : successors(BB)) {
                if (vmap[Successor] == 1) {
                    if (lowkey[BB] > lowkey[Successor]) {
                        lowkey[BB] = lowkey[Successor];
                    }
                } else {
                    dfs(Successor, vmap, index, lowkey, indexcount);
                }
            }
            for (BasicBlock *Successor : successors(BB)) {
                if (lowkey[BB] > lowkey[Successor]) {
                    lowkey[BB] = lowkey[Successor];
                }
            }
        }

        void visitor(Function &F) {
            std::map<BasicBlock*, int> vmap;
            std::map<BasicBlock*, int> index;
            std::map<BasicBlock*, int> lowkey;
            int indexcount = 0;
            errs() << "\t\t\t\t---- Next function ---------\n\n\n";
            BasicBlock *EntryBB = &F.getEntryBlock();
            dfs(EntryBB, vmap, index, lowkey, indexcount);
            obtainList2(F, lowkey, indexcount);
            int bbcount1 = 0;
            for (BasicBlock &B : F) {
                bbcount1++;
            }
        }

        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            visitor(F);
            return PreservedAnalyses::all();
        }
        static bool isRequired() { return true; }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "ArthemeticPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "arithmetic-intensity") {
                        FPM.addPass(ArithmeticIntensityPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}
