/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import { ESPLoader, Transport } from "esptool-js";
import { ImprovSerial } from "improv-wifi-serial-sdk/dist/serial.js";
import "bootstrap/dist/css/bootstrap.min.css";
import "./style.css";

const IMPROV_AUTHORIZED_STATE = 0x02;
const AUTHORIZATION_TIMEOUT_MS = 60000;
/** Firmware application baud rate used by Improv and MiniTerminal. */
const SERIAL_BAUD_RATE = 115200;
/** ESP32 ROM/stub baud rate used to accelerate firmware flashing. */
const FLASH_BAUD_RATE = 460800;
const MAX_TERMINAL_OUTPUT_LENGTH = 48000;
const WIFI_CREDENTIAL_COMMAND = /^write\s+wifi\s+(ssid|passphrase)\b/i;

const elements = {
    releaseVersion: document.getElementById("release-version"),
    boardCount: document.getElementById("board-count"),
    boardSelect: document.getElementById("board-select"),
    connectButton: document.getElementById("connect-button"),
    chipStatus: document.getElementById("chip-status"),
    bootloaderHelp: document.getElementById("bootloader-help"),
    flashButton: document.getElementById("flash-button"),
    progressWrap: document.getElementById("progress-wrap"),
    flashProgress: document.getElementById("flash-progress"),
    progressLabel: document.getElementById("progress-label"),
    wifiConnectButton: document.getElementById("wifi-connect-button"),
    authorizationStatus: document.getElementById("authorization-status"),
    wifiForm: document.getElementById("wifi-form"),
    ssidInput: document.getElementById("ssid-input"),
    passwordInput: document.getElementById("password-input"),
    provisionButton: document.getElementById("provision-button"),
    terminalConnectButton: document.getElementById("terminal-connect-button"),
    terminalClearButton: document.getElementById("terminal-clear-button"),
    terminalState: document.getElementById("terminal-state"),
    terminalOutput: document.getElementById("terminal-output"),
    terminalForm: document.getElementById("terminal-form"),
    terminalCommand: document.getElementById("terminal-command"),
    statusList: document.getElementById("status-list"),
};

let manifest;
let selectedTarget;
let serialPort;
let transport;
let loader;
let improv;
let devicePortReady = false;
let isFlashing = false;
let terminalReader;
let terminalReadTask;
let terminalDisconnectRequested = false;

/**
 * Add a timestamped message to the installer status list.
 * @param {string} message Status text
 * @param {string} [status="info"] Status style: info, success, or error
 * @returns {void}
 */
function addStatus(message, status = "info") {
    const item = document.createElement("li");
    item.className = `status-${status}`;
    item.textContent = `${new Date().toLocaleTimeString()}  ${message}`;
    elements.statusList.prepend(item);
}

/**
 * Append text to the bounded serial console buffer and scroll to its end.
 * @param {string} text Text received from the device or active protocol
 * @returns {void}
 */
function appendTerminalOutput(text) {
    elements.terminalOutput.textContent += text;
    if (MAX_TERMINAL_OUTPUT_LENGTH < elements.terminalOutput.textContent.length) {
        elements.terminalOutput.textContent = elements.terminalOutput.textContent.slice(-MAX_TERMINAL_OUTPUT_LENGTH);
    }
    elements.terminalOutput.scrollTop = elements.terminalOutput.scrollHeight;
}

/**
 * Show the correct manual download-mode sequence for a board.
 * @param {{board?: string}|null|undefined} target Selected firmware target
 * @returns {void}
 */
function updateBootloaderInstructions(target) {
    const boardId = target?.board?.toLowerCase() ?? "";

    if (boardId.includes("esp32doit-devkit-v1")) {
        elements.bootloaderHelp.textContent =
            "ESP32 DevKit V1: if sync times out, hold BOOT, tap EN/RESET, then release BOOT when the connection starts. Other boards usually enter download mode automatically.";
    } else {
        elements.bootloaderHelp.textContent =
            "Most boards enter download mode automatically. If sync fails, hold BOOT while tapping RESET/EN, then release it when connection starts.";
    }
}

/**
 * Synchronize controls with the current serial-port owner and device state.
 * @returns {void}
 */
