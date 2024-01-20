function cls(...classes: readonly (string | [string, boolean])[]): string {
    return classes
        .flatMap(c => (typeof c === "string" ? [c] : c[1] ? [c[0]] : []))
        .join(" ");
}

export default cls;
