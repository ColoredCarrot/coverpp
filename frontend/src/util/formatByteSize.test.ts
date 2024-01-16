import { expect, test } from "vitest";
import formatByteSize from "#/util/formatByteSize";

type TestCase = Readonly<{
    bytes: number;
    si: string;
    binary: string;
}>;

const testCases: readonly TestCase[] = [
    { bytes: 0, si: "0 B", binary: "0 B" },
    { bytes: -0, si: "0 B", binary: "0 B" },

    { bytes: 1, si: "1 B", binary: "1 B" },
    { bytes: 1000, si: "1.00 kB", binary: "1000 B" },
    { bytes: 1024, si: "1.02 kB", binary: "1.00 KiB" },

    { bytes: -1, si: "-1 B", binary: "-1 B" },

    { bytes: NaN, si: "NaN", binary: "NaN" },
    { bytes: Infinity, si: "Infinite", binary: "Infinite" },
    { bytes: -Infinity, si: "-Infinite", binary: "-Infinite" },
];

test.each(testCases)("Binary formatByteSize($bytes) -> $binary", testCase => {
    expect(formatByteSize(testCase.bytes, { si: false })).toBe(testCase.binary);
});

test.each(testCases)("SI formatByteSize($bytes) -> $si", testCase => {
    expect(formatByteSize(testCase.bytes, { si: true })).toBe(testCase.si);
});