function updateSerialControls() {
    const isConsoleConnected = Boolean(terminalReader);
    const isSessionOwned = Boolean(transport || improv);
    let consoleState = "DISCONNECTED";

    if (transport) {
        consoleState = "FLASH OUTPUT";
    } else if (improv) {
        consoleState = "IMPROV STATUS";
    } else if (isConsoleConnected) {
        consoleState = "CONNECTED";
    }

    elements.terminalState.textContent = consoleState;
    elements.terminalState.dataset.state = isConsoleConnected ? "connected" : "disconnected";
    elements.terminalConnectButton.textContent = isConsoleConnected ? "Close serial console" : "Open serial console";
    elements.terminalConnectButton.disabled = isSessionOwned || !("serial" in navigator);
    elements.terminalForm.hidden = !isConsoleConnected;
    elements.connectButton.disabled = !manifest || isSessionOwned || isFlashing;
    elements.flashButton.disabled = !loader || !transport || Boolean(improv || isFlashing);
    elements.wifiConnectButton.disabled = !devicePortReady || Boolean(improv || isFlashing);
}

/**
 * Reopen the previously selected serial port or request a device selection.
 * @returns {Promise<SerialPort>} An open Web Serial port at firmware baud rate
 * @throws {Error} If the user cancels selection or the port cannot be opened
 */
async function openSerialPort() {
    if (serialPort) {
        if ((null !== serialPort.readable) && (null !== serialPort.writable)) {
            return serialPort;
        }

        try {
            await serialPort.open({ baudRate: SERIAL_BAUD_RATE });
            return serialPort;
        } catch (error) {
            serialPort = undefined;
            addStatus(`USB serial port changed after restart. Select it again: ${error.message}`);
        }
    }

    serialPort = await navigator.serial.requestPort();
    await serialPort.open({ baudRate: SERIAL_BAUD_RATE });
    return serialPort;
}

/**
 * Update one setup step's visible state label.
 * @param {string} stepId Step DOM id prefix
 * @param {string} state Human-readable state
 * @returns {void}
 */
function setStepState(stepId, state) {
    const label = document.getElementById(`${stepId}-state`);
    label.textContent = state;
    label.dataset.state = state.toLowerCase();
}

/**
 * Compare a target ESP family with the detected ROM family.
 * @param {string} expectedChip Chip family required by the selected target
 * @param {string} detectedChip Chip family reported by esptool-js
 * @returns {boolean} Whether the detected chip matches the target
 */
function expectedChipMatches(expectedChip, detectedChip) {
    return expectedChip.replaceAll("-", "").toLowerCase() === detectedChip.replaceAll("-", "").toLowerCase();
}

/**
 * Find the selected manifest target for a board environment.
 * @param {string} environment PlatformIO environment identifier
 * @returns {object|undefined} Matching firmware target
 */
function findTarget(environment) {
    let selectedTarget;
    for (const target of manifest.targets) {
        if (target.environment === environment) {
            selectedTarget = target;
            break;
        }
    }
    return selectedTarget;
}

/**
 * Load firmware metadata and populate the board selector.
 * @returns {Promise<void>}
 * @throws {Error} If the release manifest cannot be loaded or parsed
 */
async function loadManifest() {
    const response = await fetch("./firmware-manifest.json", { cache: "no-store" });
    const contentType = response.headers.get("content-type") ?? "";
    if (!response.ok || !contentType.includes("application/json")) {
        throw new Error("No release firmware manifest is available in this preview.");
    }

    manifest = await response.json();
    elements.releaseVersion.textContent = manifest.version;
    elements.boardCount.textContent = `${manifest.targets.length} board builds`;
    elements.boardSelect.replaceChildren();

    for (const target of manifest.targets) {
        const option = document.createElement("option");
        option.value = target.environment;
        option.textContent = target.environment;
        elements.boardSelect.append(option);
    }

    elements.boardSelect.disabled = false;
    selectedTarget = manifest.targets[0];
    updateBootloaderInstructions(selectedTarget);
    elements.boardSelect.addEventListener("change", handleBoardSelection);

    updateSerialControls();
    addStatus(`${manifest.targets.length} board configurations are ready.`);
}

