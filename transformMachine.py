import json
import sys
import pyperclip

with open("_machines/" + sys.argv[1], "r") as bb:
    machine = json.load(bb)


firstState = next(iter(machine))

codeStr = f"""input: ''
blank: 0
start state: {firstState}
table:\n"""

for key, value in machine.items():
    codeStr += f"  {key}:\n"
    codeStr += f"    0: {{write: {value['blankWrite']}, {value['blankShift'].upper()}: {value['blankState']}}}\n"
    codeStr += f"    1: {{write: {value['oneWrite']}, {value['oneShift'].upper()}: {value['oneState']}}}\n"

codeStr += "  HALT:\n  ERROR_STATE:\n"

pyperclip.copy(codeStr)