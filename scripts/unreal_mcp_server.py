"""
M.E.G. Reclamation — Native Unreal Engine 5.8 MCP Server
Implements Model Context Protocol (MCP) JSON-RPC 2.0 over Stdio.
Exposes project management, testing, compilation and python execution tools to Antigravity.
"""

import sys
import json
import subprocess
import os

PROJECT_DIR = r"F:\MEG_Reclamation"
PYTHON_UE = r"F:\UE_5.8\Engine\Binaries\ThirdParty\Python3\Win64\python.exe"
UE_CMD = r"F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
UPROJECT = r"F:\MEG_Reclamation\MEG_Reclamation.uproject"

TOOLS = [
    {
        "name": "unreal_build",
        "description": "Compile le projet C++ MEG_Reclamation en Development Win64 avec MSVC.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "configuration": {
                    "type": "string",
                    "enum": ["Development", "Shipping"],
                    "default": "Development",
                    "description": "Configuration de build"
                }
            }
        }
    },
    {
        "name": "unreal_run_tests",
        "description": "Execute la suite complete des 30 tests d'automatisation UE 5.8 natifs.",
        "inputSchema": {
            "type": "object",
            "properties": {}
        }
    },
    {
        "name": "unreal_audit_integrity",
        "description": "Execute l'audit complet du projet (31 controles d'integrite : maps, binaires, ruche multi-agents).",
        "inputSchema": {
            "type": "object",
            "properties": {}
        }
    },
    {
        "name": "unreal_exec_python",
        "description": "Execute un script Python directement dans l'environnement Unreal Engine 5.8.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "script_path": {
                    "type": "string",
                    "description": "Chemin absolu vers le script Python a executer dans UE5."
                }
            },
            "required": ["script_path"]
        }
    }
]

def handle_tool_call(name, args):
    if name == "unreal_build":
        cfg = args.get("configuration", "Development")
        cmd = [
            r"F:\UE_5.8\Engine\Build\BatchFiles\Build.bat",
            "MEG_ReclamationEditor", "Win64", cfg,
            UPROJECT, "-WaitMutex"
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
        return {
            "content": [{"type": "text", "text": f"Build exit code {res.returncode}:\n{res.stdout[-1500:]}"}]
        }
    elif name == "unreal_run_tests":
        cmd = ["powershell", "-ExecutionPolicy", "Bypass", "-File", r".\Run_Automation_Tests.ps1"]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
        return {
            "content": [{"type": "text", "text": f"Tests exit code {res.returncode}:\n{res.stdout[-2000:]}"}]
        }
    elif name == "unreal_audit_integrity":
        cmd = ["powershell", "-ExecutionPolicy", "Bypass", "-File", r".\Run_Auto_Check.ps1"]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
        return {
            "content": [{"type": "text", "text": f"Audit exit code {res.returncode}:\n{res.stdout[-2000:]}"}]
        }
    elif name == "unreal_exec_python":
        script_path = args.get("script_path", "")
        cmd = [UE_CMD, UPROJECT, f"-ExecutePythonScript={script_path}", "-NullRHI", "-unattended", "-nosplash"]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
        return {
            "content": [{"type": "text", "text": f"Python exec exit code {res.returncode}:\n{res.stdout[-2000:]}"}]
        }
    else:
        return {
            "content": [{"type": "text", "text": f"Unknown tool: {name}"}],
            "isError": True
        }

def main():
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        line = line.strip()
        if not line:
            continue
        try:
            req = json.loads(line)
        except Exception:
            continue

        method = req.get("method")
        msg_id = req.get("id")

        if method == "initialize":
            resp = {
                "jsonrpc": "2.0",
                "id": msg_id,
                "result": {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {
                        "tools": {}
                    },
                    "serverInfo": {
                        "name": "meg-unreal-engine-mcp",
                        "version": "1.0.0"
                    }
                }
            }
            sys.stdout.write(json.dumps(resp) + "\n")
            sys.stdout.flush()

        elif method == "notifications/initialized":
            pass

        elif method == "ping":
            resp = {"jsonrpc": "2.0", "id": msg_id, "result": {}}
            sys.stdout.write(json.dumps(resp) + "\n")
            sys.stdout.flush()

        elif method == "tools/list":
            resp = {
                "jsonrpc": "2.0",
                "id": msg_id,
                "result": {
                    "tools": TOOLS
                }
            }
            sys.stdout.write(json.dumps(resp) + "\n")
            sys.stdout.flush()

        elif method == "tools/call":
            params = req.get("params", {})
            name = params.get("name")
            arguments = params.get("arguments", {})
            result = handle_tool_call(name, arguments)
            resp = {
                "jsonrpc": "2.0",
                "id": msg_id,
                "result": result
            }
            sys.stdout.write(json.dumps(resp) + "\n")
            sys.stdout.flush()

        elif method in ["resources/list", "prompts/list"]:
            resp = {
                "jsonrpc": "2.0",
                "id": msg_id,
                "result": {
                    method.split("/")[0]: []
                }
            }
            sys.stdout.write(json.dumps(resp) + "\n")
            sys.stdout.flush()

        else:
            if msg_id is not None:
                resp = {
                    "jsonrpc": "2.0",
                    "id": msg_id,
                    "error": {
                        "code": -32601,
                        "message": f"Method '{method}' not found"
                    }
                }
                sys.stdout.write(json.dumps(resp) + "\n")
                sys.stdout.flush()

if __name__ == "__main__":
    main()