/**
 * Handle a board/display target selection change.
 * @param {Event} event Change event from the board selector
 * @returns {void}
 */
function handleBoardSelection(event) {
    const selector = /** @type {HTMLSelectElement} */ (event.currentTarget);
    selectedTarget = findTarget(selector.value);
    devicePortReady = false;
    setStepState("connect", "Waiting");
    setStepState("flash", "Waiting");
    elements.chipStatus.textContent = "Select the USB device to detect its chip.";
    updateBootloaderInstructions(selectedTarget);
    updateSerialControls();
    addStatus(`Selected ${selectedTarget.environment}.`);
}

/**
 * Select a serial device, detect its ESP chip, and enable compatible actions.
 * @returns {Promise<void>}
 * @throws {Error} If serial access, chip detection, or target matching fails
 */
async function connectToBoard() {
    if (terminalReader) {
        await disconnectTerminal();
    }
    if (improv) {
        await closeImprovSession();
    }

    devicePortReady = false;
    elements.connectButton.disabled = true;
    elements.chipStatus.textContent = "Choose the board's serial device in the browser dialog.";

    try {
        serialPort = serialPort ?? await navigator.serial.requestPort();
        transport = new Transport(serialPort, true);
        loader = new ESPLoader({
            transport,
            baudrate: FLASH_BAUD_RATE,
            terminal: {
                /** Keep existing console output while esptool starts. */
                clean() { },
                /**
                 * Forward a complete esptool log line to both status surfaces.
                 * @param {string} message Log line
                 * @returns {void}
                 */
                writeLine(message) {
                    addStatus(message);
                    appendTerminalOutput(`${message}\r\n`);
                },
                /**
                 * Forward a partial esptool log message to both status surfaces.
                 * @param {string} message Log fragment
                 * @returns {void}
                 */
                write(message) {
                    if (message.trim()) {
                        addStatus(message.trim());
                        appendTerminalOutput(message);
                    }
                },
            },
        });
        updateSerialControls();

        await loader.main();
        const detectedChip = loader.chip.CHIP_NAME;
        if (!expectedChipMatches(selectedTarget.chip, detectedChip)) {
            throw new Error(`Selected target needs ${selectedTarget.chip}; detected ${detectedChip}.`);
        }

        elements.chipStatus.textContent = `Detected ${detectedChip}.`;
        elements.flashButton.disabled = false;
        devicePortReady = true;
        setStepState("connect", "Connected");
        updateSerialControls();
        addStatus(`Detected ${detectedChip}; target matches.`, "success");
        appendTerminalOutput(`\r\n[Detected ${detectedChip}; board is ready.]\r\n`);
    } catch (error) {
        devicePortReady = false;
        elements.chipStatus.textContent = error.message;
        addStatus(error.message, "error");
        await disconnectFlasher();
        updateSerialControls();
    }
}

/**
 * Fetch a firmware image and verify its SHA-256 digest before flashing.
 * @param {{file: string, url: string, sha256: string, offset: number}} image Manifest image
 * @returns {Promise<{data: Uint8Array, address: number}>} Verified bytes and flash offset
 * @throws {Error} If fetching or checksum validation fails
 */
async function fetchImage(image) {
    const response = await fetch(`./${image.url}`, { cache: "force-cache" });
    if (!response.ok) {
        throw new Error(`Could not download ${image.file} (${response.status}).`);
    }

    const data = new Uint8Array(await response.arrayBuffer());
    const digest = await crypto.subtle.digest("SHA-256", data);
    const checksum = Array.from(new Uint8Array(digest), byteToHex).join("");

    if (checksum !== image.sha256) {
        throw new Error(`${image.file} failed its SHA-256 check.`);
    }

    return { data, address: image.offset };
}

/**
 * Format one checksum byte as a two-character lowercase hexadecimal string.
 * @param {number} byte Unsigned byte
 * @returns {string} Two-character hexadecimal byte
 */
function byteToHex(byte) {
    return byte.toString(16).padStart(2, "0");
}

/**
 * Sum uncompressed image sizes for aggregate progress calculation.
 * @param {Array<{data: Uint8Array}>} images Images being written
 * @returns {number} Total image bytes
 */
