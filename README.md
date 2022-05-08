# compilerbau-pkk

## Specification

- Entwicklungsumgebung: vscode / CLion
- Build tool: cmake
- Extensions: ms-vscode.cpptools, luniclynx.bison, luniclynx.lex, twxs.cmake, ms-vscode.cmake-tools
- händisch mit C
- umgebung: GNU/Linux
- Source Files **müssen** in LF gespeichert werden

## Token-Übersetzung

| Original | iel |
| -------- | --- |

## EBNF

- alle Non-Terminals sind eine Funktion
- jede Funktion gibt ``double`` zurück

Siehe [hier](syntax.bnf).

## TODO
- Oliver
  - [x] cmd_calc
  - [x] cmd_draw
- Max
  - [x] cmd_counter
  - [x] expr
  - [x] term
  - [x] cond/cond_s
- Felix
  - [x] operand 
    - added `"rand" "(" EXPR "," EXPR ")"` 
    - removed `"[" EXPR "]"`
  - [x] args -> fill_args
