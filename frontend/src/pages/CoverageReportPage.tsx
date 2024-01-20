import { routes } from "#/routes";
import { isDev } from "#/environment";
import { useEffect, useState } from "react";
import { parseCoverage } from "#/coverage/CoverageParser";
import {
    CoverageReportT,
    DirectoryReportT,
    FileReportT,
    RootT,
} from "../../../_generated/frontend/coverpp/report";
import { FileCoverage, LineCoverage } from "#/coverage/FileCoverage";
import RemoteCoverageCodeBlock from "#/components/RemoteCoverageCodeBlock";

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
        <CoverageRoot key={root.path as string} root={root} />
    ));

    // return <RemoteCoverageCodeBlock coverage={fileContent.content[0]} />;
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
