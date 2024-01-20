import { lazyRouteComponent, RootRoute, Route } from "@tanstack/react-router";
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

const coverageReport = new Route({
    getParentRoute: () => root,
    path: "/report/$",
    component: lazyRouteComponent(() => import("#/pages/CoverageReportPage")),
});

export const routes = {
    root,
    index,
    coverageReport,
} as const;
