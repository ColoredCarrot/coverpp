import fs from "node:fs";
import { Version } from "./util/version.ts";
import { parseChangelog } from "./util/changelog.ts";
import { argv } from "zx";
import "temporal-polyfill/global";

const changelogFile = "CHANGELOG.md";

const versionString = argv._[0];
if (!versionString) {
    console.error("Usage: get-release-notes <version>");
    process.exit(1);
}

const version = Version.parseOrThrow(versionString);

const changelog = parseChangelog(fs.readFileSync(changelogFile, "utf-8"));

const release = changelog.releases.find(release =>
    release.version.equals(version),
);
if (!release) {
    console.error(`Release ${version} not found in ${changelogFile}`);
    process.exit(1);
}

console.log(release.content.markdown);
