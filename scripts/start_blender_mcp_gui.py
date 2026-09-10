"""Launch Blender GUI and start the BlenderMCP socket server on 9876 if the addon is installed."""

import bpy

try:
    bpy.ops.preferences.addon_enable(module="blender_mcp")
except Exception as exc:
    print("WARN: could not enable blender_mcp addon:", exc)

try:
    bpy.ops.blendermcp.start_server()
    print("BlenderMCP server start requested on 127.0.0.1:9876")
except Exception as exc:
    print("WARN: blendermcp.start_server failed:", exc)
    print("Enable the addon manually: Edit > Preferences > Add-ons > blender MCP")
