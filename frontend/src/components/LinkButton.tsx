import { type ReactNode } from "react";
import styles from "./LinkButton.module.css";
import cls from "#/util/cls";

export function InlineLinkButton(props: {
    onClick: () => void | Promise<void>;
    children: ReactNode;
    className?: string;
}) {
    return (
        <button
            type="button"
            className={cls(styles.InlineLinkButton, props.className)}
            onClick={props.onClick}
        >
            {props.children}
        </button>
    );
}

export default function LinkButton(props: {
    onClick: () => void | Promise<void>;
    children: ReactNode;
}) {
    return (
        <InlineLinkButton onClick={props.onClick} className={styles.LinkButton}>
            {props.children}
        </InlineLinkButton>
    );
}
