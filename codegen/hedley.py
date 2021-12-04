import re
import sys
from pathlib import Path

hedley = Path(sys.argv[1])

with open(hedley) as f:
    lines = f.readlines()

defined_macros = dict()

for line in lines:
    line: str
    copy = line
    if line.startswith("#"):
        copy = line[1:].lstrip()
    if copy[:6] == "define":
        copy = copy[6:].lstrip()
        macro_name: str = re.split(" |\(|\n", copy)[0]
        defined_macros[macro_name] = None

for macro in defined_macros:
    print(f"#undef {macro}")
