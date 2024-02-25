import styles from "./Gauge.module.css";
import * as Scale from "@visx/scale";
import { Group } from "@visx/group";
import * as Shape from "@visx/shape";
import { Text } from "@visx/text";
import {
    animated,
    useTransition,
    to,
} from "@react-spring/web";
import { PieArcDatum, ProvidedProps } from "@visx/shape/lib/shapes/Pie";

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

type AnimatedStyles = { startAngle: number; endAngle: number };

function AnimatedArc<Datum>(
    props: ProvidedProps<Datum> & {
        getKey: (d: PieArcDatum<Datum>) => string;
        getColor: (d: PieArcDatum<Datum>) => string;
    },
) {
    const transitions = useTransition<PieArcDatum<Datum>, AnimatedStyles>(
        props.arcs,
        {
            from: (datum): AnimatedStyles => ({
                startAngle: datum.startAngle,
                endAngle: datum.startAngle,
            }),
            enter: datum => ({ endAngle: datum.endAngle }),
            keys: props.getKey,
        },
    );

    const { path, getColor } = props;

    return transitions((props, arc, { key }) => {
        return (
            <g key={key}>
                <animated.path
                    // compute interpolated path d attribute from intermediate angle values
                    d={to(
                        [props.startAngle, props.endAngle],
                        (startAngle, endAngle) =>
                            path({
                                ...arc,
                                startAngle,
                                endAngle,
                            }),
                    )}
                    fill={getColor(arc)}
                />
            </g>
        );
    });
}

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
        formatValue,
    });

    const innerArcOuterRadius = math.innerRadius + innerArcWidth;

    const valueColor = getColor(normalizedValue);

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
                    {({ arcs, path, pie }) => (
                        <AnimatedArc
                            path={path}
                            arcs={arcs}
                            pie={pie}
                            getColor={s => getColor(s.data)}
                            getKey={s => s.index + ""}
                        />
                        // <>
                        //     <path d={path(arcs[0]) ?? ""} fill={valueColor} />
                        //     <path
                        //         className={styles.arcFillBackground}
                        //         d={path(arcs[1]) ?? ""}
                        //     />
                        // </>
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
