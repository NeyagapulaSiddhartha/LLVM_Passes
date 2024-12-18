#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/DebugInfoMetadata.h" // Add this include for DbgDeclareInst
#include "llvm/ADT/DepthFirstIterator.h" 
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include <map>
#include <stack>
#include <vector>
#include <string>
#include <cstdlib>
#include <cmath>  
#include <algorithm>
#include <iostream>
#include<set>
#include<queue>
#include "llvm/Support/CommandLine.h"
using namespace llvm;
static cl::opt<std::string> variableName("var-name", cl::desc("variable name for"),
                                 cl::init("cool"));

namespace {
    struct FootprintPass : public PassInfoMixin<FootprintPass> {
        //std::string VarName;
        // FootprintPass() : PassInfoMixin(), VarName(VarNameOption) {}
  std:: set<   std::pair<Instruction*,Instruction *>   > gmap;
  std::vector<Instruction * > all_variables;
  std:: map<BasicBlock* , std:: set<std::pair<Instruction* ,Instruction*>>>B_map;
  std:: map<BasicBlock* , std:: set<std::pair<Instruction* ,Instruction*>>>Prev_B_map;

  BasicBlock * getExitBlock(Function &F)
{
  for(BasicBlock &BB :F)
  {
    for (Instruction &I:BB )
    {
      if (ReturnInst *RI = dyn_cast<ReturnInst>(&I))
      {
        return &BB;

      }
    }
  }
}
        bool is_variable(Instruction * I)
{
  if(dyn_cast<AllocaInst>(I)->getAllocatedType()->getTypeID() ==15||dyn_cast<AllocaInst>(I)->getAllocatedType()->getTypeID() ==17)
  {
    return 1;
  }
  else 
  {
    return 0;
  }

}
std::set<std::pair<Instruction*, Instruction*>> Init(Function& F) {
  std::set<std::pair<Instruction*, Instruction*>> gmap;
  all_variables.push_back(nullptr);
  for (BasicBlock& BB : F) {
    for (Instruction& I : BB) {
      if (AllocaInst* AI = dyn_cast<AllocaInst>(&I)) {
        if (AI->getAllocatedType()->getTypeID() == 15 || AI->getAllocatedType()->getTypeID() == 17) {
          gmap.insert(std::make_pair(&I, nullptr));
          all_variables.push_back(&I);
        } else if (AI->getAllocatedType()->getTypeID() != 15) {
          all_variables.push_back(&I);
        }
      }
    }
  }
  return gmap;
}
std::vector<Instruction*> getALLelements(std::set<std::pair<Instruction*, Instruction*>> gmap) {
  return all_variables;
}

std::set<std::pair<Instruction*, Instruction*>> cross(std::vector<Instruction*> v1, std::vector<Instruction*> v2) {
  std::set<std::pair<Instruction*, Instruction*>> out;
  for (auto& key : v1) {
    for (auto& value : v2) {
      out.insert(std::make_pair(key, value));
    }
  }
  return out;
}
std::set<std::pair<Instruction*, Instruction*>> mustkill(BasicBlock* BB, std::vector<Instruction*> gen, std::set<std::pair<Instruction*, Instruction*>> gmap) {
  std::set<std::pair<Instruction*, Instruction*>> killinfo;
  if (gen.size() == 1) {
    std::vector<Instruction*> v = getALLelements(gmap);
    killinfo = cross(gen, v);
  } else {
    killinfo.insert(std::make_pair(nullptr, nullptr));
  }
  return killinfo;
}

std::vector<Instruction*> Gen(BasicBlock* BB, Instruction* ROperand, std::set<std::pair<Instruction*, Instruction*>> gmap) {
  int x = 0;
  Instruction* init_operand = ROperand;
  while (!dyn_cast<AllocaInst>(ROperand)) {
    ROperand = dyn_cast<Instruction>(ROperand->getOperand(0));
    x++;
  }
  if (init_operand != ROperand && dyn_cast<GetElementPtrInst>(init_operand)) {
    x = 0;
  }
  std::vector<Instruction*> v1, v2;
  v1.push_back(ROperand);
  while (x) {
    std::vector<Instruction*> v2;
    --x;
    while (!v1.empty()) {
      Instruction* temp2 = v1.back();
      v1.pop_back();
      for (const auto& i : gmap) {
        if (i.first == temp2 && i.first != i.second) {
          v2.push_back(i.second);
        }
      }
    }
    v1 = v2;
  }
  return v1;
}

std::vector<Instruction*> Pointee(BasicBlock* BB, Instruction* ROperand, std::set<std::pair<Instruction*, Instruction*>> gmap) {
  int x = 0;
  Instruction* init_operand = ROperand;
  while (!dyn_cast<AllocaInst>(ROperand)) {
    ROperand = dyn_cast<Instruction>(ROperand->getOperand(0));
    x++;
  }
  if (init_operand != ROperand && dyn_cast<GetElementPtrInst>(init_operand)) {
    x = 0;
  }
  std::vector<Instruction*> v1;
  v1.push_back(ROperand);
  while (x) {
    std::vector<Instruction*> v2;
    --x;
    while (!v1.empty()) {
      Instruction* temp2 = v1.back();
      v1.pop_back();
      for (const auto& i : gmap) {
        if (i.first == temp2) {
          v2.push_back(i.second);
        }
      }
    }
    v1 = v2;
  }
  return v1;
}

std::set<std::pair<Instruction*, Instruction*>> minus(std::set<std::pair<Instruction*, Instruction*>> gen, std::set<std::pair<Instruction*, Instruction*>> kill) {
  std::set<std::pair<Instruction*, Instruction*>> s1;
  std::set_difference(gen.begin(), gen.end(), kill.begin(), kill.end(), std::inserter(s1, s1.begin()));
  return s1;
}

std::set<std::pair<Instruction*, Instruction*>> uni(std::set<std::pair<Instruction*, Instruction*>> a, std::set<std::pair<Instruction*, Instruction*>> b) {
  std::set<std::pair<Instruction*, Instruction*>> s1;
  std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::inserter(s1, s1.begin()));
  return s1;
}
bool processblock(BasicBlock* BB) {
  int i = 0;
  int change = 0;
  std::set<std::pair<Instruction*, Instruction*>> iin;
  std::set<std::pair<Instruction*, Instruction*>> gmap;
  for (BasicBlock* pred : predecessors(BB)) {
    iin = uni(iin, B_map[pred]);
  }
  gmap = iin;

  for (Instruction& I : *BB) {
    StoreInst* si = dyn_cast<StoreInst>(&I);
    if (si) {
      Instruction* OP1 = dyn_cast<Instruction>(si->getOperand(0));
      Instruction* OP2 = dyn_cast<Instruction>(si->getOperand(1));
      if (si && (si->getOperand(0)->getType()->getTypeID() == 15 || si->getOperand(0)->getType()->getTypeID() == 17) && (si->getOperand(1)->getType()->getTypeID() == 15 || si->getOperand(1)->getType()->getTypeID() == 17)) {
        i++;
        std::vector<Instruction*> gen = Gen(BB, OP2, gmap);
        std::vector<Instruction*> pointee = Pointee(BB, OP1, gmap);
        std::set<std::pair<Instruction*, Instruction*>> kill = mustkill(BB, gen, gmap);
        gmap = minus(gmap, kill);
        gmap = uni(gmap, cross(gen, pointee));
      }
    }
  }

  if (gmap != B_map[BB]) {
    change = 1;
  }
  B_map[BB] = gmap;

  if (i == 0 && Prev_B_map[BB] != B_map[BB]) {
    Prev_B_map[BB] = B_map[BB];
    return true;
  }
  return change;
}

std::set<std::pair<Instruction*, Instruction*>> kildal(Function& F) {
  BasicBlock* entry = &F.getEntryBlock();
  B_map[entry] = Init(F);
  std::queue<BasicBlock*> worklist;
  std::set<std::pair<Instruction*, Instruction*>> final_result;
  worklist.push(entry);

  while (!worklist.empty()) {
    BasicBlock* curr_node = worklist.front();
    int change = processblock(curr_node);
    worklist.pop();
    final_result = B_map[curr_node];
    if (change) {
      for (BasicBlock* Succ : successors(curr_node)) {
        B_map[Succ] = B_map[curr_node];
        worklist.push(Succ);
      }
    }
  }
  return final_result;
}


