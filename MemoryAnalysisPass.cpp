#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <set>
#include <string>
#include <algorithm>
using namespace llvm;

namespace {
    struct MemoryAnalyzer : public FunctionPass {
        static char ID;
        std::map<std::string, int> blockMemoryMap;
        int id=1;
        MemoryAnalyzer() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            for (BasicBlock &BB : F) {
                int memoryRequirement = analyzeBasicBlockMemory(&BB);
                blockMemoryMap[BB.getName().str()] = memoryRequirement;
            }
            printMemoryRequirements();
            return false;
        }

        int analyzeBasicBlockMemory(BasicBlock *BB) {
            int memoryRequirement = 0;
            std::set<Value *> allocatedPointers;
            DataLayout DL = BB->getParent()->getParent()->getDataLayout();
            for (Instruction &I : *BB) {
                if (isa<AllocaInst>(&I)) {
                    memoryRequirement += getTypeSize(I.getType(), DL);
                } else if (isa<LoadInst>(&I)) {
                    Value *pointerOperand = I.getOperand(0);
                    if (allocatedPointers.count(pointerOperand)) {
                        memoryRequirement += getTypeSize(pointerOperand->getType(), DL);
                    }
                } else if (isa<StoreInst>(&I)) {
                    Value *pointerOperand = I.getOperand(1);
                    if (allocatedPointers.count(pointerOperand)) {
                        memoryRequirement += getTypeSize(pointerOperand->getType(), DL);
                    }
                } else if (isa<CallInst>(&I)) {
                    Function *calledFunction = cast<CallInst>(&I)->getCalledFunction();
                    if (calledFunction && calledFunction->getName().startswith("malloc")) {
                        Type *allocatedType = calledFunction->getReturnType()->getPointerElementType();
                        memoryRequirement += getTypeSize(allocatedType, DL);
                        allocatedPointers.insert(&I);
                    }
                }
            }
            return memoryRequirement;
        }

        int getTypeSize(Type *Ty, const DataLayout &DL) {
            if (Ty->isSized()) {
                return DL.getTypeAllocSize(Ty);
            }
            return 0;
        }

        void printMemoryRequirements() {
            for (auto &entry : blockMemoryMap) {
                errs() << "Basic Block ID: " << id << ", Memory Requirement: " << entry.second << " bytes\n";
                id++;
            }
        }
    };
}

char MemoryAnalyzer::ID = 0;
static RegisterPass<MemoryAnalyzer> X("memory-analyzer", "Memory Analyzer Pass", false, false);
