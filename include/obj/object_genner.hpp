#pragma once

#include <llvm/IR/Module.h>

namespace toyc::obj {

class ObjectGenner {
public:
    ObjectGenner() = default;

    bool generate(llvm::Module& module);
};

} // namespace toyc::obj