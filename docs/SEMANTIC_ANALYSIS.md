# Semantic Analysis Documentation

## Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Component Descriptions](#component-descriptions)
4. [Complete Flow](#complete-flow)
5. [Visitor Handlers](#visitor-handlers)
6. [Error Handling](#error-handling)
7. [Examples](#examples)

---

## Overview

Semantic Analysis is the third phase of compilation, occurring after:
1. **Lexical Analysis** (Tokenizer) → Tokens
2. **Syntax Analysis** (Parser) → AST

Semantic Analysis validates the **meaning** of the code:
- Are all variables declared before use?
- Are all functions defined before call?
- Do types match in assignments and expressions?
- Do all code paths return a value?
- Are control statements (break/continue) inside loops?

### Input/Output

```
INPUT:  AST (Abstract Syntax Tree)
OUTPUT: DecoratedAST (AST + Symbol Tables)
        - FuncTable: All functions with signatures
        - VarTableNode: All variables with types/scopes
```

---

## Architecture

### Component Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SEMANTIC ANALYSIS PIPELINE                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                  SemanticAnalysisLogicComponent                     │    │
│  │                         (ORCHESTRATOR)                              │    │
│  │  ┌─────────────┐  ┌──────────────┐  ┌────────────────────────────┐  │    │
│  │  │ ASTIterator │  │ DecoratedAST │  │ SemanticAnalysisVisitor    │  │    │
│  │  │ (traversal) │  │ (output)     │  │ (processing)               │  │    │
│  │  └─────────────┘  └──────────────┘  └────────────────────────────┘  │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                    │                                        │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    SemanticAnalysisVisitor                          │    │
│  │                       (VISITOR PATTERN)                             │    │
│  │                                                                     │    │
│  │   Uses:                         Produces:                           │    │
│  │   ┌─────────────┐              ┌───────────────┐                    │    │
│  │   │ASTExtractor │              │ VarMetaData   │                    │    │
│  │   │(extract)    │              │ FuncMetaData  │                    │    │
│  │   └─────────────┘              └───────────────┘                    │    │
│  │   ┌─────────────┐              ┌───────────────┐                    │    │
│  │   │VarFactory   │              │ Errors via    │                    │    │
│  │   │FuncFactory  │              │ ErrorManager  │                    │    │
│  │   │(create)     │              └───────────────┘                    │    │
│  │   └─────────────┘                                                   │    │
│  │   ┌──────────────────┐                                              │    │
│  │   │SemanticChecker   │                                              │    │
│  │   │Facade (validate) │                                              │    │
│  │   └──────────────────┘                                              │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Class Relationships

```
SemanticAnalysisLogicComponent
    │
    ├── owns: EntityLogger
    ├── owns: ASTIterator
    ├── owns: DecoratedAST
    │           ├── ref: AST (original)
    │           ├── owns: FuncTable
    │           └── owns: VarTableNode (root)
    │
    └── owns: SemanticAnalysisVisitor
                ├── ref: EntityLogger
                ├── ref: ErrorManager
                ├── ref: VarTableNode (root)
                ├── ref: FuncTable
                ├── owns: VarFactory
                ├── owns: FuncFactory
                └── ptr: m_currentScope (tracks current block)
```

---

## Component Descriptions

### 1. SemanticAnalysisLogicComponent (Orchestrator)
**File:** `semantic_analysis_logic.h/cpp`

The main entry point. Coordinates the semantic analysis pipeline.

```cpp
class SemanticAnalysisLogicComponent
{
    EntityLogger m_semanticAnalysisLogger;
    ASTIterator m_astIterator;
    DecoratedAST m_decoratedAST;
    SemanticAnalysisVisitor m_semanticAnalysisVisitor;
    
    const DecoratedAST& MainFlow();  // Entry point
};
```

### 2. SemanticAnalysisVisitor (Processor)
**File:** `semantic_analysis_visitor.h/cpp`

Implements the Visitor pattern. Contains handlers for each AST node type.

```cpp
class SemanticAnalysisVisitor : public IASTVisitor
{
    // Symbol table references
    VarTableNode& m_varTable;
    FuncTable& m_funcTable;
    
    // Factories
    VarFactory m_varFactory;
    FuncFactory m_funcFactory;
    
    // Context tracking
    VarTableNode* m_currentScope;
    ExpressionType m_lastExpressionType;
    
    // Handlers
    void VisitVarDeclaration(ASTNode* node);
    void VisitExpression(ASTNode* node);
    // ... more handlers
};
```

### 3. ASTExtractor (Data Extraction)
**File:** `ast_extractor.h/cpp`

Pure utility class that extracts raw values from AST nodes.

```cpp
class ASTExtractor
{
    static const std::string& ExtractVarName(ASTNode* node);
    static Type ExtractVarType(ASTNode* node);
    static Scope ExtractVarScope(ASTNode* node);
    static AccessRights ExtractVarAccessRight(ASTNode* node);
};
```

### 4. VarFactory / FuncFactory (Object Creation)
**Files:** `var_factory.h/cpp`, `func_factory.h/cpp`

Create metadata objects from extracted values.

```cpp
class VarFactory
{
    VarMetaData CreateVarMetadata(
        std::string name,
        Type type,
        AccessRights access,
        Scope scope
    );
};
```

### 5. SemanticCheckerFacade (Validation)
**File:** `semantic_checker_facade.h/cpp`

Static class containing all validation logic.

```cpp
class SemanticCheckerFacade
{
    // Existence checks
    static bool CheckVarExistence(const VarTableNode&, const std::string&);
    static bool CheckFuncDeclaration(const FuncTable&, const std::string&);
    
    // Type checks
    static ExpressionType CheckExpression(const Command&, const FuncTable&, const VarTableNode&);
    static bool CheckAssignment(const std::string&, ExpressionType, const VarTableNode&);
    static bool CheckFunctionReturnType(ExpressionType, const std::string&, const FuncTable&);
    
    // Control flow checks
    static bool CheckControlStatement();
    static bool EnteredBlock(BlockKind, bool isVoid);
    static bool ExitedBlock();
};
```

### 6. DecoratedAST (Output)
**File:** `semantic_analysis_logic.h`

The final output containing the original AST plus symbol tables.

```cpp
struct DecoratedAST
{
    const AST& ast;           // Original AST
    FuncTable funcTable;      // All functions
    VarTableNode varTable;    // All variables (tree structure)
};
```

---

## Complete Flow

### High-Level Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Parser    │ ──► │  MainFlow   │ ──► │   Visitor   │ ──► │ Decorated   │
│   (AST)     │     │  (iterate)  │     │  (process)  │     │    AST      │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

### Detailed Flow: MainFlow()

```cpp
const DecoratedAST& SemanticAnalysisLogicComponent::MainFlow()
{
    // Iterate through every node in AST (DFS order)
    while (m_astIterator.HasNext())
    {
        ASTNode* current = m_astIterator.GetCurrent();
        
        // Dispatch to appropriate handler based on node type
        m_semanticAnalysisVisitor.Visit(current);
        
        m_astIterator.Next();
    }
    
    return m_decoratedAST;  // Contains populated symbol tables
}
```

### Visitor Dispatch Flow

```
Visit(node)
    │
    ▼
node->command.GetType()
    │
    ├── VAR_DECLARATION ──► VisitVarDeclaration()
    ├── EXPRESSION ────────► VisitExpression()
    ├── ASSIGNMENT ────────► VisitAssignment()
    ├── FINISHED_FUNC_DECL ► VisitFuncDeclaration()
    ├── FINISHED_FUNC_CALL ► VisitFuncCall()
    ├── IF ────────────────► VisitIf()
    ├── WHILE ─────────────► VisitWhile()
    ├── FOR ───────────────► VisitFor()
    ├── RETURN ────────────► VisitReturn()
    ├── BLOCK_START ───────► VisitBlockStart()
    ├── BLOCK_END ─────────► VisitBlockEnd()
    └── ... etc
```

---

## Visitor Handlers

### VisitVarDeclaration

**Purpose:** Process variable declarations, check for redeclaration, add to symbol table.

```
Flow:
┌─────────────────────────────────────────────────────────────────┐
│ 1. EXTRACT: ASTExtractor::ExtractVarName/Type/Scope/Access      │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. VALIDATE: SemanticCheckerFacade::CheckVarExistence           │
│              Does variable already exist in current scope?      │
└────────────────────────────┬────────────────────────────────────┘
                             │
              ┌──────────────┴──────────────┐
              │                             │
              ▼ EXISTS                      ▼ NOT EXISTS
┌─────────────────────────┐   ┌─────────────────────────────────┐
│ ErrorManager::StoreError│   │ 3. CREATE: VarFactory::Create   │
│ NAME_ERROR              │   │    VarMetadata                  │
│ return                  │   └───────────────┬─────────────────┘
└─────────────────────────┘                   │
                                              ▼
                            ┌─────────────────────────────────────┐
                            │ 4. STORE: VarTableNode::InsertVar   │
                            └─────────────────────────────────────┘
```

### VisitExpression

**Purpose:** Validate expression, store type for parent handlers.

```
Flow:
┌─────────────────────────────────────────────────────────────────┐
│ 1. VALIDATE: SemanticCheckerFacade::CheckExpression             │
│              - Check all identifiers exist                      │
│              - Check all function calls exist                   │
│              - Check type consistency (no int + bool)           │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
              ┌──────────────┴──────────────┐
              │                             │
              ▼ UNKNOWN                     ▼ VALID TYPE
┌─────────────────────────┐   ┌─────────────────────────────────┐
│ ErrorManager::StoreError│   │ 2. STORE: m_lastExpressionType  │
│ TYPE_ERROR              │   │    (for parent handler to use)  │
└─────────────────────────┘   └─────────────────────────────────┘
```

### VisitBlockStart / VisitBlockEnd

**Purpose:** Manage scope when entering/exiting blocks.

```
VisitBlockStart:
┌─────────────────────────────────────────────────────────────────┐
│ m_currentScope = m_currentScope->CreateBlock()                  │
│ (Create new child scope, descend into it)                       │
└─────────────────────────────────────────────────────────────────┘

VisitBlockEnd:
┌─────────────────────────────────────────────────────────────────┐
│ m_currentScope = m_currentScope->GetParent()                    │
│ (Ascend back to parent scope)                                   │
│                                                                 │
│ SemanticCheckerFacade::ExitedBlock()                            │
│ (Check return path coverage for functions)                      │
└─────────────────────────────────────────────────────────────────┘
```

### VisitFuncDeclaration

**Purpose:** Register function in FuncTable, enter function context.

```
Flow:
┌─────────────────────────────────────────────────────────────────┐
│ 1. EXTRACT: Function name, return type, parameters              │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. VALIDATE: Check not redeclared/redefined                     │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. CREATE: FuncFactory::CreateFuncMetadata                      │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 4. STORE: FuncTable::InsertFunction                             │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 5. CONTEXT: SemanticCheckerFacade::EnteredBlock(Function, ...)  │
│             m_currentFunctionName = funcName                    │
└─────────────────────────────────────────────────────────────────┘
```

### VisitReturn

**Purpose:** Validate return statement type matches function signature.

```
Flow:
┌─────────────────────────────────────────────────────────────────┐
│ 1. CHECK: SemanticCheckerFacade::CheckReturnInsideFunc()        │
│           (Must be inside a function)                           │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. VISIT: VisitExpression(returnExprNode)                       │
│           (Get expression type in m_lastExpressionType)         │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. VALIDATE: SemanticCheckerFacade::CheckFunctionReturnType     │
│              (m_lastExpressionType matches function return type)│
└─────────────────────────────────────────────────────────────────┘
```

---

## Error Handling

### Error Types

| Error Type | When Triggered |
|------------|----------------|
| `NAME_ERROR` | Variable/function not found, redeclaration |
| `TYPE_ERROR` | Type mismatch in expression, assignment, return |
| `SYNTAX_ERROR` | Return outside function, break outside loop |
| `INTERNAL_ERROR` | Unexpected failures (should not happen) |

### Error Flow

```
Handler detects error
        │
        ▼
ErrorManager::StoreError(CompileTimeError(...))
        │
        ▼
Continue processing (find more errors)
        │
        ▼
After MainFlow completes, check ErrorManager
        │
        ▼
If errors exist → Report all and abort compilation
```

---

## Examples

### Example 1: Variable Declaration

**Source Code:**
```cpp
let x: int = 5;
```

**AST:**
```
VAR_DECLARATION (x)
ASSIGNMENT (=)
EXPRESSION (5)
END_OF_COMMAND (;)
```

**Semantic Analysis:**
```
1. VisitVarDeclaration(VAR_DECLARATION)
   - Extract: name="x", type=INT, scope=LET, access=DEFAULT
   - Validate: CheckVarExistence → false (not exists)
   - Create: VarMetaData{x, INT, DEFAULT, LET}
   - Store: VarTableNode.InsertVar(metadata)

2. VisitAssignment(ASSIGNMENT)
   - Get variable "x" from context
   - Get m_lastExpressionType from VisitExpression

3. VisitExpression(EXPRESSION)
   - CheckExpression → MATHEMATICAL
   - Store: m_lastExpressionType = MATHEMATICAL

4. Validate assignment: int = MATHEMATICAL → OK
```

### Example 2: Undefined Variable Error

**Source Code:**
```cpp
let y = x + 1;  // x not declared
```

**Semantic Analysis:**
```
1. VisitVarDeclaration(VAR_DECLARATION)
   - Extract: name="y", type=INT
   - Validate: OK (y not exists)
   - Create & Store: OK

2. VisitExpression(EXPRESSION: x + 1)
   - CheckExpression:
     - Check "x" in VarTableNode → NOT FOUND
     - Return: UNKNOWN
   - StoreError: TYPE_ERROR "undefined identifier"
```

### Example 3: Function with Return

**Source Code:**
```cpp
func add(a: int, b: int) -> int {
    return a + b;
}
```

**Semantic Analysis:**
```
1. VisitFuncDeclaration(FINISHED_FUNC_DECLARATION)
   - Extract: name="add", return=INT, params=[a:INT, b:INT]
   - Create & Store in FuncTable
   - EnteredBlock(Function, isVoid=false)

2. VisitBlockStart(BLOCK_START)
   - m_currentScope = CreateBlock()
   - Add parameters a, b to scope

3. VisitReturn(RETURN)
   - CheckReturnInsideFunc → true
   - VisitExpression(a + b) → MATHEMATICAL
   - CheckFunctionReturnType(MATHEMATICAL, "add") → true (INT matches)

4. VisitBlockEnd(BLOCK_END)
   - ExitedBlock() → Check all paths return → OK
   - m_currentScope = GetParent()
```

---

## Summary

### Pipeline Steps

1. **Orchestrator** calls `MainFlow()`
2. **Iterator** traverses AST in DFS order
3. **Visitor** dispatches to appropriate handler per node type
4. **Handler** follows Extract → Validate → Create → Store pattern
5. **Facade** performs all validation checks
6. **Symbol Tables** accumulate validated metadata
7. **ErrorManager** collects all errors
8. **DecoratedAST** is returned with populated tables

### Key Design Patterns

| Pattern | Component | Purpose |
|---------|-----------|---------|
| **Visitor** | IASTVisitor | Separate algorithm from AST structure |
| **Facade** | SemanticCheckerFacade | Simplify validation interface |
| **Factory** | VarFactory, FuncFactory | Encapsulate object creation |
| **Singleton** | ErrorManager | Centralized error collection |

### Data Flow

```
AST Node
    │
    ├──► ASTExtractor ──► Raw Values
    │                         │
    │                         ▼
    │                    Factory ──► Metadata
    │                                   │
    │                                   ▼
    └──► Facade ◄───────────────── Validate
            │
            ├── OK ──► Symbol Table (store)
            │
            └── FAIL ──► ErrorManager (report)
```


