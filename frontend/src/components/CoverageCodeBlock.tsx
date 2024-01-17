import { FileCoverage } from "#/coverage/FileCoverage";
import CodeBlock from "#/components/CodeBlock";

export default function CoverageCodeBlock(props: {
    coverage: FileCoverage;
    content: string;
}) {
    return <CodeBlock content={props.content} coverage={props.coverage.lines} />;
}
