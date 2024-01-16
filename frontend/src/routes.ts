import {
    RootRoute,
    Route,
} from "@tanstack/react-router";
import { Root } from "#/Root";
import IndexPage from "#/pages/IndexPage";

const root = new RootRoute({
    component: Root,
});

const index = new Route({
    getParentRoute: () => root,
    path: "/",
    component: IndexPage,
});

export const routes = {
    root,
    index,
} as const;
