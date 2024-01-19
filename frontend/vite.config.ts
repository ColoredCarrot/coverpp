import path from "node:path";
import react from "@vitejs/plugin-react";
import { UserConfig } from "vite";
import svgr from "vite-plugin-svgr";
// import { viteSingleFile } from "vite-plugin-singlefile";

export default {
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
} satisfies UserConfig;
