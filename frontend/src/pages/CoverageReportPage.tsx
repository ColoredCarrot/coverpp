import { routes } from "#/routes";
import { isDev } from "#/environment";
import { useEffect, useId, useMemo, useState } from "react";
import { parseCoverage } from "#/coverage/CoverageParser";
import {
    CoverageReportT,
    DirectoryReportT,
    FileReportT,
    RootT,
    StatsT,
} from "../../../_generated/frontend/coverpp/report";
import { FileCoverage, LineCoverage } from "#/coverage/FileCoverage";
import RemoteCoverageCodeBlock from "#/components/RemoteCoverageCodeBlock";
import GaugeComponent from "react-gauge-component";
import CoverageTable, { Scope } from "#/components/CoverageTable";

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
    // @ts-expect-error TaStack Router's type system doesn't appear to like _splat
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

    // return <RemoteCoverageCodeBlock coverage={fileContent.content[0]} />;
}

function Root(props: { root: RootT }) {
    const scopes = useMemo(() => [getScope(props.root)], [props.root]);
    //LargeStats stats={root.stats ?? new StatsT()} />
    //<CoverageRoot key={root.path as string} root={root} />
    return (
        <CoverageTable
            scopes={scopes}
            pathSeparator={(props.root.directorySeparator ?? "/") as string}
        />
    );
}

function getScope(report: RootT | DirectoryReportT | FileReportT): Scope {
    if (report instanceof DirectoryReportT) {
        const stats = report.stats ?? new StatsT();
        return {
            path: report.name as string,
            totalCovered: Number(stats.totalCovered ?? 0),
            totalReachable: Number(stats.totalReachable ?? 0),
            children: report.children?.map(getScope),
        };
    }

    if (report instanceof FileReportT) {
        return {
            path: report.path as string,
            totalCovered: report.coveredLines?.length ?? 0,
            totalReachable: report.reachableLines?.length ?? 0,
        };
    }

    const stats = report.stats ?? new StatsT();
    return {
        path: report.path as string,
        totalCovered: Number(stats.totalCovered),
        totalReachable: Number(stats.totalReachable),
        children: report.children?.map(getScope),
    };
}

function CoverageRoot(props: { root: RootT }) {
    const flattened = flatten(
        props.root.path as string,
        props.root.directorySeparator as string,
        props.root.children,
    );

    return (
        <div>
            <h4>{props.root.path as string}</h4>
            {flattened.map(fileCoverage => (
                <RemoteCoverageCodeBlock
                    key={fileCoverage.filePath}
                    coverage={fileCoverage}
                />
            ))}
        </div>
    );
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

function flatten(
    path: string,
    dirSeparator: string,
    reports: readonly (DirectoryReportT | FileReportT)[] | undefined,
): readonly FileCoverage[] {
    if (reports === undefined) {
        return [];
    }

    return reports.flatMap(report =>
        report instanceof FileReportT
            ? [
                  {
                      filePath: path + dirSeparator + report.path,
                      lines: toFileCoverageLines(report),
                  } satisfies FileCoverage,
              ]
            : flatten(
                  path + dirSeparator + report.name,
                  dirSeparator,
                  report.children,
              ),
    );
}

function CoverageDirectoryOrFile(props: {
    report: DirectoryReportT | FileReportT;
}) {
    return props.report instanceof DirectoryReportT ? (
        <CoverageDirectory directory={props.report} />
    ) : (
        <></>
    );
}

function CoverageDirectory(props: { directory: DirectoryReportT }) {
    return <div></div>;
}

const percentCoveredNumberFormat = new Intl.NumberFormat("en-US", {
    style: "percent",
    maximumFractionDigits: 1,
});

function LargeStats(props: { stats: StatsT }) {
    const id = useId();

    return (
        <GaugeComponent
            id={id}
            arc={{
                colorArray: ["#EA4228", "#F5CD19", "#5BE12C"],
                subArcs: [
                    { length: 1 / 3 },
                    { length: 1 / 3 },
                    { length: 1 / 3 },
                ],
            }}
            value={
                Number(props.stats.totalCovered ?? 0) /
                Number(props.stats.totalReachable ?? 0)
            }
            minValue={0}
            maxValue={1}
            labels={{
                valueLabel: {
                    matchColorWithArc: true,
                    formatTextValue: (v: number) =>
                        percentCoveredNumberFormat.format(v),
                    maxDecimalDigits: 10,
                },
                tickLabels: {
                    defaultTickValueConfig: {
                        formatTextValue: (v: number) =>
                            percentCoveredNumberFormat.format(v),
                    },
                },
            }}
        />
    );
}
