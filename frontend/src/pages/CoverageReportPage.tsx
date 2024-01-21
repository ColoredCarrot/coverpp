import { useEffect, useMemo, useState } from "react";
import {
    CoverageReportT,
    DirectoryReportT,
    FileReportT,
    RootT,
    StatsT,
} from "#/_generated/coverpp/report";
import CoverageTable, { Scope } from "#/components/CoverageTable";
import CoverageStatsOverview from "#/components/visualization/CoverageStatsOverview";
import { parseCoverage } from "#/coverage/CoverageParser";
import { LineCoverage } from "#/coverage/FileCoverage";
import { isDev } from "#/environment";
import { routes } from "#/routes";

const makeSourceFileUrl: (sourceFile: string) => string = isDev()
    ? sourceFile => "/@fs/" + sourceFile
    : sourceFile => "/src/" + sourceFile;

type FileContent =
    | { status: "fetching" }
    | { status: "ready"; content: CoverageReportT }
    | { status: "error"; code: number };

/*===================================== TODO remove

   EXAMPLE URL

   http://localhost:5173/report/G:/Voidev/Official/Projects/C++/Cover++/cmake-build-debug-visual-studio/Debug/coverpp-report/report.coverpp

*/

function useReport(reportPath: string): FileContent {
    const [fileContent, setFileContent] = useState<FileContent>({
        status: "fetching",
    });

    useEffect(() => {
        const controller = new AbortController();
        const signal = controller.signal;

        setFileContent({ status: "fetching" });

        (async () => {
            const response = await fetch(makeSourceFileUrl(reportPath));

            if (response.status !== 200) {
                setFileContent({ status: "error", code: response.status });
                return;
            }

            const rawContent = new Uint8Array(await response.arrayBuffer());

            signal.throwIfAborted();

            setFileContent({
                status: "ready",
                content: parseCoverage(rawContent),
            });
        })().catch(reason => {
            if (
                reason instanceof DOMException &&
                reason.name === "AbortError"
            ) {
                // Ignore
            } else {
                throw reason;
            }
        });

        return () => controller.abort();
    }, [reportPath]);

    return fileContent;
}

export default function CoverageReportPage() {
    const { _splat: reportPath } = routes.coverageReport.useParams();

    const fileContent = useReport(reportPath);

    if (fileContent.status === "fetching") {
        return <p>Fetching...</p>;
    }

    if (fileContent.status === "error") {
        return <p>Error</p>;
    }

    const report = fileContent.content;

    return report.roots.map(root => (
        <Root key={root.path as string} root={root} />
    ));
}

function Root(props: { root: RootT }) {
    const [scopes, leafRegistry] = useMemo((): [Scope[], FileReportT[]] => {
        const leafRegistry: FileReportT[] = [];
        return [[getScope(props.root, leafRegistry)], leafRegistry];
    }, [props.root]);

    return (
        <div>
            <h2>Code Coverage Report</h2>
            <CoverageStatsOverview stats={props.root.stats ?? new StatsT()} />
            <div style={{ height: "2rem" }} />
            <CoverageTable
                scopes={scopes}
                pathSeparator={(props.root.directorySeparator ?? "/") as string}
                getCoverageLines={leafId =>
                    toFileCoverageLines(leafRegistry[leafId])
                }
            />
        </div>
    );
}

function getScope(
    report: RootT | DirectoryReportT | FileReportT,
    leafRegistry: FileReportT[],
): Scope {
    if (report instanceof DirectoryReportT) {
        const stats = report.stats ?? new StatsT();
        return {
            path: report.name as string,
            totalCovered: Number(stats.totalCovered ?? 0),
            totalReachable: Number(stats.totalReachable ?? 0),
            children: report.children?.map(child =>
                getScope(child, leafRegistry),
            ),
        };
    }

    if (report instanceof FileReportT) {
        const id = leafRegistry.length;
        leafRegistry.push(report);
        return {
            path: report.path as string,
            totalCovered: report.coveredLines?.length ?? 0,
            totalReachable: report.reachableLines?.length ?? 0,
            leafId: id,
        };
    }

    const stats = report.stats ?? new StatsT();
    return {
        path: report.path as string,
        totalCovered: Number(stats.totalCovered),
        totalReachable: Number(stats.totalReachable),
        children: report.children?.map(child => getScope(child, leafRegistry)),
    };
}

function toFileCoverageLines(fileReport: FileReportT): LineCoverage[] {
    const reachableLines = fileReport.reachableLines ?? [];
    const coveredLines = fileReport.coveredLines ?? [];

    const maxReachableLine = reachableLines.reduce(
        (acc, line) => Math.max(acc, line),
        0,
    );

    return Array.from(Array(maxReachableLine + 1).keys()).map(line =>
        reachableLines.includes(line + 1)
            ? coveredLines.includes(line + 1)
                ? LineCoverage.Full
                : LineCoverage.None
            : LineCoverage.NotApplicable,
    );
}
