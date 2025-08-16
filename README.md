# Tools der Arbeit nutzen

## Struktur

- `Simulator/` enthält den Code des Simulators
- `Compiler/` enthält den Code des Compilers
- `Lib/` enthält geteilten Code
- `_BusyCCode/` enthält BusyC-Programme
- `_machines/` enthält Turingmaschinen
- `_machines/compiler/` enthält Submaschinen für den Compiler
- `busyc-language/` enthält eine VS Code Erweiterung für BusyC Syntax Highlighting (.bc-Dateien)
- `busyCToC.py` übersetzt BusyC-Programme in C-Programme
- `transformMachine.py` übersetzt eine Turingmaschine im JSON-Format in die Beschreibungssprache für diesen [Online-Simulator](https://turingmachine.io)

## Simulator

### Kompilieren

```zsh
make sim
```

### Ausführen

```zsh
./sim <maschine>
```

Der Simulator erwartet den Namen einer Maschinendatei als Argument. Dabei muss der Suffix `.json` weggelassen werden. Die Datei wird im `_machines/`-Ordner gesucht.

#### Beispiel
Simulieren der Maschine `_machines/bb5.json`:

```zsh
./sim bb5
```

#### Ändern der Blockgröße
Die Blockgröße wird durch die Konstante `COMPRESSIONSIZE` in der Datei `Simulator/simulator.h` festgelegt. Nach dem Ändern muss der Simulator neu kompiliert werden. Die Blockgröße muss kleiner als die Anzahl der Bits des Bandtyps (uint32_t) sein. Praktisch war auf dem Entwicklungsrechner maximal 27 möglich wegen Speicherverbrauch.

## Compiler

### Kompilieren

```zsh
make comp
```

### Ausführen

```zsh
./comp <programm>.bc
```

Der Compiler sucht das Programm im Ordner `_BusyCCode/`. Die kompilierte Maschine wird im Ordner `_machines/` abgelegt.

#### Beispiel
Kompilieren des Programms `_BusyCCode/isEven.bc`:

```zsh
./comp isEven.bc
```

Die kompilierte Maschine wird als `_machines/isEven.json` gespeichert.

#### Submaschinen
Die Dateien für die Submaschinen liegen im Ordner `_machines/compiler/`. Diese werden während der Kompilierung geladen und in die Maschine eingebaut. Ohne diese Dateien funktioniert der Compiler nicht.

## BusyC
Eine Einführung in BusyC gibt es in Kapitel 4 der Arbeit `MarvinDetzkeit_bachelorthesis.pdf`.

## VS Code Erweiterung
Die Erweiterung fügt BusyC Syntax Highlighting für .bc-Dateien hinzu.

### Installieren

```zsh
code --install-extension busyc-language/busyc-language-0.0.1.vsix
```

### Deinstallieren
Über den Erweiterungen-Tab in VS Code.

## Python-Skripte
Beide Skripte nutzen die externe Bibliothek _pyperclip_, um die Ausgabe in die Zwischenablage zu schreiben.

### `busyCToC.py `

```zsh
python3 busyCToC.py <programm>.bc
```

Das Programm wird im `_BusyCCode/`-Ordner gesucht.

### `transformMachine.py`

```zsh
python3 transformMachine.py <maschine>.json
```

Die Maschine wird im `_machines/`-Ordner gesucht.
