import sys
import json

with open(sys.argv[1], "r") as bb:
    machine = json.load(bb)

stateIDs = {}
c = 0
for key in machine.keys():
    stateIDs[key] = c
    c += 1

#Placeholders for compiler states
stateIDs["-1"] = -1
stateIDs["-2"] = -2
stateIDs["-3"] = -3
stateIDs["-10"] = -10

#Special states
stateIDs["HALT"] = -1
stateIDs["ERROR_STATE"] = -10

machineArr = []
snum = 0

for value in machine.values():
    machineArr += [snum]
    machineArr += [value["blankWrite"]]
    machineArr += [value["blankShift"]]
    machineArr += [stateIDs[value["blankState"]]]
    machineArr += [value["oneWrite"]]
    machineArr += [value["oneShift"]]
    machineArr += [stateIDs[value["oneState"]]]
    snum += 1

machineTxt = str(len(machine)) + "\n"

for i in range(len(machineArr)):
    if (machineArr[i] == 'r') or (machineArr[i] == 'R'):
        machineArr[i] = 1
    elif (machineArr[i] == 'l') or (machineArr[i] == 'L'):
        machineArr[i] = -1
    elif machineArr[i] == 'HALT':
        machineArr[i] = -1
    machineTxt += str(machineArr[i]) + "\n"

with open("machine.txt", "w") as f:
    f.write(machineTxt)