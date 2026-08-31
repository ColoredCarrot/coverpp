// @ts-check
import { defineConfig } from "astro/config";
import { unified } from "@astrojs/markdown-remark";
import starlight from "@astrojs/starlight";
import { visit } from "unist-util-visit";

/** @import { Root } from "hast" */

/** @type {string|undefined} */
const base = process.env.ASTRO_BASE || undefined;

/**
 * @type {() => (tree: Root) => void}
 */
const prefixBaseUrls = () => {
    if (!base) {
        return () => {};
    }

    let effectiveBase = base;
    if (!effectiveBase.startsWith("/")) {
        effectiveBase = "/" + effectiveBase;
    }
    if (effectiveBase.endsWith("/")) {
        effectiveBase = effectiveBase.slice(0, -1);
    }

    return ast => {
        visit(ast, "element", node => {
            if (node.tagName === "a") {
                const url = node.properties.href;
                if (
                    typeof url === "string" &&
                    url.startsWith("/") &&
                    !url.startsWith("//") &&
                    !url.startsWith(`${effectiveBase}/`)
                ) {
                    node.properties.href = effectiveBase + url;
                }
            }
            // TODO also images with src
        });
    };
};

// https://astro.build/config
export default defineConfig({
    site: process.env.ASTRO_SITE,
    base: base,

    // This is better for deployment on GitHub Pages
    trailingSlash: "always",
    build: {
        format: "directory",
    },

    markdown: {
        processor: unified({
            rehypePlugins: [prefixBaseUrls],
        }),
    },

    integrations: [
        starlight({
            title: "Cover++ Documentation",
            sidebar: [
                {
                    label: "Guides",
                    items: [
                        "guides/getting-started",
                        "guides/installation",
                        "guides/relocation",
                    ],
                },
                {
                    label: "Reference",
                    items: [{ autogenerate: { directory: "reference" } }],
                },
            ],
            favicon: `${base ?? ""}/favicon.png`,
            editLink: {
                baseUrl:
                    "https://github.com/ColoredCarrot/coverpp/edit/master/docs/",
            },
            social: [
                {
                    label: "GitHub",
                    icon: "github",
                    href: "https://github.com/ColoredCarrot/coverpp",
                },
            ],
        }),
    ],
});
