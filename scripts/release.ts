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
import { $, chalk, usePowerShell } from "zx";

// We have to use PowerShell on Windows, otherwise `git status` reports a bunch of modifications
if (process.platform === "win32") {
    usePowerShell();
}

async function confirmOrExit(message: string) {
    const confirmed = await confirm({ message, default: false });
    if (!confirmed) {
        process.exit(1);
    }
}

function errorAndExit(message: string): never {
    console.error(`${chalk.red("✗")} ${message}`);
    process.exit(1);
}

async function ensureGitIsClean() {
    const status = await $`git status --porcelain`;
    const isClean = status.stdout.trim() === "";
    if (!isClean) {
        errorAndExit(
            "Git working directory is not clean. Please commit or stash changes before proceeding.",
        );
    }
}

await ensureGitIsClean();

const branch = (await $`git branch --show-current`).stdout.trim();
const isMainBranch = branch === "master" || branch === "main";
if (!isMainBranch) {
    errorAndExit(`Not on main branch: ${chalk.red(branch)}`);
}

////////////////////////////////////////////////////////////////////////////////
// Changelog
//

const changelogFile = "CHANGELOG.md";

async function getUnreleased(changelog: Changelog): Promise<ReleaseContent> {
    const unreleased = changelog.unreleased;

    if (!unreleased) {
        errorAndExit(
            `Missing ${chalk.red("Unreleased")} section in ${chalk.cyan(changelogFile)}`,
        );
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
        console.log(
            `${chalk.yellow("⚡")} This is the first release of the year ${year}!\n`,
        );
        const newVersion = new Version(shortYear, 0, 0);
        await confirmOrExit(
            `Continue releasing version ${chalk.cyan.bold(newVersion)}?`,
        );
        return newVersion;
    }
    const previousVersion = previousRelease.version;

    return await select({
        message: `What kind of release is this? ${chalk.dim(`(previous: ${previousVersion})`)}`,
        choices: [
            {
                value: previousVersion.nextMinor(),
                name: `Minor (${chalk.cyan(previousVersion.nextMinor())})`,
            },
            {
                value: previousVersion.nextMajor(),
                name: `Major (${chalk.cyan(previousVersion.nextMajor())})`,
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

console.log(
    `${chalk.blue("→")} Updating ${chalk.cyan(changelogFile)} with new version: ${chalk.cyan.bold(newVersion)}`,
);

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

// Commit the CHANGELOG.md update
await $`git add ${changelogFile}`;
await $`git commit -m ${`Release version ${newVersion}`}`;

// Tag the release commit
const gitTag = `v${newVersion}`;
await $`git tag -a ${gitTag} -m ${`Release ${newVersion}`}`;

// Push
await $`git push origin ${branch} ${gitTag}`;

console.log(
    `${chalk.green("✓")} Tagged ${chalk.cyan.bold(gitTag)}. A release will be created shortly.`,
);
