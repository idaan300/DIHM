import requests

# Define the authorization token
authorization_token = "Bearer NNSXS.Z5XAZZ7T7PTICIRYEVJBJEODFKLEGCV75BUOMWY.KZG7VOHDCL4HXKA3F3V2XJQDMOWKV2HUM4UMQNZYZSD3SUDFSQJA"

# Define the payload data
payload_data = {
    "downlinks": [{
        "frm_payload": "vu8=",
        "f_port": 15,
        "priority": "NORMAL"
    }]
}

# Define the headers
headers = {
    "Authorization": authorization_token,
    "Content-Type": "application/json",
    "User-Agent": "DIHM-LoRa/1.0"
}

# Define the URL
url = "https://eu1.cloud.thethings.network/api/v3/as/applications/dihm-module/webhooks/blockchain-server/devices/eui-70b3d57ed0066206/down/push"

# Make the POST request
response = requests.post(url, json=payload_data, headers=headers)

# Check if the request was successful
if response.status_code == 200:
    print("Downlink message scheduled successfully!")
else:
    print("Failed to schedule downlink message. Status code:", response.status_code)

