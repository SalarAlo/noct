# noct

A tree-walking interpreter for a dynamically typed programming language, written in C++23.

## Features

- **Types**: nil, booleans, numbers (double), strings, functions, classes, instances
- **Variables**: lexical scoping with shadowing (`var x = 1;`)
- **Expressions**: arithmetic, comparison, equality, logical (`and`/`or`), ternary (`? :`), unary, prefix/postfix `++`/`--`
- **Control flow**: `if`/`else`, `while`, `for` (desugared to `while`), `break`
- **Functions**: named functions, lambdas (`fn(params) { body }`), closures, recursion
- **Classes**: single-dispatch OOP with `this`, constructor via `init`, dynamic fields
- **Built‑ins**: `print`, `maybe` (50/50 random boolean)
- **Comments**: `// line` and `/* block */`

## Example

```noct
class Node {
  fn init(val) {
    this.val = val;
    this.next = nil;
  }
}

class List {
  fn init() {
    this.head = nil;
  }

  fn push(val) {
    var node = Node(val);
    node.next = this.head;
    this.head = node;
  }

  fn print() {
    var cur = this.head;
    while (cur != nil) {
      print cur.val;
      cur = cur.next;
    }
  }
}

var list = List();
list.push(3);
list.push(2);
list.push(1);
list.print();
```

## Quick Start

```sh
# Build
./generate.sh

# Run a script
./run.sh path/to/script.noct
```

## Tests

```sh
cmake --build build && build/NoctTests
```

or

```sh
cd build && ctest --output-on-failure
```

## Build Dependencies

- **CMake** ≥ 3.20
- **Ninja** build system
- **fmt** (external, found via `find_package`)
- C++23 compiler

## Project Structure

| Path | Description |
|---|---|
| `src/Main.cpp` | Entry point |
| `src/noct/Run.cpp` | Pipeline: lex → parse → resolve → interpret |
| `src/noct/lexer/` | Scanner / tokens |
| `src/noct/parser/` | Recursive‑descent parser → AST |
| `src/noct/parser/expression/` | Expression AST nodes |
| `src/noct/parser/statement/` | Statement AST nodes |
| `src/noct/resolver/` | Semantic analysis (variable resolution, error detection) |
| `src/noct/interpreter/` | Tree‑walking interpreter |
| `src/noct/Environment.h` | Lexical scope with slot‑based storage |
| `vendor/doctest/` | Header‑only test framework |
| `tests/` | ~100 test cases across 10 files |

## License

MIT
