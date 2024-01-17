import { FileCoverage } from "#/coverage/FileCoverage";
import { useEffect, useState } from "react";
import { isDev } from "#/environment";
import CoverageCodeBlock from "#/components/CoverageCodeBlock";

const makeSourceFileUrl: (sourceFile: string) => string = isDev()
    ? sourceFile => "/@fs/" + sourceFile
    : sourceFile => "/src/" + sourceFile;

type FileContent =
    | { status: "fetching" }
    | { status: "ready"; content: string }
    | { status: "error"; code: number };

export default function RemoteCoverageCodeBlock(props: {
    coverage: FileCoverage;
}) {
    // Fetch file content
    const [fileContent, setFileContent] = useState<FileContent>({
        status: "fetching",
    });

    useEffect(() => {
        const controller = new AbortController();
        const signal = controller.signal;

        setFileContent({ status: "fetching" });

        (async () => {
            const response = await fetch(
                makeSourceFileUrl(props.coverage.filePath),
            );

            if (response.status !== 200) {
                setFileContent({ status: "error", code: response.status });
                return;
            }

            const content = await response.text();
            if (!signal.aborted) {
                setFileContent({ status: "ready", content });
            }
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
    }, [props.coverage.filePath]);

    if (fileContent.status === "fetching") {
        return <p>Fetching...</p>;
    }

    if (fileContent.status === "error") {
        return <p>Error</p>;
    }

    return (
        <CoverageCodeBlock
            coverage={props.coverage}
            content={fileContent.content}
        />
    );
}
