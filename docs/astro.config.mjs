// @ts-check
import { defineConfig } from "astro/config";
import starlight from "@astrojs/starlight";

// https://astro.build/config
export default defineConfig({
    site: process.env.ASTRO_SITE,
    base: process.env.ASTRO_BASE,

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
            favicon: `${process.env.ASTRO_BASE ?? ""}/favicon.png`,
            editLink: {
                baseUrl:
                    "https://github.com/ColoredCarrot/coverpp/edit/master/docs/",
            },
        }),
    ],
});
