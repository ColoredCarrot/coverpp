export default function formatByteSize(
    bytes: number | undefined,
    options: { si?: boolean; decimalPlaces?: number } = {},
): string {
    const si = options.si ?? true;
    const decimalPlaces = options.decimalPlaces ?? 2;

    if (bytes === undefined) {
        return "???";
    }

    switch (Math.sign(bytes)) {
        case 0:
            return "0 B";
        case 1:
            return formatPositiveByteSize(bytes, si, decimalPlaces);
        case -1:
            return "-" + formatPositiveByteSize(-bytes, si, decimalPlaces);
        default:
            return "NaN";
    }
}

function formatPositiveByteSize(
    bytes: number,
    si: boolean,
    decimalPlaces: number,
) {
    if (bytes === Infinity) {
        return "Infinite";
    }

    const base = si ? 1000 : 1024;

    const kb: readonly [string, string] = si ? ["k", "B"] : ["K", "iB"];

    const i = Math.floor(Math.log(bytes) / Math.log(base));
    const requiresPrefix = i > 0;

    const num = bytes / Math.pow(base, i);
    const numStr = requiresPrefix ? num.toFixed(decimalPlaces) : num.toString();

    const unit = requiresPrefix ? (kb[0] + "MGTPEZY")[i - 1] + kb[1] : "B";

    return `${numStr} ${unit}`;
}
