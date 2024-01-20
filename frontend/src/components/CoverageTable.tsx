import styles from "./CoverageTable.module.css";
import { IconChevronDown, IconChevronRight } from "@tabler/icons-react";

function cls(...classes: readonly string[]): string {
    return classes.join(" ");
}

export default function CoverageTable() {
    return (
        <div className={styles.CoverageTable}>
            <div className={cls(styles.Row, styles.Head)}>
                <div />
                <div>Source file</div>
                <div>Covered</div>
                <div>Total</div>
                <div>Percent</div>
            </div>

            <div className={styles.Row}>
                <div>
                    <IconChevronRight size={"1.2rem"} />
                </div>
                <div>
                    <code>/project/src/foo/main.cpp</code>
                </div>
                <div>142</div>
                <div>190</div>
                <div>71%</div>
            </div>

            <div className={styles.Row}>
                <div>
                    <IconChevronDown size={"1.2rem"} />
                </div>
                <div>
                    <code>/project/src/foo/lib</code>
                </div>
                <div>142</div>
                <div>190</div>
                <div>71%</div>
            </div>

            <div style={{ "--indent": 1 }}>
                <div className={styles.Row}>
                    <div>
                        <IconChevronRight size={"1.2rem"} />
                    </div>
                    <div>
                        <code>string_helper.cpp</code>
                    </div>
                    <div>142</div>
                    <div>190</div>
                    <div>71%</div>
                </div>
            </div>

            <div className={styles.Row}>
                <div>
                    <IconChevronRight size={"1.2rem"} />
                </div>
                <div>
                    <code>/project/src/foo/main.cpp</code>
                </div>
                <div>142</div>
                <div>190</div>
                <div>71%</div>
            </div>
        </div>
    );
}
