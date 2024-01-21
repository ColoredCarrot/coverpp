function cls(
    ...classes: readonly (string | [string, boolean] | undefined)[]
): string {
    return classes
        .flatMap(c =>
            c === undefined
                ? []
                : typeof c === "string"
                  ? [c]
                  : c[1]
                    ? [c[0]]
                    : [],
        )
        .join(" ");
}

export default cls;
