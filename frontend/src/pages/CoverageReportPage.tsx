import { useEffect, useState } from "react";
import { CoverageReportT } from "#/_generated/coverpp/report";
import CoverageReport from "#/components/CoverageReport";
import { parseCoverage } from "#/coverage/CoverageParser";
import { isDev } from "#/environment";
import { routes } from "#/routes";

const makeSourceFileUrl: (sourceFile: string) => string = isDev()
    ? sourceFile => "/@fs/" + sourceFile
    : sourceFile => "/src/" + sourceFile;

type FileContent =
    | { status: "fetching" }
    | { status: "ready"; content: CoverageReportT }
    | { status: "error"; code: number };

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

    const fileContent = useReport(reportPath ?? "");

    if (fileContent.status === "fetching") {
        return <p>Fetching...</p>;
    }

    if (fileContent.status === "error") {
        return <p>Error</p>;
    }

    return <CoverageReport report={fileContent.content} />;
}
