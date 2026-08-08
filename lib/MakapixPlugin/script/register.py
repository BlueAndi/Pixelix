"""
    Register Makapix plugin as player.
"""
import sys
from requests import post

def register_makapix_player() -> int:
    """
    Register a Makapix player and obtain a player key.

    Returns:
        int: 0 on success, 1 on failure.
    """
    status = 0

    url = "https://makapix.club/api/player/provision"
    headers = {
        "Content-Type": "application/json"
    }
    payload = {
        "device_model": "Pixelix",
        "firmware_version": "9.0.2"
    }

    response = post(url, json=payload, headers=headers, timeout=10)

    if response.status_code == 201:
        data = response.json()
        player_key = data.get("player_key")
        registration_code = data.get("registration_code")
        expires_at = data.get("registration_code_expires_at")
        mqtt_broker = data.get("mqtt_broker", {})
        mqtt_host = mqtt_broker.get("host")
        mqtt_port = mqtt_broker.get("port")

        print(f"Makapix player registered successfully. Player Key: {player_key}")
        print(f"Registration Code: {registration_code}")
        print(f"Expires At: {expires_at}")
        print(f"MQTT Broker Host: {mqtt_host}")
        print(f"MQTT Broker Port: {mqtt_port}")

    else:
        print(f"Failed to register Makapix player. Status Code: {response.status_code}")
        status = 1

    return status

if __name__ == "__main__":
    sys.exit(register_makapix_player())
