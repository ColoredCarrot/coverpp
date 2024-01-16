import path from "node:path";
import react from "@vitejs/plugin-react";
import { UserConfig } from "vite";
import svgr from "vite-plugin-svgr";

export default {
    plugins: [
        react({
            babel: { plugins: ["@babel/plugin-syntax-import-attributes"] },
        }),
        svgr(),
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
    },
} satisfies UserConfig;
