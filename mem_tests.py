"""
ATTRIBUTION: Danny Piper
========================

This is a modified version of my script from github.com/MonarchDevelopment/SquireDesktop
used to automate valgrind usage. It is licenced under AGPL3 (printed below)

========================
The AGPLv3 License (AGPLv3)

Copyright (c) 2022 Author

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import subprocess
import os
import sys

NO_LEAKS = "All heap blocks were freed -- no leaks are possible"
TEST_EXEC_NAME = "test_benchmarking_h"
VALGRIND_OPTS = "--leak-check=full --show-leak-kinds=all --track-fds=yes"  # all" silly ubuntu has no all


def tests():
    print(
        f"Running memcheck for {TEST_EXEC_NAME}",
    )

    p = subprocess.Popen(
        f"valgrind {VALGRIND_OPTS} ./{TEST_EXEC_NAME}",
        shell=True,
        bufsize=4096,
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdout=subprocess.DEVNULL,
    )

    output = ""
    for __line in p.stderr:
        line = __line.decode("UTF-8")
        print(f">> {line[:-1]}")
        output += line
    p.wait()

    if NO_LEAKS in output and p.returncode == 0:
        return 0
    else:
        return 1


def main():
    a = tests()
    if a == 1:
        print("Tests failed")
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
