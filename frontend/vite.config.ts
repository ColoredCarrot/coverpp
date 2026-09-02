import path from "node:path";
import react from "@vitejs/plugin-react";
import { defineConfig } from "vite";
import svgr from "vite-plugin-svgr";
import fs from "node:fs";
// import { viteSingleFile } from "vite-plugin-singlefile";

function ensureSameDrive(inputPath: string): void {
    if (process.platform !== "win32") {
        return;
    }

    const inputDrive = path.win32
        .parse(inputPath)
        .root.match(/^[A-Za-z]:/)?.[0];
    if (!inputDrive) {
        return;
    }

    const cwdDrive = path.win32
        .parse(process.cwd())
        .root.match(/^[A-Za-z]:/)?.[0];
    if (!cwdDrive) {
        return;
    }

    if (inputDrive.toLowerCase() !== cwdDrive.toLowerCase()) {
        throw new Error(
            `Vite's /@fs/ handling only supports files on the same drive as the current working directory (${cwdDrive}), but the report path is on ${inputDrive} `,
        );
    }
}

function getPathToDevReport() {
    const file = path.resolve(__dirname, "../cmake-build-debug/report.coverpp");

    if (!fs.existsSync(file)) {
        throw new Error(
            `Cover++ report file for development not found: ${file}`,
        );
    }

    ensureSameDrive(file);

    return `String.raw\`${file}\``;
}

export default defineConfig(({ mode }) => ({
    plugins: [
        react({
            babel: { plugins: ["@babel/plugin-syntax-import-attributes"] },
        }),
        svgr(),
        // viteSingleFile(),
    ],
    resolve: {
        alias: {
            "#": path.resolve(__dirname, "./src"),
        },
    },
    server: {
        headers: {
            // Required for WASM
            "Cross-Origin-Opener-Policy": "same-origin",
            "Cross-Origin-Embedder-Policy": "require-corp",
        },
        fs: {
            strict: mode !== "development",
        },
        // Only listen on localhost; don't expose ourselves to the outside world
        host: "localhost",
    },
    define: {
        // If running under a dev server, there is no Cover++ server behind us, so we cannot access /ctx
        // Instead, inject the absolute path to an example report file
        __DEV_ABSOLUTE_PATH__:
            mode === "development" ? getPathToDevReport() : undefined,
    },
}));
