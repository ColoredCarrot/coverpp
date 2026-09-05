import * as flatbuffers from "flatbuffers";
import * as Report from "#/_generated/coverpp/report";
import { CoverageReportT } from "#/_generated/coverpp/report";

const MAGIC = new Uint8Array([0x43, 0x76, 0x50, 0x50, 0xc0, 0x99]); // "CvPP\xC0\x99"
const FORMAT_VERSION_BYTES = 4;

export function parseCoverage(raw: Uint8Array): CoverageReportT {
    if (raw.length < MAGIC.length + FORMAT_VERSION_BYTES) {
        throw new Error("Malformed coverage report");
    }

    // Magic
    if (!MAGIC.every((byte, i) => raw[i] === byte)) {
        throw new Error("Malformed coverage report");
    }

    // Format version
    const formatVersion = new DataView(
        raw.buffer,
        raw.byteOffset,
        raw.byteLength,
    ).getInt32(MAGIC.length, true);
    if (formatVersion !== 1) {
        throw new Error(
            `Unsupported coverage report format version: ${formatVersion}`,
        );
    }

    // Payload
    const payload = raw.subarray(MAGIC.length + FORMAT_VERSION_BYTES);

    const buffer = new flatbuffers.ByteBuffer(payload);
    const coverageReport =
        Report.CoverageReport.getRootAsCoverageReport(buffer).unpack();

    // Heuristic: Flatbuffers doesn't reject malformed input, but just returns a default-constructed object
    if (raw.length > 100 && !coverageReport.roots?.length) {
        throw new Error("Malformed coverage report");
    }

    return coverageReport;
}
