import { type ReactNode } from "react";
import "./styles.css";
import GlobalThemeToggle from "#/components/GlobalThemeToggle";

export default function DefaultLayout(props: { children: ReactNode }) {
    return (
        <>
            <main>{props.children}</main>
            <GlobalThemeToggle />
        </>
    );
}
