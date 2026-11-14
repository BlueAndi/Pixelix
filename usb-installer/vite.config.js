import { execSync } from "node:child_process";
import { defineConfig } from "vite";

/**
 * Run a git command in the repository and return its trimmed output.
 * @param {string} command Git arguments
 * @returns {string} Command output, or an empty string if git is not available
 */
function git(command) {
    try {
        return execSync(`git ${command}`, { encoding: "utf8", stdio: ["ignore", "pipe", "ignore"] }).trim();
    } catch {
        return "";
    }
}

/** Build information shown in the page footer to recognize a stale copy. */
const installerBuild = {
    commit: git("rev-parse --short HEAD"),
    dirty: "" !== git("status --porcelain -- ."),
    builtAt: new Date().toISOString(),
};

export default defineConfig({
    base: "./",
    define: {
        __INSTALLER_BUILD__: JSON.stringify(installerBuild),
    },
});
