#!/bin/python3
import json
from Blockchain import Blockchain
from Block import Block
from flask import Flask, request

blockchain = Blockchain()
#peers list
peers = []

app = Flask(__name__)

@app.route("/")
def index():
    return "Welcome to the chain"

app.run(host="0.0.0.0", port=80)
