/**
 * Safe Resilient MCP Proxy for Antigravity IDE
 * Seamlessly connects to named pipes when available,
 * or gracefully responds to MCP handshake without error when inactive.
 */
const net = require("net");
const os = require("os");
const path = require("path");
const readline = require("readline");

const target = process.argv[2] || "default";
const pipePath = path.isAbsolute(target) || target.startsWith("\\\\?\\pipe\\")
    ? target
    : process.platform === "win32"
        ? path.join("\\\\?\\pipe\\", `datacloud-mcp-${target}`)
        : path.join(os.tmpdir(), `datacloud-mcp-${target}.sock`);

let socket = net.createConnection(pipePath);

socket.on("connect", () => {
    process.stdin.pipe(socket);
    socket.pipe(process.stdout);
});

socket.on("error", () => {
    // Pipe not running — provide graceful fallback MCP server
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        terminal: false
    });

    rl.on("line", (line) => {
        line = line.trim();
        if (!line) return;
        try {
            const req = JSON.parse(line);
            const id = req.id;
            const method = req.method;

            if (method === "initialize") {
                console.log(JSON.stringify({
                    jsonrpc: "2.0",
                    id: id,
                    result: {
                        protocolVersion: "2024-11-05",
                        capabilities: { tools: {} },
                        serverInfo: { name: `datacloud-${target}`, version: "1.0.0" }
                    }
                }));
            } else if (method === "notifications/initialized") {
                // No response needed
            } else if (method === "tools/list") {
                console.log(JSON.stringify({
                    jsonrpc: "2.0",
                    id: id,
                    result: { tools: [] }
                }));
            } else if (method === "ping") {
                console.log(JSON.stringify({ jsonrpc: "2.0", id: id, result: {} }));
            } else if (id !== undefined) {
                console.log(JSON.stringify({
                    jsonrpc: "2.0",
                    id: id,
                    result: {}
                }));
            }
        } catch (e) {
            // Ignore parse errors
        }
    });
});
