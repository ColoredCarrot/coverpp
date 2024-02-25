import { RouterProvider, createRouter } from "@tanstack/react-router";
import React from "react";
import { createRoot as createReactRoot } from "react-dom/client";
import { routes } from "#/routes";

const routeTree = routes.root.addChildren([
    routes.index,
    routes.coverageReport,
]);

const router = createRouter({
    routeTree,
    defaultPreload: "intent",
});

declare module "@tanstack/react-router" {
    // noinspection JSUnusedGlobalSymbols
    interface Register {
        router: typeof router;
    }
}

createReactRoot(document.getElementById("root")!).render(
    <React.StrictMode>
        <RouterProvider router={router} />
    </React.StrictMode>,
);
