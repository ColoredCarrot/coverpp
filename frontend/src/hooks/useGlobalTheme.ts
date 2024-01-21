import { useEffect, useState } from "react";

export type Theme = "light" | "dark";

const ATTR_NAME = "data-theme";

function getCurrentTheme(): Theme {
    const attr = document.body.getAttribute(ATTR_NAME);
    return attr === "dark" ? "dark" : "light";
}

// More themes (e.g. toggling between three) would be possible
function getNextTheme(current: Theme = getCurrentTheme()): Theme {
    return current === "light" ? "dark" : "light";
}

export function toggleGlobalTheme() {
    document.body.setAttribute(ATTR_NAME, getNextTheme());
}

export function useGlobalTheme(): Theme {
    const [globalTheme, setGlobalTheme] = useState(getCurrentTheme);

    useEffect(() => {
        const observer = new MutationObserver(mutations => {
            for (const mutation of mutations) {
                if (
                    mutation.type === "attributes" &&
                    mutation.attributeName === ATTR_NAME
                ) {
                    setGlobalTheme(getCurrentTheme);
                }
            }
        });

        observer.observe(document.body, {
            attributes: true,
            attributeFilter: [ATTR_NAME],
        });

        return () => observer.disconnect();
    }, []);

    return globalTheme;
}