function getTotalImageBytes(images) {
    let totalBytes = 0;
    for (const image of images) {
        totalBytes += image.data.length;
    }
    return totalBytes;
}

/**
 * Update aggregate progress using the current image's compressed transfer ratio.
 * @param {number} fileIndex Index reported by esptool-js
 * @param {number} written Compressed bytes sent for this image
 * @param {number} compressedTotal Total compressed bytes for this image
 * @param {Array<{data: Uint8Array}>} files All images in flash order
 * @param {number} totalBytes Sum of original image sizes
 * @returns {void}
 */
function updateFlashProgress(fileIndex, written, compressedTotal, files, totalBytes) {
    const image = files[fileIndex];
    if (!image || (0 >= totalBytes)) {
        return;
    }

    const completedBeforeImage = getTotalImageBytes(files.slice(0, fileIndex));
    const imageFraction = 0 < compressedTotal
        ? Math.min(1, written / compressedTotal)
        : 0;
    const completedBytes = completedBeforeImage + (image.data.length * imageFraction);
    const percent = Math.min(100, Math.floor((completedBytes / totalBytes) * 100));
    elements.flashProgress.value = percent;
    elements.progressLabel.textContent = `${percent}%`;
}

/**
 * Flash the selected target's verified images to the detected ESP32.
 * @returns {Promise<void>}
 * @throws {Error} If an image cannot be fetched or the flash operation fails
 */
async function flashBoard() {
    if (!loader || !transport || isFlashing) {
        return;
    }

    isFlashing = true;
    elements.flashButton.disabled = true;
    elements.connectButton.disabled = true;
    elements.progressWrap.hidden = false;
    elements.flashProgress.value = 0;
    setStepState("flash", "Preparing");

    try {
        const files = await Promise.all(selectedTarget.images.map(fetchImage));
        const totalBytes = getTotalImageBytes(files);

        addStatus(`Verified all ${files.length} images. Erasing flash and installing ${selectedTarget.environment}.`);
        setStepState("flash", "Flashing");

        await loader.writeFlash({
            fileArray: files,
            flashMode: selectedTarget.flashMode,
            flashFreq: selectedTarget.flashFrequency,
            flashSize: selectedTarget.flashSize,
            eraseAll: selectedTarget.eraseAll,
            compress: true,
            /**
             * Translate esptool-js per-image compressed progress to total image progress.
             * @param {number} fileIndex Image index
             * @param {number} written Compressed bytes written for this image
             * @param {number} compressedTotal Compressed size of this image
             * @returns {void}
             */
            reportProgress(fileIndex, written, compressedTotal) {
                updateFlashProgress(fileIndex, written, compressedTotal, files, totalBytes);
            },
        });

        await loader.after("hard_reset");
        await disconnectFlasher();
        setStepState("flash", "Complete");
        devicePortReady = true;
        updateSerialControls();
        addStatus("Firmware installed. Reconnect to the device for Wi-Fi setup.", "success");
        appendTerminalOutput("\r\n[Firmware flashed. The device rebooted; USB remains connected.]\r\n");
    } catch (error) {
        devicePortReady = false;
        setStepState("flash", "Failed");
        elements.chipStatus.textContent = error.message;
        addStatus(error.message, "error");
        appendTerminalOutput(`\r\n[Flash failed: ${error.message}]\r\n`);
        await disconnectFlasher();
        updateSerialControls();
    } finally {
        isFlashing = false;
        updateSerialControls();
    }
}

/**
 * Close esptool-js transport and release its serial stream locks.
 * @returns {Promise<void>}
 */
async function disconnectFlasher() {
    if (transport) {
        try {
            await transport.disconnect();
        } catch {
            // The device may already have disconnected during reset.
        }
        transport = undefined;
        loader = undefined;
    }
    updateSerialControls();
}

/**
 * Open the standalone diagnostic console using the selected Web Serial device.
 * @returns {Promise<void>}
 */
