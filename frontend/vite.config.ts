import path from "node:path";
import react from "@vitejs/plugin-react";
import { defineConfig } from "vite";
import svgr from "vite-plugin-svgr";
// import { viteSingleFile } from "vite-plugin-singlefile";

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
            // Allow access to the Cover++ project directory
            allow: [".."],
        },
        // Only listen on localhost; don't expose ourselves to the outside world
        host: "localhost",
    },
    define: {
        // If running under a dev server, there is no Cover++ server behind us, so we cannot access /ctx
        // Instead, inject the absolute path to an example report file
        __DEV_ABSOLUTE_PATH__:
            mode === "development"
                ? `String.raw\`${path.resolve(__dirname, "../cmake-build-debug-visual-studio/Debug/coverpp-report/report.coverpp")}\``
                : undefined,
    },
}));
