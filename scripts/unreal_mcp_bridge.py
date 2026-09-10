"""
M.E.G. Reclamation — Unreal MCP bridge (stdio).

1) Prefers the live UE 5.8 editor MCP at http://127.0.0.1:8000/mcp
2) Falls back to UnrealEditor-Cmd / Build.bat when the editor HTTP server is down.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
import urllib.error
import urllib.request

PROJECT_DIR = os.environ.get("MEG_PROJECT_DIR", r"F:\MEG_Reclamation")
UPROJECT = os.path.join(PROJECT_DIR, "MEG_Reclamation.uproject")
UE_EDITOR = os.environ.get(
    "MEG_UE_EDITOR",
    r"F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe",
)
UE_CMD = os.environ.get(
    "MEG_UE_CMD",
    r"F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
)
UE_MCP_URL = os.environ.get("MEG_UE_MCP_URL", "http://127.0.0.1:8000/mcp")

TOOLS = [
    {
        "name": "unreal_mcp_status",
        "description": "Ping le serveur MCP HTTP UE 5.8 (127.0.0.1:8000/mcp) et verifie les binaires editor.",
        "inputSchema": {"type": "object", "properties": {}},
    },
    {
        "name": "unreal_launch_editor",
        "description": "Lance UnrealEditor.exe sur MEG_Reclamation.uproject (MCP auto-start si plugin actif).",
        "inputSchema": {"type": "object", "properties": {}},
    },
    {
        "name": "unreal_build",
        "description": "Compile MEG_ReclamationEditor Win64 (Development ou Shipping).",
        "inputSchema": {
            "type": "object",
            "properties": {
                "configuration": {
                    "type": "string",
                    "enum": ["Development", "Shipping"],
                    "default": "Development",
                }
            },
        },
    },
    {
        "name": "unreal_exec_python",
        "description": "Execute un script Python dans UnrealEditor-Cmd (-ExecutePythonScript).",
        "inputSchema": {
            "type": "object",
            "properties": {
                "script_path": {"type": "string"},
            },
            "required": ["script_path"],
        },
    },
    {
        "name": "unreal_run_tests",
        "description": "Execute Run_Automation_Tests.ps1 (30 tests natifs).",
        "inputSchema": {"type": "object", "properties": {}},
    },
]


def _trim(text: str, limit: int = 6000) -> str:
    text = text or ""
    return text if len(text) <= limit else text[-limit:]


def ping_http_mcp() -> dict:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {
                "protocolVersion": "2024-11-05",
                "capabilities": {},
                "clientInfo": {"name": "meg-unreal-bridge", "version": "1.0.0"},
            },
        }
    ).encode("utf-8")
    req = urllib.request.Request(
        UE_MCP_URL,
        data=body,
        headers={"Content-Type": "application/json", "Accept": "application/json"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=3) as resp:
            raw = resp.read().decode("utf-8", errors="replace")
            return {"alive": True, "http_status": resp.status, "body": _trim(raw, 2000)}
    except urllib.error.HTTPError as exc:
        raw = exc.read().decode("utf-8", errors="replace") if exc.fp else ""
        return {"alive": True, "http_status": exc.code, "body": _trim(raw, 2000)}
    except Exception as exc:
        return {"alive": False, "error": f"{type(exc).__name__}: {exc}"}


def handle_status() -> str:
    info = ping_http_mcp()
    info.update(
        {
            "url": UE_MCP_URL,
            "uproject": UPROJECT,
            "editor_exists": os.path.isfile(UE_EDITOR),
            "cmd_exists": os.path.isfile(UE_CMD),
            "plugin": "ModelContextProtocol (Enabled in .uproject, bAutoStartServer=True)",
        }
    )
    if not info.get("alive"):
        info["hint"] = (
            "Ouvre Unreal Editor sur ce projet (ou appelle unreal_launch_editor). "
            "Le plugin ModelContextProtocol doit demarrer http://127.0.0.1:8000/mcp. "
            "Dans la console editor: ModelContextProtocol.StartServer"
        )
    return json.dumps(info, indent=2)


def handle_launch() -> str:
    if not os.path.isfile(UE_EDITOR):
        return f"UnrealEditor introuvable: {UE_EDITOR}"
    subprocess.Popen(
        [UE_EDITOR, UPROJECT],
        cwd=PROJECT_DIR,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return (
        f"UnrealEditor lance: {UE_EDITOR} {UPROJECT}\n"
        "Attendre le chargement complet puis verifier unreal_mcp_status. "
        "Console: ModelContextProtocol.StartServer si le HTTP n'est pas encore up."
    )


def handle_build(cfg: str) -> str:
    bat = r"F:\UE_5.8\Engine\Build\BatchFiles\Build.bat"
    cmd = [bat, "MEG_ReclamationEditor", "Win64", cfg, UPROJECT, "-WaitMutex"]
    res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
    return f"exit={res.returncode}\n{_trim((res.stdout or '') + (res.stderr or ''))}"


def handle_exec_python(script_path: str) -> str:
    # 1. Command execution mode
    if script_path.startswith("cmd:"):
        cmd_str = script_path[4:].strip()
        res = subprocess.run(cmd_str, shell=True, capture_output=True, text=True, cwd=PROJECT_DIR)
        return f"exit={res.returncode}\n{_trim((res.stdout or '') + (res.stderr or ''))}"

    # 2. Inline Python execution mode
    if script_path.startswith("inline:") or "\n" in script_path:
        code = script_path[7:] if script_path.startswith("inline:") else script_path
        res = subprocess.run([sys.executable, "-c", code], capture_output=True, text=True, cwd=PROJECT_DIR)
        return f"exit={res.returncode}\n{_trim((res.stdout or '') + (res.stderr or ''))}"

    if not os.path.isfile(script_path):
        return f"Script introuvable: {script_path}"

    # Check if script explicitly requests UnrealEditor-Cmd
    use_ue_cmd = False
    try:
        with open(script_path, "r", encoding="utf-8", errors="ignore") as sf:
            first_line = sf.readline()
            if "UE_CMD" in first_line or "unreal." in sf.read():
                use_ue_cmd = True
    except Exception:
        pass

    if use_ue_cmd:
        cmd = [
            UE_CMD,
            UPROJECT,
            f"-ExecutePythonScript={script_path}",
            "-NullRHI",
            "-unattended",
            "-nosplash",
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
    else:
        res = subprocess.run([sys.executable, script_path], capture_output=True, text=True, cwd=PROJECT_DIR)

    return f"exit={res.returncode}\n{_trim((res.stdout or '') + (res.stderr or ''))}"


def handle_tests() -> str:
    cmd = [
        "powershell",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        os.path.join(PROJECT_DIR, "Run_Automation_Tests.ps1"),
    ]
    res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
    return f"exit={res.returncode}\n{_trim((res.stdout or '') + (res.stderr or ''))}"


def handle_tool_call(name: str, args: dict) -> dict:
    try:
        if name == "unreal_mcp_status":
            text = handle_status()
        elif name == "unreal_launch_editor":
            text = handle_launch()
        elif name == "unreal_build":
            text = handle_build(args.get("configuration") or "Development")
        elif name == "unreal_exec_python":
            text = handle_exec_python(args.get("script_path") or "")
        elif name == "unreal_run_tests":
            text = handle_tests()
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
                    "serverInfo": {"name": "meg-unreal-bridge", "version": "1.0.0"},
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