async function connectTerminal() {
    if (transport || improv) {
        appendTerminalOutput("\r\n[Close the active flasher or Improv session before opening the terminal.]\r\n");
    } else {
        elements.terminalConnectButton.disabled = true;
        try {
            const port = await openSerialPort();
            const reader = port.readable.getReader();
            terminalReader = reader;
            devicePortReady = true;
            terminalDisconnectRequested = false;
            appendTerminalOutput("\r\n[Serial console connected at 115200 baud.]\r\n");
            updateSerialControls();
            terminalReadTask = readTerminal(reader);
        } catch (error) {
            serialPort = undefined;
            devicePortReady = false;
            elements.terminalConnectButton.disabled = false;
            updateSerialControls();
            appendTerminalOutput(`\r\n[Could not open serial console: ${error.message}]\r\n`);
        }
    }
}

/**
 * Read and display serial data until the port is disconnected or closed.
 * @param {ReadableStreamDefaultReader<Uint8Array>} reader Locked serial reader
 * @returns {Promise<void>}
 */
async function readTerminal(reader) {
    const decoder = new TextDecoder();

    try {
        while (true) {
            const { value, done } = await reader.read();
            if (done) {
                break;
            }
            if (value) {
                appendTerminalOutput(decoder.decode(value, { stream: true }));
            }
        }
        appendTerminalOutput(decoder.decode());
    } catch (error) {
        if (!terminalDisconnectRequested) {
            appendTerminalOutput(`\r\n[Serial connection ended: ${error.message}]\r\n`);
        }
    } finally {
        reader.releaseLock();
        if (terminalReader === reader) {
            terminalReader = undefined;
            terminalReadTask = undefined;
            updateSerialControls();
        }
    }
}

/**
 * Cancel the diagnostic reader and close its port without removing permission.
 * @returns {Promise<void>}
 */
async function disconnectTerminal() {
    const reader = terminalReader;
    terminalDisconnectRequested = true;

    if (reader) {
        await reader.cancel();
        if (terminalReadTask) {
            await terminalReadTask;
        }
    }

    if (serialPort && (null !== serialPort.readable) && (false === serialPort.readable.locked) &&
        (null !== serialPort.writable) && (false === serialPort.writable.locked)) {
        await serialPort.close();
    }

    terminalReader = undefined;
    terminalReadTask = undefined;
    terminalDisconnectRequested = false;
    appendTerminalOutput("\r\n[Serial console disconnected.]\r\n");
    updateSerialControls();
}

/**
 * Send one safe MiniTerminal diagnostic command.
 * @param {SubmitEvent} event Terminal form submission
 * @returns {Promise<void>}
 */
async function sendTerminalCommand(event) {
    event.preventDefault();
    const command = elements.terminalCommand.value.trim();

    if ("" === command) {
        return;
    }
    if (WIFI_CREDENTIAL_COMMAND.test(command)) {
        appendTerminalOutput("\r\n[Use Improv Wi-Fi setup for credentials; the firmware terminal echoes typed input.]\r\n");
        elements.terminalCommand.value = "";
        return;
    }
    if (!serialPort || !serialPort.writable || serialPort.writable.locked) {
        appendTerminalOutput("\r\n[Serial port is not writable.]\r\n");
    } else {
        const writer = serialPort.writable.getWriter();
        try {
            await writer.write(new TextEncoder().encode(`${command}\n`));
            elements.terminalCommand.value = "";
        } catch (error) {
            appendTerminalOutput(`\r\n[Command could not be sent: ${error.message}]\r\n`);
        } finally {
            writer.releaseLock();
        }
    }
}

/**
 * Wait for the user to authorize Improv provisioning using a device button.
 * @param {ImprovSerial} service Active Improv Serial service
 * @returns {Promise<void>} Resolves when the service enters its authorized state
 * @throws {Error} If physical authorization does not arrive before the timeout
 */
