import { ReactNode } from "@tanstack/react-router";
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
