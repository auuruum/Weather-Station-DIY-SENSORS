Import("env")

import io
import os


def _patch_file(path: str) -> bool:
    with io.open(path, "r", encoding="utf-8", newline="") as f:
        content = f.read()

    old = "add((Text)en);"
    new = "add(en.toText());"

    if new in content:
        return False
    if old not in content:
        return False

    content = content.replace(old, new)
    with io.open(path, "w", encoding="utf-8", newline="") as f:
        f.write(content)
    return True


def main() -> None:
    libdeps_dir = env.subst("$PROJECT_LIBDEPS_DIR")
    pioenv = env.subst("$PIOENV")

    packet_h = os.path.join(libdeps_dir, pioenv, "Settings", "src", "core", "packet.h")
    if not os.path.isfile(packet_h):
        return

    _patch_file(packet_h)


main()
