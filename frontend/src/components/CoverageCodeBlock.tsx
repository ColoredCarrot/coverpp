import CodeBlock from "#/components/CodeBlock";
import { FileCoverage } from "#/coverage/FileCoverage";

export default function CoverageCodeBlock(props: {
    coverage: FileCoverage;
    content: string;
}) {
    return <CodeBlock content={props.content} coverage={props.coverage.lines} />;
}
