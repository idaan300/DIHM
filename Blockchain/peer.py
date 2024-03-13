#!/bin/python3
import json
from Blockchain import Blockchain
from Block import Block
from flask import Flask, request

blockchain = Blockchain()
#peers list
validity = True
peers = []

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
    chain = []
    #create a new chain from our blockchain
    for block in blockchain.chain:
        chain.append(block.to_dict())
    validity = blockchain.check_chain_validity(chain)
    #print("Chain Len: {0}".format(len(chain)))
    return json.dumps({"length" : len(chain), "chain" : chain})

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
