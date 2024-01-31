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

export default function CoverageStatsOverview(props: { stats: StatsT }) {
    return (
        <div className={styles.CoverageStatsOverview}>
            <CoveragePercentGauge
                stats={props.stats}
                className={cls(styles.Percent, styles.Display)}
            />
            <div className={cls(styles.Percent, styles.Label)}>
                Total coverage
            </div>

            <OLD_CoveragePercentGauge
                stats={props.stats}
                className={cls(styles.CoveredLOC, styles.Display)}
            />
            <div className={cls(styles.CoveredLOC, styles.Label)}>
                Total coverage
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

const getColor = Scale.scaleQuantile({
    domain: [0, 1],
    range: ["#EA4228", "#F5CD19", "#5BE12C"],
});

const angles = {
    start: -Math.PI / 1.6,
    end: Math.PI / 1.6,
} as const;

interface Margins {
    left: number;
    right: number;
    top: number;
    bottom: number;
}

interface GaugeProps {
    width: number;
    outerArcWidth: number;
    outerArcToInnerMargin: number;
    innerArcWidth: number;
    margins: Margins;
    textMarginInline: number;
}

interface GaugeMath {
    boundingWidth: number;
    boundingHeight: number;
    innerRadius: number;
    outermostRadius: number;
    heightBelowCenterline: number;
}

function calculateGauge(props: GaugeProps): GaugeMath {
    const innerRadius = Math.max(
        (props.width -
            props.margins.left -
            props.margins.right -
            2 *
                (props.innerArcWidth +
                    props.outerArcToInnerMargin +
                    props.outerArcWidth)) /
            2,
        1,
    );

    const innerArcOuterRadius = innerRadius + props.innerArcWidth;

    const outermostRadius =
        innerArcOuterRadius + props.outerArcToInnerMargin + props.outerArcWidth;

    const heightBelowCenterline =
        outermostRadius * Math.sin(angles.end - Math.PI / 2);

    return {
        boundingWidth: props.width,
        boundingHeight:
            outermostRadius +
            heightBelowCenterline +
            props.margins.top +
            props.margins.bottom,
        outermostRadius,
        heightBelowCenterline,
        innerRadius,
    };
}

function Gauge({
    width,
    outerArcWidth = 5,
    outerArcToInnerMargin = 2,
    innerArcWidth = 20,
    margins = {
        left: 40,
        right: 40,
        top: 10,
        bottom: 10,
    },
    textMarginInline = 20,
    value: normalizedValue, // value in [0, 1]
}: {
    width: number;
    value: number;
} & Partial<GaugeProps>) {
    const math = calculateGauge({
        width,
        outerArcWidth,
        outerArcToInnerMargin,
        innerArcWidth,
        margins,
        textMarginInline,
    });

    const innerArcOuterRadius = math.innerRadius + innerArcWidth;

    return (
        <svg width={width} height={math.boundingHeight}>
            <Group top={math.outermostRadius + margins.top} left={width / 2}>
                {/* Outer arc (labels) */}
                <Shape.Pie
                    data={getColor.range()}
                    outerRadius={math.outermostRadius}
                    innerRadius={innerArcOuterRadius + outerArcToInnerMargin}
                    cornerRadius={0}
                    padAngle={0}
                    startAngle={angles.start}
                    endAngle={angles.end}
                    fill={datum => datum.data}
                    pieValue={() => 1}
                />

                {/* Inner arc (gauge) */}
                <Shape.Pie
                    data={[normalizedValue, 1 - normalizedValue]}
                    innerRadius={math.innerRadius}
                    outerRadius={innerArcOuterRadius}
                    startAngle={angles.start}
                    endAngle={angles.end}
                    pieSort={null}
                >
                    {({ arcs, path }) => (
                        <>
                            <path
                                d={path(arcs[0]) || ""}
                                fill={getColor(arcs[0].value)}
                            />
                            <path
                                className={styles.arcFillBackground}
                                d={path(arcs[1]) || ""}
                            />
                        </>
                    )}
                </Shape.Pie>
                <Text
                    fill={getColor(normalizedValue)}
                    scaleToFit
                    width={(math.innerRadius - textMarginInline) * 2}
                    textAnchor="middle"
                    verticalAnchor="start"
                >
                    {percentCoveredNumberFormat.format(normalizedValue)}
                </Text>
            </Group>
        </svg>
    );
}

function CoveragePercentGauge(props: { stats: StatsT; className?: string }) {
    const value =
        Number(props.stats.totalCovered) / Number(props.stats.totalReachable);

    return (
        <ParentSize className={props.className}>
            {({ width }) => <Gauge width={width} value={value} />}
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
