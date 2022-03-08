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

- alle Non-Terminals sind eine Funktion
- jede Funktion gibt ``double`` zurück

```txt
PROGRAM    ::= { PATHDEF | CALCDEF } begin STATEMENTS end
PATHDEF    ::= path NAME [ "("[ PARAMS ]")" ] STATEMENTS endpath
CALCDEF    ::= calculation NAME "("[ PARAMS ]")" [ STATEMENTS ] returns EXPR endcalc
PARAMS     ::= [ VAR { "," VAR } ] (*Parameter-Liste für jede Funktion erforderlich für Eindeutigkeit der Namen*)
NAME       ::= [@] ( "a"..."z" | "A"..."Z" | "_" ) { "a"..."z" | "A"..."Z" | "_" | "0"..."9" }
VAR        ::= NAME
COND       ::= 
EXPR       ::= TERM { ( - | + ) TERM }
TERM       ::= FAKTOR { ( * | / ) FAKTOR }
FAKTOR     ::= <operand> [ ^ FAKTOR ]
OPERAND    ::= [ - ] ( "sqrt" | "sin" | "cos" | "tan" "(" EXPR ")" | "(" EXPR ")" | "|" EXPR "|" | "[" EXPR "]" | <value> | <var> ) 
ARGS       ::= [ EXPR { "," EXPR } ]
STATEMENTS ::= 
```