        bool hasOOperand(Instruction *Inst, Instruction  *I) {
            int flag =0;
            for (Use &u : Inst->operands()) {
                Value *Operand = u.get();
                //Instruction *InstPtr2 = dyn_cast<Instruction>(Operand);
                if (Operand == I) { // Fix: Use comparison operator '=='
                    flag=1;
                }
            }
            return flag;
        }

        int getlines(Instruction *Inst) {
            if (
                MDNode *MD = Inst->getMetadata(LLVMContext::MD_dbg)) {
                if (auto *DILoc = dyn_cast<DILocation>(MD)) {
                  //  errs()<<DILoc->getLine()<<"\n";
                    return DILoc->getLine();
                }
            }
            return -1;
        }

        std::set<int> Footprint(Instruction *I, Function *F,std::set<int> footprintset) {
            int max_scope = -1; // Initialize max_scope
            for (auto &BB : *F) {
                for (auto &Inst : BB) {
                    if (hasOOperand(&Inst, I)) { 
                        //max_scope = std::max(max_scope, getlines(&Inst));
                        //errs()<<"the instruction "<<Inst<<" ";
                        footprintset.insert(getlines(&Inst));
                        //errs()<<" "<<getlines(&Inst)<<"\t";
                    }
                }
            }
            return footprintset;
            
            errs()<<"\n";
            
        }
        // void GettingUsers(Instruction *I , Function *F)
        // {
        //     for ( llvm::Value::user_iterator  iter = I->user_begin(); iter!=I->user_end(); ++iter)
        //     {
        //         errs()<<getlines(iter)<<" \t";
        //     }
        //     errs()<<" \n";
        // }


