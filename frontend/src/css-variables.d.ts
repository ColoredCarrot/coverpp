import "react";

declare module "react" {
    interface CSSProperties {
        // Allow custom properties prefixed with `--`, which are CSS variables.
        [key: `--${string}`]: string | number;
    }
}
