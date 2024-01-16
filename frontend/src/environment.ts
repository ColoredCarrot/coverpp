export function isDev(): boolean {
    // noinspection JSUnresolvedReference
    return import.meta.env.DEV;
}

export function isProd(): boolean {
    return !isDev();
}