function waitForAuthorization(service) {
    if (service.state === IMPROV_AUTHORIZED_STATE) {
        return Promise.resolve();
    }

    /**
     * Register the authorization listener and its timeout.
     * @param {(value?: void | PromiseLike<void>) => void} resolve Promise resolve callback
     * @param {(reason?: Error) => void} reject Promise reject callback
     * @returns {void}
     */
    function registerAuthorizationListener(resolve, reject) {
        /** Reject authorization when the physical-button window expires. */
        function handleAuthorizationTimeout() {
            service.removeEventListener("state-changed", onStateChanged);
            reject(new Error("Authorization timed out. Press a physical button and try again."));
        }

        const timeout = window.setTimeout(handleAuthorizationTimeout, AUTHORIZATION_TIMEOUT_MS);

        /**
         * Resolve when Improv reports physical authorization.
         * @param {CustomEvent<number>} event Improv state-change event
         * @returns {void}
         */
        function onStateChanged(event) {
            if (event.detail === IMPROV_AUTHORIZED_STATE) {
                window.clearTimeout(timeout);
                service.removeEventListener("state-changed", onStateChanged);
                resolve();
            }
        }

        service.addEventListener("state-changed", onStateChanged);
    }

    return new Promise(registerAuthorizationListener);
}

/**
 * Open Improv on the selected port, returning a running board to its application first.
 * @returns {Promise<void>}
 */
async function connectForWifiSetup() {
    if (isFlashing) {
        return;
    }

    elements.wifiConnectButton.disabled = true;
    elements.authorizationStatus.textContent = "Connecting to Improv Serial…";

    try {
        if (terminalReader) {
            await disconnectTerminal();
        }
        if (transport) {
            await loader.after("hard_reset");
            await disconnectFlasher();
        }
        const port = await openSerialPort();
        const quietLogger = {
            /** Suppress protocol logs from the status area. */
            log() { },
            /** Suppress protocol debug output. */
            debug() { },
            /** Suppress protocol error output; the UI handles state events. */
            error() { },
        };
        improv = new ImprovSerial(port, quietLogger);
        updateSerialControls();

        appendTerminalOutput("\r\n[Improv Wi-Fi session started. Waiting for button authorization.]\r\n");

        improv.addEventListener("state-changed", handleImprovStateChanged);
        improv.addEventListener("error-changed", handleImprovErrorChanged);

        const deviceInfo = await improv.initialize(5000);
        elements.wifiForm.hidden = false;
        elements.authorizationStatus.textContent = `Press any physical button on the device to authorize Wi-Fi setup. ${deviceInfo?.name ?? "Pixelix"} is waiting.`;
        appendTerminalOutput(`\r\n[${deviceInfo?.name ?? "Pixelix"} is waiting for a physical button.]\r\n`);
        setStepState("wifi", "Waiting for button");

        await waitForAuthorization(improv);
        elements.authorizationStatus.textContent = "Authorized. Enter the Wi-Fi credentials.";
        elements.provisionButton.disabled = false;
    } catch (error) {
        elements.authorizationStatus.textContent = error.message;
        addStatus(error.message, "error");
        appendTerminalOutput(`\r\n[Wi-Fi setup failed: ${error.message}]\r\n`);
        elements.wifiConnectButton.disabled = false;
        if (improv) {
            await closeImprovSession();
        } else if (serialPort && serialPort.readable && !serialPort.readable.locked) {
            await serialPort.close();
        }
    }
}

/**
 * Update installer and console output when Improv state changes.
 * @param {CustomEvent<number>} event Improv state-change event
 * @returns {void}
 */
function handleImprovStateChanged(event) {
    appendTerminalOutput(`\r\n[Improv state: ${event.detail}]\r\n`);
    if (event.detail === IMPROV_AUTHORIZED_STATE) {
        elements.authorizationStatus.textContent = "Authorized. Enter the Wi-Fi credentials.";
        elements.provisionButton.disabled = false;
        setStepState("wifi", "Authorized");
    }
}

/**
 * Forward an Improv error code to the serial console without exposing credentials.
 * @param {CustomEvent<number>} event Improv error-change event
 * @returns {void}
 */
function handleImprovErrorChanged(event) {
    if (0 !== event.detail) {
        appendTerminalOutput(`\r\n[Improv error: ${event.detail}]\r\n`);
    }
}

/**
 * Close the Improv reader and release the selected serial port.
 * @returns {Promise<void>}
 */
