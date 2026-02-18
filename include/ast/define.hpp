#pragma once

#include <climits>
#include <cstdint>

namespace toyc::ast {

enum VarType {
    VAR_TYPE_VOID = 0,
    VAR_TYPE_DEFINED = 1,
    VAR_TYPE_CHAR = 2,
    VAR_TYPE_BOOL = 3,
    VAR_TYPE_SHORT = 4,
    VAR_TYPE_INT = 5,
    VAR_TYPE_LONG = 6,
    VAR_TYPE_FLOAT = 7,
    VAR_TYPE_DOUBLE = 8,
    VAR_TYPE_PTR = 9,
    VAR_TYPE_STRUCT = 10,
    VAR_TYPE_ARRAY = 11
};

// ==================== TypeIdx ====================

using TypeIdx = uint32_t;
static constexpr TypeIdx InvalidTypeIdx = UINT32_MAX;

// ==================== TypeQualifier ====================

enum TypeQualifier : uint8_t {
    QUAL_NONE = 0,
    QUAL_CONST = 1 << 0,
    QUAL_VOLATILE = 1 << 1,
};

// Bit positions used when the grammar encodes a pointer level + qualifiers into one int.
// Bits 0-15: pointer level count. Bit 16: CONST. Bit 17: VOLATILE.
static constexpr int POINTER_LEVEL_MASK = 0xFFFF;
static constexpr int POINTER_CONST_BIT = (1 << 16);
static constexpr int POINTER_VOLATILE_BIT = (1 << 17);

enum BineryOperator { AND, OR, ADD, SUB, MUL, DIV, MOD, LEFT, RIGHT, EQ, NE, LE, GE, LT, GT, BIT_AND, BIT_OR, XOR };

enum UnaryOperator { L_INC, R_INC, L_DEC, R_DEC, ADDR, DEREF, PLUS, MINUS, LOG_NOT, BIT_NOT };

}  // namespace toyc::ast
