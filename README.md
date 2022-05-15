# compilerbau-pkk

## Prerequisites

- cmake >= 3.21
- make
- SDL2

## Build instructions

- run `build.sh`
- binary can be found in `./build`

## Documentation

- run `doxygen`
- inspect documentation under `./docs`

## Bericht zum Projektverlauf

Nach einigen Startschwierigkeiten war der Lexer nach relativ kurzer Zeit funktionstüchtig.
All die Parser Funktionen zu implementieren hat sehr viel Zeit beansprucht und hat für viele Probleme gesorgt.
Daher musste zum Schluss sehr intensives Debugging betrieben werden, um Fehler zu beheben.
Da die Umsetzung insgesamt länger gedauert hat, als erwartet, konnte die geplante deutsche Übersetzung der Sprache nicht mehr realisiert werden.

Zunächst wurden die Hinweise, welche Informationen in welche Söhne geschrieben werden müssen, übersehen.
Das führte zu dazu, dass häufig der Code des Evaluators analysiert wurde.

Insgesamt konnte das Projekt jedoch fertiggestellt und auf einen funktionstüchtigen Stand gebracht werden.
Das Programm wurde mit den, sich in `./tests/` befindlichen, Testprogrammen getestet.
