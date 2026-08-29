import fs from "node:fs";
import { confirm, select } from "@inquirer/prompts";
import { Version } from "./util/version.ts";
import {
    type Changelog,
    parseChangelog,
    printChangelog,
    type ReleaseContent,
} from "./util/changelog.ts";
import "temporal-polyfill/global";

async function confirmOrExit(message: string) {
    const confirmed = await confirm({ message, default: false });
    if (!confirmed) {
        process.exit(1);
    }
}

async function getUnreleased(changelog: Changelog): Promise<ReleaseContent> {
    const unreleased = changelog.unreleased;

    if (!unreleased) {
        console.error("Missing Unreleased section in CHANGELOG.md");
        process.exit(1);
    }

    if (!unreleased.markdown) {
        await confirmOrExit("No unreleased changes. Continue?");
    }

    return unreleased;
}

async function getNewVersion(changelog: Changelog): Promise<Version> {
    const year = new Date().getFullYear();
    const shortYear = year - 2000;

    const previousRelease = changelog.releases[0];
    if (!previousRelease || previousRelease.version.shortYear !== shortYear) {
        console.log(`This is the first release of the year ${year}!`);
        return new Version(shortYear, 0, 0);
    }
    const previousVersion = previousRelease.version;
    console.log(`Previous release: ${previousVersion}`);

    return await select({
        message: "What kind of release is this?",
        choices: [
            {
                value: previousVersion.nextMinor(),
                name: `Minor (${previousVersion.nextMinor()})`,
            },
            {
                value: previousVersion.nextMajor(),
                name: `Major (${previousVersion.nextMajor()})`,
            },
        ],
    });
}

function readChangelog() {
    const markdown = fs.readFileSync("CHANGELOG.md", "utf-8");
    const changelog = parseChangelog(markdown);
    return {
        changelog,
        newline: markdown.includes("\r\n") ? "\r\n" : "\n",
    };
}

const { changelog, newline } = readChangelog();

const unreleased = await getUnreleased(changelog);

const newVersion = await getNewVersion(changelog);

console.log(`Updating CHANGELOG.md with new version: ${newVersion}`);

changelog.releases.unshift({
    version: newVersion,
    date: Temporal.Now.plainDateISO(),
    content: unreleased,
});
changelog.unreleased = { markdown: "" };

fs.writeFileSync(
    "CHANGELOG.md",
    printChangelog(changelog, { newline }),
    "utf-8",
);
