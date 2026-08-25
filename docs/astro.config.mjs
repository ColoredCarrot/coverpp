// @ts-check
import { defineConfig } from "astro/config";
import starlight from "@astrojs/starlight";

// https://astro.build/config
export default defineConfig({
    trailingSlash: "never",
    integrations: [
        starlight({
            title: "Cover++ Documentation",
            sidebar: [
                {
                    label: "Guides",
                    items: [{ autogenerate: { directory: "guides" } }],
                },
                {
                    label: "Reference",
                    items: [{ autogenerate: { directory: "reference" } }],
                },
            ],
            favicon: "/favicon.png",
        }),
    ],
});
