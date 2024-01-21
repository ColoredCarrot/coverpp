import { NotFoundRoute, Router, RouterProvider } from "@tanstack/react-router";
import React from "react";
import { createRoot as createReactRoot } from "react-dom/client";
import NotFoundPage from "#/pages/NotFoundPage";
import { routes } from "#/routes";

const routeTree = routes.root.addChildren([
    routes.index,
    routes.coverageReport,
]);

const notFoundRoute = new NotFoundRoute({
    getParentRoute: () => routes.root,
    component: NotFoundPage,
});

const router = new Router({
    routeTree,
    defaultPreload: "intent",
    notFoundRoute,
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
