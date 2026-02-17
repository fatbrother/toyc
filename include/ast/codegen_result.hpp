#pragma once

#include <llvm/IR/Value.h>

#include <string>

#include "ast/define.hpp"

namespace toyc::ast {

template <typename T>
class CodegenResult {
public:
    CodegenResult() : errorMessage("") {}
    CodegenResult(const T& data) : data(data), errorMessage("") {}
    CodegenResult(const std::string& errMsg) : errorMessage(errMsg) {}
    CodegenResult(const char* errMsg) : errorMessage(std::string(errMsg)) {}

    template <typename... Args>
    CodegenResult(Args... args) : data(args...), errorMessage("") {}

    template <typename U>
    CodegenResult& operator<<(const CodegenResult<U>& other) {
        if (!other.isSuccess()) {
            if (errorMessage.empty()) {
                errorMessage = other.getErrorMessage();
            } else {
                errorMessage = errorMessage + "\n" + other.getErrorMessage();
            }
        }
        return *this;
    }

    bool isSuccess() const {
        if constexpr (std::is_same_v<T, void>) {
            return errorMessage.empty();
        } else {
            return isSuccessImpl(0);
        }
    }

private:
    template <typename U = T>
    auto isSuccessImpl(int) const -> decltype(std::declval<U>().isValid(), bool()) {
        return errorMessage.empty() && data.isValid();
    }

    template <typename U = T>
    bool isSuccessImpl(long) const {
        return errorMessage.empty();
    }

public:
    std::string getErrorMessage() const { return errorMessage; }

    T getData() const { return data; }

    template <typename U = T>
    auto getValue() const -> decltype(std::declval<U>().value) {
        return data.value;
    }

    template <typename U = T>
    auto getType() const -> decltype(std::declval<U>().type) {
        return data.type;
    }

    template <typename U = T>
    auto getAllocaInst() const -> decltype(std::declval<U>().allocInst) {
        return data.allocInst;
    }

private:
    T data;
    std::string errorMessage;
};

template <>
class CodegenResult<void> {
public:
    CodegenResult() : errorMessage("") {}
    CodegenResult(const std::string& errMsg) : errorMessage(errMsg) {}

    template <typename U>
    CodegenResult& operator<<(const CodegenResult<U>& other) {
        if (!other.isSuccess()) {
            if (errorMessage.empty()) {
                errorMessage = other.getErrorMessage();
            } else {
                errorMessage = errorMessage + "\n" + other.getErrorMessage();
            }
        }
        return *this;
    }

    bool isSuccess() const { return errorMessage.empty(); }
    std::string getErrorMessage() const { return errorMessage; }

private:
    std::string errorMessage;
};

/**
 * Value type for Expression code generation results.
 */
struct ExprValue {
    llvm::Value* value = nullptr;
    TypeIdx type = InvalidTypeIdx;

    ExprValue() = default;
    ExprValue(llvm::Value* v, TypeIdx t) : value(v), type(t) {}
    bool isValid() const { return value != nullptr && type != InvalidTypeIdx; }
};

struct AllocValue {
    llvm::Value* allocInst = nullptr;
    TypeIdx type = InvalidTypeIdx;

    AllocValue() = default;
    AllocValue(llvm::Value* a, TypeIdx t) : allocInst(a), type(t) {}
    bool isValid() const { return allocInst != nullptr && type != InvalidTypeIdx; }
};

// Type aliases for common use cases
using ExprCodegenResult = CodegenResult<ExprValue>;
using StmtCodegenResult = CodegenResult<void>;
using AllocCodegenResult = CodegenResult<AllocValue>;

}  // namespace toyc::ast
