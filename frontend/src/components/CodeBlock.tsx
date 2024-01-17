import Code, { CodeProps } from "#/components/Code";
import styles from "./CodeBlock.module.css";

export default function CodeBlock(props: CodeProps) {
    return (
        <pre className={styles.CodeBlock}>
            <Code {...props} />
        </pre>
    );
}
