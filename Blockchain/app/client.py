#!/bin/python3
import json
import os
import requests
from flask import Flask, render_template, redirect, request,send_file
from werkzeug.utils import secure_filename
from app import app
from timeit import default_timer as timer

ADDR = "http://127.0.0.1:8000"

# Stores all the post transaction in the node
request_tx = []
#store filename
files = {}
#destiantion for upload files
UPLOAD_FOLDER = "app/Uploads"
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

#peers list
peers = []

app = Flask(__name__)

@app.route("/")
def index():
    #get_tx_req()
    return render_template("index.html",title="FileStorage",subtitle = "A Decentralized Network for File Storage/Sharing",node_address = ADDR)#,request_tx = request_tx)

app.run(host="0.0.0.0", port=80)
