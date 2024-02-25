import { useMemo } from "react";
import { CoverageReportT } from "#/_generated/coverpp/report/coverage-report";
import { DirectoryReportT } from "#/_generated/coverpp/report/directory-report";
import { FileReportT } from "#/_generated/coverpp/report/file-report";
import { RootT } from "#/_generated/coverpp/report/root";
import { StatsT } from "#/_generated/coverpp/report/stats";
import CoverageTable, { Scope } from "#/components/CoverageTable";
import CoverageStatsOverview from "#/components/visualization/CoverageStatsOverview";
import { LineCoverage } from "#/coverage/FileCoverage";

export default function CoverageReport({
    report,
}: {
    report: CoverageReportT;
}) {
    const [scopes, leafRegistry] = useMemo((): [Scope[], FileReportT[]] => {
        const leafRegistry: FileReportT[] = [];
        return [
            report.roots.map(root => getRootScope(root, leafRegistry)),
            leafRegistry,
        ];
    }, [report.roots]);

    if (report.roots.length === 0) {
        return <p>No data</p>;
    }

    return (
        <div>
            <h2>Code Coverage Report</h2>
            <p>
                Generated {formatDateTime(new Date(Number(report.timestamp)))}
            </p>
            <CoverageStatsOverview stats={report.stats ?? new StatsT()} />
            <div style={{ height: "3rem" }} />
            <CoverageTable
                scopes={scopes}
                pathSeparator={
                    // TODO support different separators per root
                    (report.roots[0].directorySeparator ?? "/") as string
                }
                getCoverageLines={leafId =>
                    toFileCoverageLines(leafRegistry[leafId])
                }
            />
        </div>
    );
}

const monthName = (() => {
    const format = new Intl.DateTimeFormat("en-US", { month: "short" });
    return format.format.bind(format);
})();

function formatDateTime(theDate: Date) {
    const weekday = [
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
    ][theDate.getDay()];

    function pad(n: number) {
        return n < 10 ? "0" + n : n;
    }

    const time = `${pad(theDate.getHours())}:${pad(theDate.getMinutes())}`;
    const date = `${pad(theDate.getDate())} ${monthName(theDate)} ${theDate.getFullYear()}`;

    return `on ${weekday}, ${date}, at ${time}`;
}

function getRootScope(report: RootT, leafRegistry: FileReportT[]): Scope {
    const stats = report.stats ?? new StatsT();
    const directorySeparator = (report.directorySeparator || "/") as string;
    return {
        path: report.path as string,
        fullPath: report.path as string,
        totalCovered: Number(stats.totalCovered),
        totalReachable: Number(stats.totalReachable),
        children: report.children?.map(child =>
            getScope(
                child,
                leafRegistry,
                report.path + directorySeparator,
                directorySeparator,
            ),
        ),
    };
}

function getScope(
    report: DirectoryReportT | FileReportT,
    leafRegistry: FileReportT[],
    prefix: string,
    directorySeparator: string,
): Scope {
    if (report instanceof DirectoryReportT) {
        const stats = report.stats ?? new StatsT();
        return {
            path: report.name as string,
            fullPath: prefix + report.name,
            totalCovered: Number(stats.totalCovered ?? 0),
            totalReachable: Number(stats.totalReachable ?? 0),
            children: report.children?.map(child =>
                getScope(
                    child,
                    leafRegistry,
                    prefix + report.name + directorySeparator,
                    directorySeparator,
                ),
            ),
        };
    }

    const id = leafRegistry.length;
    leafRegistry.push(report);
    return {
        path: report.path as string,
        fullPath: prefix + report.path,
        totalCovered: report.coveredLines?.length ?? 0,
        totalReachable: report.reachableLines?.length ?? 0,
        leafId: id,
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
