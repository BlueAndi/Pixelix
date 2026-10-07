import { ESPLoader, Transport } from "esptool-js";
import { ImprovSerial } from "improv-wifi-serial-sdk/dist/serial.js";
import "bootstrap/dist/css/bootstrap.min.css";
import "./style.css";

const IMPROV_AUTHORIZED_STATE = 0x02;
const AUTHORIZATION_TIMEOUT_MS = 60000;
const SERIAL_BAUD_RATE = 115200;
const MAX_TERMINAL_OUTPUT_LENGTH = 48000;
const WIFI_CREDENTIAL_COMMAND = /^write\s+wifi\s+(ssid|passphrase)\b/i;

const elements = {
    releaseVersion: document.getElementById("release-version"),
    boardCount: document.getElementById("board-count"),
    boardSelect: document.getElementById("board-select"),
    connectButton: document.getElementById("connect-button"),
    chipStatus: document.getElementById("chip-status"),
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
let terminalReader;
let terminalReadTask;
let terminalDisconnectRequested = false;

function addStatus(message, status = "info") {
    const item = document.createElement("li");
    item.className = `status-${status}`;
    item.textContent = `${new Date().toLocaleTimeString()}  ${message}`;
    elements.statusList.prepend(item);
}

function appendTerminalOutput(text) {
    elements.terminalOutput.textContent += text;
    if (MAX_TERMINAL_OUTPUT_LENGTH < elements.terminalOutput.textContent.length) {
        elements.terminalOutput.textContent = elements.terminalOutput.textContent.slice(-MAX_TERMINAL_OUTPUT_LENGTH);
    }
    elements.terminalOutput.scrollTop = elements.terminalOutput.scrollHeight;
}

function setTerminalConnected(isConnected) {
    elements.terminalState.textContent = isConnected ? "CONNECTED" : "DISCONNECTED";
    elements.terminalState.dataset.state = isConnected ? "connected" : "disconnected";
    elements.terminalConnectButton.textContent = isConnected ? "Close serial console" : "Open serial console";
    elements.terminalConnectButton.disabled = !isConnected && Boolean(transport || improv);
    elements.terminalForm.hidden = !isConnected;
    elements.connectButton.disabled = isConnected || !manifest || Boolean(transport);
    elements.flashButton.disabled = isConnected || !loader || !transport;
    elements.wifiConnectButton.disabled = isConnected || Boolean(transport) || !("serial" in navigator);
}

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

function setStepState(stepId, state) {
    const label = document.getElementById(`${stepId}-state`);
    label.textContent = state;
    label.dataset.state = state.toLowerCase();
}

function expectedChipMatches(expectedChip, detectedChip) {
    return expectedChip.replaceAll("-", "").toLowerCase() === detectedChip.replaceAll("-", "").toLowerCase();
}

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
    elements.boardSelect.addEventListener("change", () => {
        selectedTarget = manifest.targets.find((target) => target.environment === elements.boardSelect.value);
        setStepState("connect", "Waiting");
        setStepState("flash", "Waiting");
        elements.flashButton.disabled = true;
        elements.connectButton.disabled = false;
        elements.chipStatus.textContent = "Select the USB device to detect its chip.";
        addStatus(`Selected ${selectedTarget.environment}.`);
    });

    elements.connectButton.disabled = false;
    elements.wifiConnectButton.disabled = false;
    addStatus(`${manifest.targets.length} board configurations are ready.`);
}

