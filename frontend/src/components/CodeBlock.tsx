import Code from "#/components/Code";
import styles from "./CodeBlock.module.css";

export default function CodeBlock(props: { content: string }) {
    return (
        <pre className={styles.CodeBlock}>
            <Code content={props.content} />
        </pre>
    );
}
