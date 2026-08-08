import time

import paho.mqtt.client as mqtt

# MQTT broker configuration
BROKER_URL = "dev.makapix.club"
BROKER_PORT = 8883
CLIENT_ID = "99c57cf3-0796-4bb6-9ff8-e87a2152bcb3"
DURATION = 300  # 5 minutes in seconds

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
    else:
        print(f"Connection failed with code {rc}")

def on_disconnect(client, userdata, rc):
    print("Disconnected from MQTT broker")

def main():
    client = mqtt.Client(client_id=CLIENT_ID)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

    # Use TLS for secure connection
    client.tls_set()

    try:
        client.connect(BROKER_URL, BROKER_PORT, keepalive=60)
        client.loop_start()

        time.sleep(DURATION)

    finally:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