async function connectToBoard() {
    if (terminalReader) {
        await disconnectTerminal();
    }
    if (improv) {
        await closeImprovSession();
    }

    elements.connectButton.disabled = true;
    elements.chipStatus.textContent = "Choose the board's serial device in the browser dialog.";

    try {
        serialPort = await navigator.serial.requestPort();
        transport = new Transport(serialPort, true);
        loader = new ESPLoader({
            transport,
            baudrate: 115200,
            terminal: {
                clean() { },
                writeLine(message) {
                    addStatus(message);
                },
                write(message) {
                    if (message.trim()) {
                        addStatus(message.trim());
                    }
                },
            },
        });
        setTerminalConnected(false);

        await loader.main();
        const detectedChip = loader.chip.CHIP_NAME;
        if (!expectedChipMatches(selectedTarget.chip, detectedChip)) {
            throw new Error(`Selected target needs ${selectedTarget.chip}; detected ${detectedChip}.`);
        }

        elements.chipStatus.textContent = `Detected ${detectedChip}.`;
        elements.flashButton.disabled = false;
        setStepState("connect", "Connected");
        addStatus(`Detected ${detectedChip}; target matches.`, "success");
    } catch (error) {
        elements.chipStatus.textContent = error.message;
        addStatus(error.message, "error");
        await disconnectFlasher();
        setTerminalConnected(false);
    }
}

async function fetchImage(image) {
    const response = await fetch(`./${image.url}`, { cache: "force-cache" });
    if (!response.ok) {
        throw new Error(`Could not download ${image.file} (${response.status}).`);
    }

    const data = new Uint8Array(await response.arrayBuffer());
    const digest = await crypto.subtle.digest("SHA-256", data);
    const checksum = Array.from(new Uint8Array(digest), (byte) => byte.toString(16).padStart(2, "0")).join("");

    if (checksum !== image.sha256) {
        throw new Error(`${image.file} failed its SHA-256 check.`);
    }

    return { data, address: image.offset };
}

async function flashBoard() {
    elements.flashButton.disabled = true;
    elements.connectButton.disabled = true;
    elements.progressWrap.hidden = false;
    elements.flashProgress.value = 0;
    setStepState("flash", "Preparing");

    try {
        const files = await Promise.all(selectedTarget.images.map(fetchImage));
        const totalBytes = files.reduce((total, file) => total + file.data.length, 0);

        addStatus(`Verified all ${files.length} images. Erasing flash and installing ${selectedTarget.environment}.`);
        setStepState("flash", "Flashing");

        await loader.writeFlash({
            fileArray: files,
            flashMode: selectedTarget.flashMode,
            flashFreq: selectedTarget.flashFrequency,
            flashSize: selectedTarget.flashSize,
            eraseAll: selectedTarget.eraseAll,
            compress: true,
            reportProgress(fileIndex, written) {
                const completedBytes = files.slice(0, fileIndex).reduce((total, file) => total + file.data.length, 0) + written;
                const percent = Math.min(100, Math.floor((completedBytes / totalBytes) * 100));
                elements.flashProgress.value = percent;
                elements.progressLabel.textContent = `${percent}%`;
            },
        });

        await loader.after("hard_reset");
        await disconnectFlasher();
        setStepState("flash", "Complete");
        elements.wifiConnectButton.disabled = false;
        addStatus("Firmware installed. Reconnect to the device for Wi-Fi setup.", "success");
    } catch (error) {
        setStepState("flash", "Failed");
        elements.chipStatus.textContent = error.message;
        addStatus(error.message, "error");
        await disconnectFlasher();
        elements.connectButton.disabled = false;
    }
}

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
    setTerminalConnected(Boolean(terminalReader));
}

async function connectTerminal() {
    if (transport || improv) {
        appendTerminalOutput("\r\n[Close the active flasher or Improv session before opening the terminal.]\r\n");
    } else {
        elements.terminalConnectButton.disabled = true;
        try {
            const port = await openSerialPort();
            const reader = port.readable.getReader();
            terminalReader = reader;
            terminalDisconnectRequested = false;
            appendTerminalOutput("\r\n[Serial console connected at 115200 baud.]\r\n");
            setTerminalConnected(true);
            terminalReadTask = readTerminal(reader);
        } catch (error) {
            serialPort = undefined;
            elements.terminalConnectButton.disabled = false;
            setTerminalConnected(false);
            appendTerminalOutput(`\r\n[Could not open serial console: ${error.message}]\r\n`);
        }
    }
}

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
            setTerminalConnected(false);
        }
    }
}

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
    setTerminalConnected(false);
}

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

