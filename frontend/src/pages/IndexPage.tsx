import { Suspense } from "react";
import RemoteCoverageCodeBlock from "#/components/RemoteCoverageCodeBlock";
import { LineCoverage } from "#/coverage/FileCoverage";

export default function IndexPage() {
    return (
        <Suspense fallback={"Highlighting..."}>
            <RemoteCoverageCodeBlock
                coverage={{
                    filePath:
                        "G:\\Voidev\\Official\\Projects\\C++\\Cover++\\example-sut\\sut-main.cpp",
                    lines: [
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.Full,
                        LineCoverage.None,
                        LineCoverage.None,
                        LineCoverage.None,
                        LineCoverage.None,
                        LineCoverage.None,
                        LineCoverage.None,
                        LineCoverage.None,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                        LineCoverage.Partial,
                    ],
                }}
            />
        </Suspense>
    );
}
