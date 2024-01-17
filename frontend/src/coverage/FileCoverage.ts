export enum LineCoverage {
    NotApplicable,
    Full,
    Partial,
    None,
}

export type FileCoverage = {
    filePath: string;
    lines: LineCoverage[];
};
