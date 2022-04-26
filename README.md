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
  - cmd_calc
  - cmd_draw
- Max
  - cmd_counter
  - expr
  - term
  - cond/cond_s
- Felix
  - [done] operand 
    - added `"rand" "(" EXPR "," EXPR ")"` 
    - removed `"[" EXPR "]"`
  - [done] args -> fill_args