async function closeImprovSession() {
    if (improv) {
        const session = improv;
        improv = undefined;
        await session.close();
    }

    if (serialPort && (null !== serialPort.readable) && (false === serialPort.readable.locked) &&
        (null !== serialPort.writable) && (false === serialPort.writable.locked)) {
        await serialPort.close();
    }

    updateSerialControls();
}

/**
 * Submit Wi-Fi credentials to Improv after physical authorization.
 * @param {SubmitEvent} event Wi-Fi form submission
 * @returns {Promise<void>}
 */
async function provisionWifi(event) {
    event.preventDefault();
    const ssid = elements.ssidInput.value;
    const password = elements.passwordInput.value;
    const encoder = new TextEncoder();

    if ((0 === encoder.encode(ssid).length) || (32 < encoder.encode(ssid).length) || (64 < encoder.encode(password).length)) {
        elements.authorizationStatus.textContent = "SSID must be 1–32 UTF-8 bytes and the password at most 64 bytes.";
        return;
    }

    elements.provisionButton.disabled = true;
    setStepState("wifi", "Saving");
    elements.authorizationStatus.textContent = "Sending credentials directly to the device over USB…";

    try {
        await improv.provision(ssid, password, 30000);
        elements.passwordInput.value = "";
        elements.authorizationStatus.textContent = "Credentials saved. Pixelix is restarting; it will join the selected Wi-Fi network.";
        setStepState("wifi", "Restarting");
        addStatus("Wi-Fi credentials accepted; the device is restarting.", "success");
        appendTerminalOutput("\r\n[Wi-Fi credentials accepted. Device restarting.]\r\n");
        await closeImprovSession();
        updateSerialControls();
    } catch (error) {
        elements.passwordInput.value = "";
        elements.authorizationStatus.textContent = "Wi-Fi setup failed. Check the credentials or use the captive portal.";
        elements.provisionButton.disabled = false;
        addStatus(`Wi-Fi provisioning failed: ${error.message}`, "error");
        appendTerminalOutput(`\r\n[Wi-Fi provisioning failed: ${error.message}]\r\n`);
    }
}

/**
 * Show the unsupported-browser message when Web Serial is unavailable.
 * @returns {void}
 */
function handleUnsupportedSerial() {
    elements.chipStatus.textContent = "Web Serial requires a secure context in Chrome or Edge on desktop.";
    elements.connectButton.disabled = true;
    elements.wifiConnectButton.disabled = true;
    elements.terminalConnectButton.disabled = true;
}

/**
 * Display manifest loading errors in the installer and activity list.
 * @param {Error} error Manifest loading failure
 * @returns {void}
 */
function handleManifestError(error) {
    elements.chipStatus.textContent = error.message;
    addStatus(error.message, "error");
}

/**
 * Toggle standalone terminal ownership of the serial port.
 * @returns {void}
 */
function toggleTerminalConnection() {
    if (terminalReader) {
        disconnectTerminal().catch(handleTerminalError);
    } else {
        connectTerminal();
    }
}

/**
 * Report a terminal connection/close failure in installer status.
 * @param {Error} error Terminal failure
 * @returns {void}
 */
function handleTerminalError(error) {
    addStatus(error.message, "error");
}

/**
 * Clear the serial console output buffer.
 * @returns {void}
 */
function clearTerminalOutput() {
    elements.terminalOutput.textContent = "";
}

/**
 * Clear the installer status activity list.
 * @returns {void}
 */
function clearInstallerStatus() {
    elements.statusList.replaceChildren();
}

if (!("serial" in navigator) || !window.isSecureContext) {
    handleUnsupportedSerial();
} else {
    updateSerialControls();
    loadManifest().catch(handleManifestError);
}

elements.connectButton.addEventListener("click", connectToBoard);
elements.flashButton.addEventListener("click", flashBoard);
elements.wifiConnectButton.addEventListener("click", connectForWifiSetup);
elements.wifiForm.addEventListener("submit", provisionWifi);
elements.terminalConnectButton.addEventListener("click", toggleTerminalConnection);
elements.terminalClearButton.addEventListener("click", clearTerminalOutput);
elements.terminalForm.addEventListener("submit", sendTerminalCommand);
document.getElementById("clear-status").addEventListener("click", clearInstallerStatus);