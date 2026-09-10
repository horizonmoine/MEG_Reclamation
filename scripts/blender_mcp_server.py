"""
M.E.G. Reclamation — Blender 5.2 MCP (stdio JSON-RPC 2.0)

Always available: drives F:\\blender\\blender.exe headless (bpy) without a GUI addon.
Optional live check: BlenderMCP socket on 127.0.0.1:9876 if the GUI addon is running.
"""

from __future__ import annotations

import json
import os
import socket
import subprocess
import sys
import tempfile

BLENDER_EXE = os.environ.get("MEG_BLENDER_EXE", r"F:\blender\blender.exe")
PROJECT_DIR = os.environ.get("MEG_PROJECT_DIR", r"F:\MEG_Reclamation")
ADDON_HOST = os.environ.get("BLENDER_HOST", "127.0.0.1")
ADDON_PORT = int(os.environ.get("BLENDER_PORT", "9876"))

TOOLS = [
    {
        "name": "blender_status",
        "description": "Statut Blender: exe, version, socket addon 9876, chemins projet.",
        "inputSchema": {"type": "object", "properties": {}},
    },
    {
        "name": "blender_exec_python",
        "description": (
            "Execute un script Python bpy dans Blender 5.2 en mode headless (-b). "
            "Passer python_code (source) ou script_path (fichier .py existant)."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "python_code": {
                    "type": "string",
                    "description": "Source Python bpy a executer.",
                },
                "script_path": {
                    "type": "string",
                    "description": "Chemin absolu vers un .py existant.",
                },
                "blend_file": {
                    "type": "string",
                    "description": "Fichier .blend optionnel a ouvrir avant le script.",
                },
            },
        },
    },
    {
        "name": "blender_addon_command",
        "description": (
            "Envoie une commande JSON au serveur addon BlenderMCP (port 9876) si Blender GUI est ouvert."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "type": {"type": "string", "description": "Type de commande addon (ex: get_scene_info)."},
                "params": {"type": "object", "description": "Parametres JSON de la commande."},
            },
            "required": ["type"],
        },
    },
]


def _trim(text: str, limit: int = 8000) -> str:
    if not text:
        return ""
    if len(text) <= limit:
        return text
    return text[-limit:]


def blender_socket_alive() -> bool:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(0.4)
    try:
        sock.connect((ADDON_HOST, ADDON_PORT))
        return True
    except OSError:
        return False
    finally:
        sock.close()


def send_addon_command(cmd_type: str, params: dict | None = None) -> str:
    payload = json.dumps({"type": cmd_type, "params": params or {}})
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(30.0)
    try:
        sock.connect((ADDON_HOST, ADDON_PORT))
        sock.sendall(payload.encode("utf-8"))
        chunks: list[bytes] = []
        while True:
            data = sock.recv(8192)
            if not data:
                break
            chunks.append(data)
        return b"".join(chunks).decode("utf-8", errors="replace")
    finally:
        sock.close()


def handle_blender_status() -> str:
    exe_ok = os.path.isfile(BLENDER_EXE)
    version = ""
    if exe_ok:
        res = subprocess.run(
            [BLENDER_EXE, "--version"],
            capture_output=True,
            text=True,
            timeout=30,
        )
        version = (res.stdout or res.stderr or "").splitlines()[:3]
        version = " | ".join(version)
    return json.dumps(
        {
            "blender_exe": BLENDER_EXE,
            "exe_exists": exe_ok,
            "version": version,
            "project_dir": PROJECT_DIR,
            "addon_socket": f"{ADDON_HOST}:{ADDON_PORT}",
            "addon_alive": blender_socket_alive(),
        },
        indent=2,
    )


def handle_exec_python(args: dict) -> str:
    script_path = args.get("script_path") or ""
    python_code = args.get("python_code") or ""
    blend_file = args.get("blend_file") or ""
    temp_path = None

    if not script_path and not python_code:
        return "Erreur: fournir python_code ou script_path."

    if python_code:
        fd, temp_path = tempfile.mkstemp(suffix="_meg_blender.py", text=True)
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            handle.write(python_code)
        script_path = temp_path

    if not os.path.isfile(script_path):
        return f"Erreur: script introuvable: {script_path}"

    if not os.path.isfile(BLENDER_EXE):
        return f"Erreur: Blender introuvable: {BLENDER_EXE}"

    cmd = [BLENDER_EXE, "-b"]
    if blend_file:
        cmd.append(blend_file)
    cmd.extend(["--python", script_path])

    try:
        res = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            cwd=PROJECT_DIR,
            timeout=300,
        )
        out = _trim((res.stdout or "") + "\n" + (res.stderr or ""))
        return f"exit={res.returncode}\n{out}"
    finally:
        if temp_path:
            try:
                os.remove(temp_path)
            except OSError:
                pass


def handle_tool_call(name: str, args: dict) -> dict:
    try:
        if name == "blender_status":
            text = handle_blender_status()
        elif name == "blender_exec_python":
            text = handle_exec_python(args)
        elif name == "blender_addon_command":
            if not blender_socket_alive():
                text = (
                    "Addon BlenderMCP inactif sur 127.0.0.1:9876. "
                    "Ouvre Blender, active l'addon blenderMCP, Start MCP Server. "
                    "En attendant, utilise blender_exec_python (headless)."
                )
            else:
                text = send_addon_command(args.get("type", ""), args.get("params") or {})
        else:
            return {
                "content": [{"type": "text", "text": f"Unknown tool: {name}"}],
                "isError": True,
            }
        return {"content": [{"type": "text", "text": text}]}
    except Exception as exc:
        return {
            "content": [{"type": "text", "text": f"{type(exc).__name__}: {exc}"}],
            "isError": True,
        }


def reply(msg_id, result=None, error=None):
    payload = {"jsonrpc": "2.0", "id": msg_id}
    if error is not None:
        payload["error"] = error
    else:
        payload["result"] = result
    sys.stdout.write(json.dumps(payload) + "\n")
    sys.stdout.flush()


def main() -> None:
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        line = line.strip()
        if not line:
            continue
        try:
            req = json.loads(line)
        except json.JSONDecodeError:
            continue

        method = req.get("method")
        msg_id = req.get("id")

        if method == "initialize":
            reply(
                msg_id,
                {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {"tools": {}},
                    "serverInfo": {"name": "meg-blender-mcp", "version": "1.0.0"},
                },
            )
        elif method == "notifications/initialized":
            pass
        elif method == "ping":
            reply(msg_id, {})
        elif method == "tools/list":
            reply(msg_id, {"tools": TOOLS})
        elif method == "tools/call":
            params = req.get("params") or {}
            reply(msg_id, handle_tool_call(params.get("name"), params.get("arguments") or {}))
        elif method in ("resources/list", "prompts/list"):
            reply(msg_id, {method.split("/")[0]: []})
        elif msg_id is not None:
            reply(msg_id, error={"code": -32601, "message": f"Method '{method}' not found"})


if __name__ == "__main__":
    main()
