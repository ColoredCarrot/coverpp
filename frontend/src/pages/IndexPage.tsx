import { Suspense } from "react";
import CoverageCodeBlock from "#/components/CoverageCodeBlock";
import { LineCoverage } from "#/coverage/FileCoverage";

export default function IndexPage() {
    return (
        <Suspense fallback={"Highlighting..."}>
            <CoverageCodeBlock
                coverage={{
                    filePath:
                        "G:\\Voidev\\Official\\Projects\\C++\\Cover++\\example-sut\\sut-main.cpp",
                    lines: [LineCoverage.Full],
                }}
            />
        </Suspense>
    );
}
