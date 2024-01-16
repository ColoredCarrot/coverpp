import { Outlet } from "@tanstack/react-router";
import DefaultLayout from "#/layouts/DefaultLayout";

export function Root() {
    return (
        <DefaultLayout>
            <Outlet />
        </DefaultLayout>
    );
}
