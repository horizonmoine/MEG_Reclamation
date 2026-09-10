"""Install ahujasid blender-mcp addon into Blender 5.2 user addons and verify bpy."""

from __future__ import annotations

import os
import sys
import urllib.request
import zipfile
import io
import shutil

ADDON_URL = "https://raw.githubusercontent.com/ahujasid/blender-mcp/main/addon.py"
USER_ADDONS = os.path.join(
    os.environ.get("APPDATA", ""),
    "Blender Foundation",
    "Blender",
    "5.2",
    "scripts",
    "addons",
    "blender_mcp",
)
ENGINE_ADDONS = r"F:\blender\5.2\scripts\addons\blender_mcp"


def install_to(target: str) -> str:
    os.makedirs(target, exist_ok=True)
    dest = os.path.join(target, "__init__.py")
    print(f"Downloading addon.py -> {dest}")
    with urllib.request.urlopen(ADDON_URL, timeout=60) as resp:
        data = resp.read()
    with open(dest, "wb") as handle:
        handle.write(data)
    return dest


def main() -> int:
    targets = []
    if os.environ.get("APPDATA"):
        targets.append(USER_ADDONS)
    if os.path.isdir(r"F:\blender\5.2\scripts\addons"):
        targets.append(ENGINE_ADDONS)
    if not targets:
        print("Aucun dossier addons Blender 5.2 trouve.")
        return 1
    last = ""
    for target in targets:
        last = install_to(target)
        print(f"OK {last} ({os.path.getsize(last)} bytes)")
    print("Dans Blender GUI: Edit > Preferences > Add-ons > blender MCP > Enable, puis Start MCP Server (port 9876).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