function waitForAuthorization(service) {
    if (service.state === IMPROV_AUTHORIZED_STATE) {
        return Promise.resolve();
    }

    return new Promise((resolve, reject) => {
        const timeout = window.setTimeout(() => {
            service.removeEventListener("state-changed", onStateChanged);
            reject(new Error("Authorization timed out. Press a physical button and try again."));
        }, AUTHORIZATION_TIMEOUT_MS);

        function onStateChanged(event) {
            if (event.detail === IMPROV_AUTHORIZED_STATE) {
                window.clearTimeout(timeout);
                service.removeEventListener("state-changed", onStateChanged);
                resolve();
            }
        }

        service.addEventListener("state-changed", onStateChanged);
    });
}

async function connectForWifiSetup() {
    elements.wifiConnectButton.disabled = true;
    elements.authorizationStatus.textContent = "Connecting to Improv Serial…";

    try {
        if (terminalReader) {
            await disconnectTerminal();
        }
        if (transport) {
            await disconnectFlasher();
        }
        const port = await openSerialPort();
        const quietLogger = { log() { }, debug() { }, error() { } };
        improv = new ImprovSerial(port, quietLogger);
        setTerminalConnected(false);

        improv.addEventListener("state-changed", (event) => {
            if (event.detail === IMPROV_AUTHORIZED_STATE) {
                elements.authorizationStatus.textContent = "Authorized. Enter the Wi-Fi credentials.";
                elements.provisionButton.disabled = false;
                setStepState("wifi", "Authorized");
            }
        });

        const deviceInfo = await improv.initialize(5000);
        elements.wifiForm.hidden = false;
        elements.authorizationStatus.textContent = `Press any physical button on the device to authorize Wi-Fi setup. ${deviceInfo?.name ?? "Pixelix"} is waiting.`;
        setStepState("wifi", "Waiting for button");

        await waitForAuthorization(improv);
        elements.authorizationStatus.textContent = "Authorized. Enter the Wi-Fi credentials.";
        elements.provisionButton.disabled = false;
    } catch (error) {
        elements.authorizationStatus.textContent = error.message;
        addStatus(error.message, "error");
        elements.wifiConnectButton.disabled = false;
        if (improv) {
            await closeImprovSession();
        } else if (serialPort && serialPort.readable && !serialPort.readable.locked) {
            await serialPort.close();
        }
    }
}

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

    setTerminalConnected(false);
}

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
        await closeImprovSession();
        elements.wifiConnectButton.disabled = false;
    } catch (error) {
        elements.passwordInput.value = "";
        elements.authorizationStatus.textContent = "Wi-Fi setup failed. Check the credentials or use the captive portal.";
        elements.provisionButton.disabled = false;
        addStatus(`Wi-Fi provisioning failed: ${error.message}`, "error");
    }
}

if (!("serial" in navigator) || !window.isSecureContext) {
    elements.chipStatus.textContent = "Web Serial requires a secure context in Chrome or Edge on desktop.";
    elements.connectButton.disabled = true;
    elements.wifiConnectButton.disabled = true;
    elements.terminalConnectButton.disabled = true;
} else {
    elements.terminalConnectButton.disabled = false;
    loadManifest().catch((error) => {
        elements.chipStatus.textContent = error.message;
        addStatus(error.message, "error");
    });
}

elements.connectButton.addEventListener("click", connectToBoard);
elements.flashButton.addEventListener("click", flashBoard);
elements.wifiConnectButton.addEventListener("click", connectForWifiSetup);
elements.wifiForm.addEventListener("submit", provisionWifi);
elements.terminalConnectButton.addEventListener("click", () => {
    if (terminalReader) {
        disconnectTerminal().catch((error) => addStatus(error.message, "error"));
    } else {
        connectTerminal();
    }
});
elements.terminalClearButton.addEventListener("click", () => {
    elements.terminalOutput.textContent = "";
});
elements.terminalForm.addEventListener("submit", sendTerminalCommand);
document.getElementById("clear-status").addEventListener("click", () => elements.statusList.replaceChildren());