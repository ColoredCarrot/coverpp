import "./styles.css";
import { ReactNode } from "@tanstack/react-router";

export default function DefaultLayout(props: { children: ReactNode }) {
    return <main>{props.children}</main>;
}
