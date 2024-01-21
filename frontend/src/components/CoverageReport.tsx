import { useMemo } from "react";
import CoverageTable, { Scope } from "#/components/CoverageTable";
import CoverageStatsOverview from "#/components/visualization/CoverageStatsOverview";
import { LineCoverage } from "#/coverage/FileCoverage";
import { CoverageReportT } from "#/_generated/coverpp/report/coverage-report";
import { RootT } from "#/_generated/coverpp/report/root";
import { FileReportT } from "#/_generated/coverpp/report/file-report";
import { StatsT } from "#/_generated/coverpp/report/stats";
import { DirectoryReportT } from "#/_generated/coverpp/report/directory-report";

export default function CoverageReport(props: { report: CoverageReportT }) {
    if (props.report.roots.length === 0) {
        return <p>No data</p>;
    }

    return props.report.roots.map(root => (
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
            <div style={{ height: "3rem" }} />
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
