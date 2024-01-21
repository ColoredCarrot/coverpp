import { ReactNode } from "@tanstack/react-router";
import styles from "./LinkButton.module.css";

export default function LinkButton(props: {
    onClick: () => void | Promise<void>;
    children: ReactNode;
}) {
    return (
        <button
            type="button"
            className={styles.LinkButton}
            onClick={props.onClick}
        >
            {props.children}
        </button>
    );
}
