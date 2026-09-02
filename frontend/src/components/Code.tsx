import type { Element, ElementContent } from "hast";
import { toJsxRuntime } from "hast-util-to-jsx-runtime";
import { memo, useDeferredValue } from "react";
import { Fragment, jsx, jsxs } from "react/jsx-runtime";
import styles from "./Code.module.css";
import "./themes/tritanopia.css";
import { LineCoverage } from "#/coverage/FileCoverage";
import useStarryNight from "#/hooks/useStarryNight";
import { starryNightGutter } from "#/util/hast-util-starry-night-gutter";

function makeCreateLine(coverage: LineCoverage[]) {
    return (children: ElementContent[], line: number): Element => {
        const className = [styles.Line];
        if (line <= coverage.length) {
            const cov = coverage[line - 1];
            switch (cov) {
                case LineCoverage.Full:
                    className.push(styles.CovFull);
                    break;
                case LineCoverage.Partial:
                    className.push(styles.CovPartial);
                    break;
                case LineCoverage.None:
                    className.push(styles.CovNone);
                    break;
            }
        }

        return {
            type: "element",
            tagName: "span",
            properties: { className, dataLineNumber: line },
            children,
        };
    };
}

export type CodeProps = {
    content: string;
    coverage?: LineCoverage[];
};

// memo because starry night highlighting is very expensive
const Code = memo(function Code(props: CodeProps) {
    const starryNight = useStarryNight();

    // Since highlighting takes a long time, we could fall back to Prism.js highlighting while it's loading

    const content = useDeferredValue(props.content);

    const tree = starryNight.highlight(content, "source.c++");
    starryNightGutter(
        tree,
        props.coverage ? makeCreateLine(props.coverage) : undefined,
    );
    const element = toJsxRuntime(tree, { Fragment, jsx, jsxs });
    return <code className={styles.Code}>{element}</code>;
});

export default Code;
