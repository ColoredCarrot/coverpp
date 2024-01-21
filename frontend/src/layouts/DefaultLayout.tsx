import { ReactNode } from "@tanstack/react-router";
import "./styles.css";

export default function DefaultLayout(props: { children: ReactNode }) {
    return <main>{props.children}</main>;
}
