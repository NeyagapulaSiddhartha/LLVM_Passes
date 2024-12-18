#include "llvm/IR/LegacyPassManager.h"
#include "llvm/ADT/DepthFirstIterator.h" 
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DebugInfoMetadata.h" // Add this include for DbgDeclareInst
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/GenericLoopInfo.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cmath>  
#include <cstdlib>
#include <iostream>
#include <map>
#include<queue>
#include<set>
#include <stack>
#include <string>
#include <vector>
using namespace llvm;
static cl::opt<std::string> variableName("var-name", cl::desc("variable name for"),
                                 cl::init("cool"));

namespace {
    struct FootprintPass : public PassInfoMixin<FootprintPass> {

  void ProcessLatch(BasicBlock *BB){

  }

        void ProcessHeader(BasicBlock *BB)
         {
            std::set<Instruction*>todelete;
     
            
           
            for(Instruction &I :*BB)
            { IRBuilder<> Builder(&I);
             //  errs()<<"the next instruction is  \t"<<I<<"  \n";
             
                 if(CmpInst *ci = dyn_cast<CmpInst>(&I))
                {   
                    
                       //errs()<<ci->getPredicate();
                     Value *c = dyn_cast<Value>(ci->getOperand(1));
                    // errs()<<*c;

                     Value *v = dyn_cast<Value>(ci->getOperand(0));
                    
                         //ICMP_SGT

                        Value *new_cmp = Builder.CreateCmp( CmpInst::ICMP_SGT,v,c);
                        //auto new_cmp = Builder.CreateICmpUGE(v,c);
                        //  I.replaceAllUsesWith(dyn_cast<Value>(new_cmp));
                        //todelete.insert(&I);
                        //new_cmp->setPredicate()
                        //  errs()<<"\n\n\n "<<*new_cmp<<" \n\n\n";
                        I.replaceAllUsesWith(dyn_cast<Value>(new_cmp));
                       // errs()<<"the new inst=="<<*new_cmp;
                        //  dyn_cast<Instruction>(new_cmp)->eraseFromParent();
                        //  todelete.insert(dyn_cast<Instruction>(ci));
                   
                }
            }
    //          for (auto element : todelete) {
    //     errs() <<"delete ==="<<*element;
    //     dyn_cast<Instruction>(element)->eraseFromParent();
    // }

           
          }   
 void processLoop(Loop *LI)
{




    if (LI) {
        // Now you can use LatchBlock and Header here
     LI->getHeader()->dump();
     errs()<<*LI->getLatchCmpInst();
       // errs() << *LatchBlock;
     //   ProcessHeader(Header);
      //  ProcessLatch(LatchBlock);
       // errs() << "\n\n\n out of process\n\n\n";
      
    }
    


}
void countBB(Loop *L)
{
   
    if(L)
    {
 //L->getHeader()->dump();
        processLoop(L);

        for(auto *NL :*L)
        // NL->dump();
        countBB(NL);
    }
}
 void loopBBCnt2(Function &F,FunctionAnalysisManager &FAM)
 {
     ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
    LoopInfo &LI=FAM.getResult<LoopAnalysis>(F);

//const auto & loop = LI.getTopLevelLoops();

    for(Loop *L : LI)
    {
        L->dump();
        static int i =0;
        errs()<<"enterring the loop i ="<<i<<"  \n";
        i++;
            if (PHINode *IndVar = L->getCanonicalInductionVariable())
            
            {
                //  errs()<<L->getBounds(SE)->getCanonicalPredicate();
                //errs()<<*(X->getStepValue())<<"-------------------------\n";
            // errs()<<*IndVar<<"-------------------------\n";
            }
                 // errs()<<x->getFinalIVValue()<<"-------------------------\n";
                 //errs()<<*(x.getStepValue())<<"-------------------------\n";
    
                // errs()<<"this the loop \n";
                //  L->dump();
                //  errs()<<"--------------\n";
                // ICmpInst * cmp = L->getLatchCmpInst();
                // cmp->dump();
                // if(cmp)
                // errs()<<*(cmp->getOperand(0))<<"this the loop------------- \n";
                // //auto OptBounds= L->LoopBounds(&SE);
   
         countBB(L);
    }
    return ;
 }
        
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
             LoopInfo &LI=FAM.getResult<LoopAnalysis>(F);
             loopBBCnt2(F ,FAM);
           
             ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
          //  auto OptBounds= LI.getBounds(SE);



    for (Loop *TopLevelLoop : LI) {
    
    BasicBlock *LatchBlock = nullptr;
    BasicBlock *Header = nullptr;
    if ((LatchBlock = TopLevelLoop->getLoopLatch()) &&
        (Header = TopLevelLoop->getHeader())) {
        // Now you can use LatchBlock and Header here
       // errs() << *Header << "\n";
       // errs() << *LatchBlock;
        //countBB(L);
       // ProcessHeader(Header);
       // errs() << "\n\n\n out of process\n\n\n";
      
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
                    if (Name == "loop-reversal") {
                        FPM.addPass(FootprintPass());
                        return true;
                    }
                    return false;
                });
        }


    };
}
