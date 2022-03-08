# compilerbau-pkk

## Specification

- Entwicklungsumgebung: vscode
- Build tool: cmake
- Extensions: ms-vscode.cpptools, luniclynx.bison, luniclynx.lex, twxs.cmake, ms-vscode.cmake-tools
- mit bison/lex probieren, bei zu großen Problemen zu C wechseln
- umgebung: linux (nativ oder wsl)

## Todo

- cmake init
- sdl einbinden + kusche header

## Token-Übersetzung

| Original | iel |
| -------- | --- |

## EBNF

```txt
PROGRAM ::= { PATHDEF | CALCDEF } begin STATEMENTS end
PATHDEF ::= path NAME [ ([ PARAMS ]) ] STATEMENTS endpath
CALCDEF ::= calculation NAME ([ PARAMS ]) [ STATEMENTS ] returns EXPR endcalc

```
