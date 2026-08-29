import { Version } from "./version.ts";

export type ReleaseContent = {
    markdown: string;
};

export type Release = {
    version: Version;
    date: Temporal.PlainDate;
    content: ReleaseContent;
};

export type Changelog = {
    preamble: string | undefined;
    unreleased: ReleaseContent | undefined;
    releases: Release[];
};

type Section = {
    heading: string | undefined;
    body: string[];
};

export function parseChangelog(markdown: string): Changelog {
    const lines = markdown.split(/\r?\n/);

    const sections: Section[] = [];

    let current: Section = {
        heading: undefined,
        body: [],
    };

    sections.push(current);

    for (const line of lines) {
        if (line.startsWith("## ")) {
            current = {
                heading: line.slice(3),
                body: [],
            };
            sections.push(current);
        } else {
            current.body.push(line);
        }
    }

    function bodyMarkdown(lines: string[]): string {
        return trimBlankLines(lines).join("\n");
    }

    const preambleSection = sections[0]!;
    const preamble = bodyMarkdown(preambleSection.body);

    let unreleased: ReleaseContent | undefined;
    const releases: Release[] = [];

    for (const section of sections.slice(1)) {
        const heading = section.heading!;

        if (heading === "[Unreleased]") {
            unreleased = {
                markdown: bodyMarkdown(section.body),
            };
            continue;
        }

        const match = heading.match(/^\[(.+?)] - (\d{4}-\d{2}-\d{2})$/);
        if (!match) {
            throw new Error(`Invalid heading: ${heading}`);
        }

        const [, version, date] = match;

        releases.push({
            version: Version.parseOrThrow(version!),
            date: Temporal.PlainDate.from(date!),
            content: {
                markdown: bodyMarkdown(section.body),
            },
        });
    }

    return {
        preamble: preamble === "" ? undefined : preamble,
        unreleased,
        releases,
    };
}

export function printChangelog(
    changelog: Changelog,
    { newline = "\n" }: { newline?: string } = {},
): string {
    let result = "";
    if (changelog.preamble) {
        result += changelog.preamble;
    }

    if (changelog.unreleased) {
        result += `${newline}${newline}## [Unreleased]`;
        if (changelog.unreleased.markdown) {
            result += newline + newline + changelog.unreleased.markdown;
        }
    }

    for (const release of changelog.releases) {
        result += `${newline}${newline}## [${release.version}] - ${release.date}`;
        if (release.content.markdown) {
            result += newline + newline + release.content.markdown;
        }
    }

    result += newline;
    return result;
}

function trimBlankLines(lines: string[]): string[] {
    let start = 0;
    let end = lines.length;

    while (start < end && lines[start]!.trim() === "") {
        start++;
    }

    while (end > start && lines[end - 1]!.trim() === "") {
        end--;
    }

    return lines.slice(start, end);
}
