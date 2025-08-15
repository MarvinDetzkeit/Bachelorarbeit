import sys
import pyperclip

with open("_BusyCCODE/" + sys.argv[1], "r") as code:
    c_code = code.read().replace("halt", "return 0")

c_code = list(c_code)
mode = 0
for i in range(len(c_code)):
    if c_code[i] == "/":
        mode = 1
    elif c_code[i] == "\n":
        mode = 0
    if mode == 1:
        c_code[i] = ""

c_code = "".join(c_code)


declarationList = []

c_code = c_code.split(";")
for i in range(len(c_code)):
    buf = c_code[i].split(":")
    for j in range(len(buf)):
        if "--" in buf[j]:
            buf[j] = buf[j].replace("--", "")
            buf[j] = f"{buf[j]} = decrement({buf[j].split(' ')[-1].strip()})"
        if buf[j].strip().split(" ")[0] == "uint":
            declarationList += [buf[j].strip()]
            buf[j] = ""
    for j in range(len(buf)):
        if buf[j] != "":
            buf[j] += ":"
    buf[-1] = buf[-1].replace(":", ";")
    c_code[i] = ""
    for x in buf:
        c_code[i] += x


newCode = ""
for d in declarationList:
    newCode += d + ";\n"
for e in c_code:
    newCode += e

c_code = f"""#include <stdio.h>
#define uint unsigned int

uint decrement(uint a) {{
    if (a > 0) a--;
    return a;
}}

int main() {{
{newCode}


return 0;
}}"""

pyperclip.copy(c_code)
