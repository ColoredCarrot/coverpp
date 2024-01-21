import styles from "./CodeBlock.module.css";
import Code, { CodeProps } from "#/components/Code";

export default function CodeBlock(props: CodeProps) {
    return (
        <pre className={styles.CodeBlock}>
            <Code {...props} />
        </pre>
    );
}
