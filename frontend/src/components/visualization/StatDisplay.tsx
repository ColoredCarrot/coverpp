import styles from "./StatDisplay.module.css";
import cls from "#/util/cls";

export default function StatDisplay(props: {
    value: number;
    unit: string;
    className?: string;
}) {
    return (
        <div className={cls(styles.StatDisplay, props.className)}>
            {formatValue(props.value) + " " + props.unit}
        </div>
    );
}

function formatValue(
    value: number | undefined,
    options: { decimalPlaces?: number } = {},
): string {
    const decimalPlaces = options.decimalPlaces ?? 1;

    if (value === undefined) {
        return "???";
    }

    switch (Math.sign(value)) {
        case 0:
            return "0";
        case 1:
            return formatPositiveValue(value, decimalPlaces);
        case -1:
            return "-" + formatPositiveValue(-value, decimalPlaces);
        default:
            return "NaN";
    }
}

function formatPositiveValue(value: number, decimalPlaces: number) {
    if (value === Infinity) {
        return "Infinite";
    }

    const base = 1000;

    const i = Math.floor(Math.log(value) / Math.log(base));
    const requiresPrefix = i > 0;

    const num = value / Math.pow(base, i);
    const numStr = requiresPrefix ? num.toFixed(decimalPlaces) : num.toString();

    const unit = requiresPrefix ? "kMGTPEZY"[i - 1] : "";

    return `${numStr}${unit}`;
}