        DILocalScope * getscope(Instruction *Inst)
        {
            if (MDNode *MD = Inst->getMetadata(LLVMContext::MD_dbg)) {
                if (auto *DILoc = dyn_cast<DILocation>(MD)) {

                   
                    return DILoc->getScope();
                }
            }
        
        }
     void  getMaxScope(DbgDeclareInst *KI, Function *F)
        {
           // DILocalScope* ScopeInfo = KI->getVariable()->getScope();
            //errs()<<*ScopeInfo<<"\n";
           Instruction *AI = dyn_cast<Instruction>(KI->getAddress());
           //errs()<<"should get aloca "<<*AI<<" \n";
            int max_scope =-1;
            for (auto &BB : *F) {
                for (auto &Inst : BB) {

              if(hasOOperand(&Inst,AI))

                   { 
                    max_scope=std::max(max_scope, getMaxLine(getscope(&Inst),F));
                    }
                }
            }
            errs()<< "  "<< max_scope<<"\n";

        }
int   getMaxLine(DILocalScope* ScopeInfo, Function *F)
        {
            
            //errs()<<*ScopeInfo<<"\n";
        int max_Line=-1;
            for (auto &BB : *F) {
                for (auto &Inst : BB) {
                    if(ScopeInfo==getscope(&Inst))
                    {
                        // errs()<< " -----------"<<*getscope(&Inst)<<"\n";
                       // errs()<<"the instruction "<<Inst<<"\n";
                        //errs()<<"got the scope "<<"\n";
                        max_Line = std:: max(max_Line, getlines(&Inst));
                    }
                }
            }
           return max_Line;

        }

int getLoopReads(Instruction *inst, Function *F,FunctionAnalysisManager * FAM)
{
    ScalarEvolution &SE = FAM->getResult<ScalarEvolutionAnalysis>(*F);
      // errs()<<"enterring into the loop reads\n";
    LoopInfo &LI=FAM->getResult<LoopAnalysis>(*F);
    int loopNo=0;
    for (Loop *TopLevelLoop : LI) {
        //errs()<<"getting trip count\n";
        //errs()<<*SE.getBackedgeTakenCount(TopLevelLoop);
    if(auto OptBounds = TopLevelLoop->getBounds(SE))
       {
   //errs() << "\nNo. of basic blocks: " << noBB << "\n";
       // errs()<<"print the bound\n";
       Value &v = OptBounds->getFinalIVValue();
       errs()<<v;

       }
        for (Loop *L : depth_first(TopLevelLoop)) {
        ++loopNo;
        unsigned noBB = 0;
        
       
       if(auto OptBounds = L->getBounds(SE))
       {
          // errs() << "\nNo. of basic blocks: " << noBB << "\n";
        //errs()<<"print the bound\n";
       Value &v = OptBounds->getFinalIVValue();
       errs()<<v;

       }
        //errs()<<B1<<"\n";
        // errs() << "\nLoop number: " << loopNo;
        for (auto *bb : L->getBlocks()) {
        //DEBUG_WITH_TYPE(“debug1”,bb->printAsOperand(err() << ", "));
        noBB++;
        }
     
        
        return loopNo;
}
}

}

int getTotalReads(Instruction *I ,Function * F,FunctionAnalysisManager *FAM)
{
    return 6;
}
 std::set<int> forAlias(Instruction * iInst,Function *F,std::set<int>footprintset,std::set<std::pair<Instruction*,Instruction*>>final_list)
 {
      std::set<Instruction *>temp;
                                for (const auto& pair : final_list) {
                                if(pair.second==iInst)
                                {
                                  temp.insert(pair.first);
                                   footprintset = Footprint(pair.first,F,footprintset);

                                }
                            }
                      for (auto it = temp.begin(); it != temp.end(); ++it) {
                                      footprintset   =   forAlias(*it,F,footprintset,final_list);
                             }

return footprintset;
                          


 }

