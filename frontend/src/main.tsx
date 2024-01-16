import {
    createHashHistory,
    NotFoundRoute,
    Router,
    RouterProvider,
} from "@tanstack/react-router";
import React from "react";
import { createRoot as createReactRoot } from "react-dom/client";
import { routes } from "#/routes";
import NotFoundPage from "#/pages/NotFoundPage";

const routeTree = routes.root.addChildren([routes.index]);

const notFoundRoute = new NotFoundRoute({
    getParentRoute: () => routes.root,
    component: NotFoundPage,
});

const router = new Router({
    routeTree,
    defaultPreload: "intent",
    notFoundRoute,
    history: createHashHistory(),
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
