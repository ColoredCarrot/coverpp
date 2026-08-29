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
import { $ } from "zx";

async function confirmOrExit(message: string) {
    const confirmed = await confirm({ message, default: false });
    if (!confirmed) {
        process.exit(1);
    }
}

async function ensureGitIsClean() {
    const status = await $`git status --porcelain`;
    const isClean = status.stdout.trim() === "";
    if (!isClean) {
        console.error(
            "Git working directory is not clean. Please commit or stash changes before proceeding.",
        );
        process.exit(1);
    }
}

await ensureGitIsClean();

////////////////////////////////////////////////////////////////////////////////
// Changelog
//

const changelogFile = "CHANGELOG.md";

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
        const newVersion = new Version(shortYear, 0, 0);
        await confirmOrExit(`Continue releasing version ${newVersion}?`);
        return newVersion;
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
    const markdown = fs.readFileSync(changelogFile, "utf-8");
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
    changelogFile,
    printChangelog(changelog, { newline }),
    "utf-8",
);

////////////////////////////////////////////////////////////////////////////////
// Git tag
//

const branch = (await $`git branch --show-current`).stdout.trim();
const isMainBranch = branch === "master" || branch === "main";
if (!isMainBranch) {
    console.error("Not on main branch. Aborting.");
    process.exit(1);
}

// Commit the CHANGELOG.md update
await $`git add ${changelogFile}`;
await $`git commit -m ${`Release version ${newVersion}`}`;

// Tag the release commit
const gitTag = `v${newVersion}`;

await $`git tag -a ${gitTag} -m ${`Release ${newVersion}`}`;
await $`git push origin ${gitTag}`;

console.log(`Tagged ${gitTag}. A release will be created shortly.`);
