import { useId } from "react";
import GaugeComponent from "react-gauge-component";
import styles from "./CoverageStatsOverview.module.css";
import { StatsT } from "#/_generated/coverpp/report/stats";
import StatDisplay from "#/components/visualization/StatDisplay";
import cls from "#/util/cls";
import * as Shape from "@visx/shape";
import * as Scale from "@visx/scale";
import { Group } from "@visx/group";
import { ParentSize } from "@visx/responsive";
import { Text } from "@visx/text";
import Gauge, {neutralBlue, redYellowGreen} from "#/components/visualization/Gauge";

export default function CoverageStatsOverview(props: { stats: StatsT }) {
    return (
        <div className={styles.CoverageStatsOverview}>
            <CoverppGauge
                value={Number(props.stats.totalCovered) / Number(props.stats.totalReachable)}
                colors="redYellowGreen"
                className={cls(styles.col1, styles.Display)}
            />
            <div className={cls(styles.col1, styles.Label)}>
                Total coverage
            </div>

            <CoverppGauge
                value={Number(props.stats.totalReachable) / Number(props.stats.totalLines)}
                colors="neutral"
                className={cls(styles.col2, styles.Display)}
            />
            <div className={cls(styles.col2, styles.Label)}>
                Code ratio
            </div>

            {/*<StatDisplay
                className={cls(styles.CoveredLoC, styles.Display)}
                value={Number(props.stats.totalCovered)}
                unit={"LoC"}
            />
            <div className={cls(styles.CoveredLoC, styles.Label)}>
                Total covered lines
            </div>*/}

            <StatDisplay
                className={cls(styles.ReachableLoC, styles.Display)}
                value={Number(props.stats.totalReachable)}
                unit={"LoC"}
            />
            <div className={cls(styles.ReachableLoC, styles.Label)}>
                Total code lines
            </div>
        </div>
    );
}

const percentCoveredNumberFormat = new Intl.NumberFormat("en-US", {
    style: "percent",
    maximumFractionDigits: 1,
});

function CoverppGauge(props: {
    value: number;
    colors: "redYellowGreen" | "neutral";
    className?: string;
}) {
    return (
        <ParentSize className={props.className}>
            {({ width }) => (
                <Gauge
                    width={width}
                    value={props.value}
                    formatValue={percentCoveredNumberFormat.format.bind(
                        percentCoveredNumberFormat,
                    )}
                    colors={props.colors === "redYellowGreen" ? redYellowGreen : neutralBlue}
                />
            )}
        </ParentSize>
    );
}

function OLD_CoveragePercentGauge(props: {
    stats: StatsT;
    className?: string;
}) {
    const id = useId();

    return (
        <GaugeComponent
            id={id}
            className={props.className ?? ""}
            arc={{
                colorArray: ["#EA4228", "#F5CD19", "#5BE12C"],
                subArcs: [
                    { length: 1 / 3 },
                    { length: 1 / 3 },
                    { length: 1 / 3 },
                ],
            }}
            value={
                Number(props.stats.totalCovered ?? 0) /
                Number(props.stats.totalReachable ?? 0)
            }
            minValue={0}
            maxValue={1}
            labels={{
                valueLabel: {
                    matchColorWithArc: true,
                    formatTextValue: (v: number) =>
                        percentCoveredNumberFormat.format(v),
                    maxDecimalDigits: 10,
                },
                tickLabels: {
                    defaultTickValueConfig: {
                        formatTextValue: (v: number) =>
                            percentCoveredNumberFormat.format(v),
                    },
                },
            }}
        />
    );
}