        void getReads(Instruction * Inst , Function * F,FunctionAnalysisManager *FAM,ScalarEvolution &SE)
        {
            int loop_reads = getLoopReads(Inst,&(*F),FAM);
            int Total_reads = getTotalReads(Inst,&(*F),FAM);
            errs()<< Total_reads - loop_reads -1;
        }

        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
std::set<std::pair<Instruction*, Instruction*>> final_list =kildal(F);
BasicBlock* FB = getExitBlock(F);
if(FB)
{
  final_list=B_map[FB];
}

            AliasAnalysis &AA = FAM.getResult<AAManager>(F);
            ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
            for (auto &BB : F) {
                for (auto &Inst : BB) {
                    if (DbgDeclareInst *KI = dyn_cast<DbgDeclareInst>(&Inst)) {
                        if (KI->getVariable()->getName() == variableName) {
                            errs() << KI->getVariable()->getName() << "\n";
                            int min_scope = KI->getVariable()->getLine() ;

                           if(Instruction *iInst = dyn_cast<Instruction>(KI->getAddress()))
                           {
                            
                            std::set<int> footprintset;
                            footprintset= Footprint(iInst,&F,footprintset);
                            
                            //errs()<<"got instruction\t"<<*iInst<<"\n";
                            errs()<<"the foot print is \n";
                            // for (const auto& pair : final_list) {

                                


                            //     if(pair.second==iInst)
                            //     {
                            //        footprintset = Footprint(pair.first,&F,footprintset);

                            //     }
                            // }
                            footprintset =forAlias(iInst,&F,footprintset,final_list);
                        for (auto it = footprintset.begin(); it != footprintset.end(); ++it) {
                                            errs() << *it << " ";
                             }
                            errs()<<"\n";
                             //getReads(iInst,&F,&FAM ,SE);
                           }

                           errs()<<"the scope is " <<min_scope<<"\t";
                           getMaxScope(KI, &F);
                           
                        }
                         
                    }
                    // You can include other conditions here if needed
                }
            }
            return PreservedAnalyses::all();
        }



        static bool isRequired() { return true; }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "Footprint", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "ftprint") {
                        FPM.addPass(FootprintPass());
                        return true;
                    }
                    return false;
                });
        }


    };
}
