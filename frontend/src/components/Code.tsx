import useStarryNight from "#/hooks/useStarryNight";
import { toJsxRuntime } from "hast-util-to-jsx-runtime";
import { Fragment, jsx, jsxs } from "react/jsx-runtime";
import { memo } from "react";
import "./themes/tritanopia.css";
import { starryNightGutter } from "#/util/hast-util-starry-night-gutter";
import styles from "./Code.module.css";

// memo because starry night highlighting is very expensive
const Code = memo(function Code(props: { content: string }) {
    const starryNight = useStarryNight();

    // Since highlighting takes a long time, we could fall back to Prism.js highlighting while it's loading

    const tree = starryNight.highlight(props.content, "source.c++");
    starryNightGutter(tree);
    const element = toJsxRuntime(tree, { Fragment, jsx, jsxs });
    return <code className={styles.Code}>{element}</code>;
});

export default Code;
