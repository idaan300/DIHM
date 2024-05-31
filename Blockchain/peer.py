#!/usr/bin/env python3
import json
import sys
print(sys.path)
from Blockchain import Blockchain
from Block import Block
from flask import Flask, request
import requests
import base64

blockchain = Blockchain()
#peers list
validity = True
cur_index = 0
peers = []
headers = {
    "Authorization": "Bearer NNSXS.Z5XAZZ7T7PTICIRYEVJBJEODFKLEGCV75BUOMWY.KZG7VOHDCL4HXKA3F3V2XJQDMOWKV2HUM4UMQNZYZSD3SUDFSQJA",
    "Content-Type": "application/json",
    "User-Agent": "DIHM-LoRa/1.0"
}
url = "https://eu1.cloud.thethings.network/api/v3/as/applications/dihm-module/webhooks/blockchain-server/devices/eui-70b3d57ed0066206/down/push"

app = Flask(__name__)

@app.route("/")
def index():
    return "Welcome to the chain"

@app.route("/new_transaction", methods=["POST"])
# new transaction added to the block. When user selects to submit new request
def new_transaction():
    file_data = request.get_json() #get json response
    required_fields = ["user", "description","v_file", "file_data", "file_size"]
    #if any of the fields is missing dont append and throw the message
    for field in required_fields:
        if not file_data.get(field):
            return "Transaction does not have valid fields!", 404
    #else append it to pending transaction
    blockchain.add_pending(file_data)
    #blockchain.add_block(file_data)
    return "Success", 201

@app.route("/chain", methods=["GET"])
def get_chain():
    global validity
    print("global validity=", validity)
    chain = []
    #create a new chain from our blockchain
    for block in blockchain.chain:
        chain.append(block.to_dict())
    validity = blockchain.check_chain_validity(chain)
    #print("Chain Len: {0}".format(len(chain)))
    return json.dumps({"length" : len(chain), "chain" : chain})

@app.route("/webrequest", methods=['GET', 'POST'])
def info():
    global cur_index
    chain = []
    for block in blockchain.chain:
        chain.append(block.to_dict())
    inf = blockchain.getInfo(chain)
    data_bytes = inf.encode('utf-8')
    encoded_data = base64.b64encode(data_bytes).decode('utf-8')
    chunk_size = 241
    #chunk = [encoded_data[0:chunk_size]] # for i in range(0, len(encoded_data), chunk_size)]
    total_chunks = len(encoded_data) // chunk_size #+ (1 if len(encoded_data) % chunk_size > 0 else 0)
    print("Total Chunks: ", total_chunks)
    print("cur index = ", cur_index)

    if cur_index < total_chunks:
        start = cur_index * chunk_size
        end = start + chunk_size
        chunk = encoded_data[start:end]
        print("current chunk:", chunk)

        payload_data = {
            "downlinks": [{
                "frm_payload": chunk,
                "f_port": 15,
                "priority": "NORMAL"
            }]
        }
        response = requests.post(url, json=payload_data, headers=headers)
        cur_index += 1
    else:
        payload_data = {
            "downlinks": [{
                "frm_payload": "99",  # Use string "99" for frm_payload
                "f_port": 15,
                "priority": "NORMAL"
            }]
        }
        response = requests.post(url, json=payload_data, headers=headers)
        cur_index = 0  # Reset index after all chunks are sent

    print("response:", response)
    
    if response.status_code == 200:
        print("Downlink message scheduled successfully!")
        return "Success"
    else:
        print("Failed to schedule downlink message. Status code:", response.status_code)
        return "Failed"

@app.route("/valid", methods=["GET"])
def getValid():
    global validity
    if validity == True:
        return "Chain In Order"
    else:
        return "Problem in Chain"

@app.route("/pending", methods=["GET"])
def get_pending():
    # consensus()
    chain = []
    #create a new chain from our blockchain
    for block in blockchain.pending:
        chain.append(block.to_dict())
    #print("Chain Len: {0}".format(len(chain)))
    return json.dumps({"length" : len(chain), "chain" : chain})

@app.route("/mine", methods=["GET"])
#Mines pending tx blocks and call mine method in blockchain
def mine():
    result = blockchain.mine()
    if result:
        return "Block #{0} mined successfully.".format(result)
    else:
        return "No pending transactions to mine."
    
@app.route("/delete", methods=["GET"])
#Mines pending tx blocks and call mine method in blockchain
def delete():
    result = blockchain.delete()
    if result:
        return "Block #{0} removed successfully.".format(result)
    else:
        return "No pending transactions to mine."

app.run(host="0.0.0.0", port=80)
