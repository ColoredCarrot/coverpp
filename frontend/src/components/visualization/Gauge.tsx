import { animated, to, useTransition } from "@react-spring/web";
import { Group } from "@visx/group";
import * as Scale from "@visx/scale";
import * as Shape from "@visx/shape";
import { PieArcDatum, ProvidedProps } from "@visx/shape/lib/shapes/Pie";
import { Text } from "@visx/text";
import styles from "./Gauge.module.css";

export const redYellowGreen = Scale.scaleQuantile({
    domain: [0, 1],
    range: ["#EA4228", "#F5CD19", "#5BE12C"],
});
export const neutralBlue = Scale.scaleLinear({
  domain: [0, 1],
  range: ["#DCEEFF", "#1976D2"],
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

    formatValue(v: number): string;
}

interface GaugeMath {
    boundingWidth: number;
    boundingHeight: number;
    innerRadius: number;
    outermostRadius: number;
    heightBelowCenterline: number;
    innerHeightBelowCenterline: number;
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
        innerHeightBelowCenterline:
            innerRadius * Math.sin(angles.end - Math.PI / 2),
    };
}

/**
 * The filled main arc of the gauge.
 *
 * Animated such that it starts out empty and expands left-to-right.
 */
function AnimatedMainArc(props: {
    path: ProvidedProps<number>["path"];
    arc: PieArcDatum<number>;
    color: string;
}) {
    type AnimatedProps = Pick<PieArcDatum<number>, "endAngle">;

    const transitions = useTransition<PieArcDatum<number>, AnimatedProps>(
        [props.arc],
        {
            // Animate from endAngle=startAngle to actual endAngle
            from: (datum): AnimatedProps => ({
                endAngle: datum.startAngle,
            }),
            enter: datum => ({ endAngle: datum.endAngle }),
        },
    );

    return transitions((animatedProps, arc) => {
        return (
            <animated.path
                // Make interpolated path d attribute from intermediate angle values
                d={to(animatedProps.endAngle, endAngle =>
                    props.path({
                        ...arc,
                        endAngle,
                    }),
                )}
                fill={props.color}
            />
        );
    });
}

const OUTER_ARC_SEGMENTS = 256;

const outerArcData = Array.from(
    { length: OUTER_ARC_SEGMENTS },
    (_, i) => (i + 0.5) / OUTER_ARC_SEGMENTS,
);

export default function Gauge({
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
    textMarginInline = 16,
    formatValue = v => `${v}`,
    value: normalizedValue, // value in [0, 1]
    colors,
}: {
    width: number;
    value: number;
    colors: typeof redYellowGreen | typeof neutralBlue;
} & Partial<GaugeProps>) {
    const math = calculateGauge({
        width,
        outerArcWidth,
        outerArcToInnerMargin,
        innerArcWidth,
        margins,
        textMarginInline,
        formatValue,
    });

    const innerArcOuterRadius = math.innerRadius + innerArcWidth;

    const valueColor = colors(normalizedValue);

    return (
        <svg width={width} height={math.boundingHeight}>
            <Group top={math.outermostRadius + margins.top} left={width / 2}>
                {/* Outer arc (labels) */}
                <Shape.Pie
                    data={outerArcData}
                    outerRadius={math.outermostRadius}
                    innerRadius={innerArcOuterRadius + outerArcToInnerMargin}
                    cornerRadius={0}
                    padAngle={0}
                    startAngle={angles.start}
                    endAngle={angles.end}
                    fill={datum => colors(datum.data)}
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
                    {({ arcs: [filledArc, unfilledArc], path }) => (
                        <>
                            {/* First, paint the arc that means "unfilled" */}
                            <path
                                className={styles.arcFillBackground}
                                d={
                                    path({
                                        // Modify the unfilled arc so that it starts where the filled arc starts
                                        ...unfilledArc,
                                        startAngle: filledArc.startAngle,
                                    }) ?? ""
                                }
                            />
                            {/* Second, atop the unfilled arc, paint the filled arc */}
                            <AnimatedMainArc
                                path={path}
                                arc={filledArc}
                                color={valueColor}
                            />
                        </>
                    )}
                </Shape.Pie>
            </Group>

            {/* Center text */}
            <Group
                top={
                    margins.top +
                    math.outermostRadius +
                    math.innerHeightBelowCenterline
                }
                left={width / 2}
            >
                <Text
                    className={styles.ValueText}
                    fill={valueColor}
                    scaleToFit
                    width={(math.innerRadius - textMarginInline) * 2}
                    textAnchor="middle"
                    verticalAnchor="end"
                >
                    {formatValue(normalizedValue)}
                </Text>
            </Group>
        </svg>
    );
}
