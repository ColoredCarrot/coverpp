const regex = /^(?<shortYear>\d{1,3})(?:\.(?<major>\d+)(?:\.(?<minor>\d+))?)?$/;

export class Version {
    readonly shortYear: number;
    readonly major: number;
    readonly minor: number;

    constructor(shortYear: number, major: number, minor: number) {
        if (
            !Number.isSafeInteger(shortYear) ||
            shortYear < 0 ||
            shortYear > 999
        ) {
            throw new Error(`Invalid short year: ${shortYear}`);
        }
        if (!Number.isSafeInteger(major) || major < 0) {
            throw new Error(`Invalid major version: ${major}`);
        }
        if (!Number.isSafeInteger(minor) || minor < 0) {
            throw new Error(`Invalid minor version: ${minor}`);
        }

        this.shortYear = shortYear;
        this.major = major;
        this.minor = minor;
    }

    static parseOrThrow(s: string): Version {
        const match = regex.exec(s);
        if (!match) {
            throw new Error(`Invalid version: ${s}`);
        }
        const { shortYear, major, minor } = match.groups!;
        return new Version(
            parseInt(shortYear!),
            major ? parseInt(major) : 0,
            minor ? parseInt(minor) : 0,
        );
    }

    nextMajor(): Version {
        return new Version(this.shortYear, this.major + 1, 0);
    }

    nextMinor(): Version {
        return new Version(this.shortYear, this.major, this.minor + 1);
    }

    toString(): string {
        if (this.minor) {
            return `${this.shortYear}.${this.major}.${this.minor}`;
        }
        if (this.major) {
            return `${this.shortYear}.${this.major}`;
        }
        return `${this.shortYear}`;
    }

    compare(that: Version): -1 | 0 | 1 {
        return Math.sign(
            this.shortYear - that.shortYear ||
                this.major - that.major ||
                this.minor - that.minor,
        ) as -1 | 0 | 1;
    }

    equals(that: Version): boolean {
        return (
            this.shortYear === that.shortYear &&
            this.major === that.major &&
            this.minor === that.minor
        );
    }
}
