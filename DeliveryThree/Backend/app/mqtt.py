#!/usr/bin/python3

class MQTT:

    def __init__(self, mongo):
        import paho.mqtt.client as mqtt
        from json import loads, dumps
        from random import randint

        self.mongo = mongo
        self.loads = loads
        self.dumps = dumps
        self.ID = f"IOT_B_{randint(1,1000000)}"

        self.sub_topics = [("620171852", 0), ("620171852_sub", 0), ("/elet2415", 0)]

        self.client = mqtt.Client(client_id=self.ID, clean_session=True, reconnect_on_failure=True)

        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        self.client.on_subscribe = self.on_subscribe

        self.client.message_callback_add("620171852", self.update)

        self.client.connect_async("www.yanacreations.com", 1883, 60)
        self.client.loop_start()

    def on_connect(self, client, userdata, flags, rc):
        print(f"Connected to MQTT broker with result code {rc}")
        client.subscribe(self.sub_topics)

    def on_message(self, client, userdata, msg):
        print(f"Message received on topic {msg.topic}")

    def on_disconnect(self, client, userdata, rc):
        print(f"Disconnected from MQTT broker with result code {rc}")

    def on_subscribe(self, client, userdata, mid, granted_qos):
        print(f"Subscription successful: {granted_qos}")

    def update(self, client, userdata, msg):
        try:
            payload = msg.payload.decode("utf-8")
            print(f"Raw payload: {payload}")

            update = self.loads(payload)
            result = self.mongo.add_update(update)

            print(f"Mongo insert result: {result}")

        except Exception as e:
            print(f"MQTT update error: {e}")