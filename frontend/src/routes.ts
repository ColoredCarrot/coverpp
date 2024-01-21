import { RootRoute, Route, lazyRouteComponent } from "@tanstack/react-router";
import IndexPage from "#/pages/IndexPage";
import { Root } from "#/Root";
import { parseCoverage } from "#/coverage/CoverageParser";
import { CoverageReportT } from "#/_generated/coverpp/report/coverage-report";
import { isDev } from "#/environment";

const root = new RootRoute({
    component: Root,
});

type IndexRouteLoaderData =
    | { status: "error"; statusCode: number; statusText: string }
    | {
          status: "ready";
          report: CoverageReportT;
      };

const index = new Route({
    getParentRoute: () => root,
    path: "/",
    component: IndexPage,
    loader: async ({ abortController }): Promise<IndexRouteLoaderData> => {
        const url = isDev()
            ? "/@fs/G:/Voidev/Official/Projects/C++/Cover++/cmake-build-debug-visual-studio/Debug/coverpp-report/report.coverpp"
            : "/ctx/report";

        const response = await fetch(url, {
            signal: abortController.signal,
        });

        if (response.status !== 200) {
            return {
                status: "error",
                statusCode: response.status,
                statusText: response.statusText,
            };
        }

        const rawContent = new Uint8Array(await response.arrayBuffer());
        abortController.signal.throwIfAborted();
        return { status: "ready", report: parseCoverage(rawContent) };
    },
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
