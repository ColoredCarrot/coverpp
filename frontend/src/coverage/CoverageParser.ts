import * as flatbuffers from "flatbuffers";
import * as Report from "#/_generated/coverpp/report";
import { CoverageReportT } from "#/_generated/coverpp/report";

export function parseCoverage(raw: Uint8Array): CoverageReportT {
    const buffer = new flatbuffers.ByteBuffer(raw);
    const coverageReport =
        Report.CoverageReport.getRootAsCoverageReport(buffer).unpack();

    // Heuristic: Flatbuffers doesn't reject malformed input, but just returns a default-constructed object
    if (raw.length > 100 && !coverageReport.roots?.length) {
        throw new Error("Malformed coverage report");
    }

    return coverageReport;
    /*
    const res: FileCoverage[] = [];
    for (
        let i = 0, limit = coverageReport.fileReportsLength();
        i < limit;
        ++i
    ) {
        const fileReport = coverageReport.fileReports(i);
        if (fileReport === null || fileReport.reachableLines(0) === null) {
            continue;
        }

        const reachableLines = fileReport.reachableLinesArray()!;
        const coveredLines = fileReport.coveredLinesArray()!;

        const maxReachableLine = reachableLines.reduce(
            (acc, line) => Math.max(acc, line),
            0,
        );

        const lines = Array.from(Array(maxReachableLine + 1).keys()).map(
            line =>
                coveredLines.includes(line + 1)
                    ? reachableLines.includes(line + 1)
                        ? LineCoverage.Full
                        : LineCoverage.None
                    : LineCoverage.NotApplicable,
        );
        console.log(lines);

        res.push({
            filePath: fileReport.path()!,
            lines,
        });
    }
    return res;*/
}
