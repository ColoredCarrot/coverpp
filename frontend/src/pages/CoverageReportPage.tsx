import { routes } from "#/routes";
import { isDev } from "#/environment";
import { useEffect, useState } from "react";
import RemoteCoverageCodeBlock from "#/components/RemoteCoverageCodeBlock";
import { parseCoverage } from "#/coverage/CoverageParser";
import { FileCoverage } from "#/coverage/FileCoverage";

const makeSourceFileUrl: (sourceFile: string) => string = isDev()
    ? sourceFile => "/@fs/" + sourceFile
    : sourceFile => "/src/" + sourceFile;

type FileContent =
    | { status: "fetching" }
    | { status: "ready"; content: FileCoverage[] }
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

    return <RemoteCoverageCodeBlock coverage={fileContent.content[0]} />;
}
