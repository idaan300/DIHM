#!/bin/python3
import json
import os
import requests
from flask import Flask, render_template, redirect, request,send_file,session
from werkzeug.utils import secure_filename
from app import app
from timeit import default_timer as timer

ADDR = "http://127.0.0.1:80"

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

def get_tx_req(): #get blockchain
    global request_tx
    chain_addr = "{0}/chain".format(ADDR)
    resp = requests.get(chain_addr)
    if resp.status_code == 200:
        content = []
        chain = json.loads(resp.content.decode())
        print("ChAIN=", chain)
        for block in chain["chain"]:
            print("BLOCK=", block["transactions"])
            block["user"] = block["transactions"]["user"]
            block["v_file"] = block["transactions"]["v_file"]
            block["file_data"] = block["transactions"]["file_data"]
            block["file_size"] = block["transactions"]["file_size"]
            # for trans in block["transactions"]:
            #     print(trans)
            #     trans["timestamp"] = block["timestamp"]
            #     trans["hash"] = block["prev_hash"]
            #     content.append(trans)
            content.append(block)
            print(content)
        request_tx = content#sorted(content,key=lambda k: k["hash"],reverse=True)


@app.route("/")
def index():
    if not session.get('logged_in'):
        return redirect("/login")  # Redirect to login if not logged in
    get_tx_req()
    return render_template("index.html",title="FileStorage",subtitle = "A Decentralized Network for File Storage/Sharing",node_address = ADDR,request_tx = request_tx)

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        # Handle the login logic here
        username = request.form['username']
        password = request.form['password']
        
        # Verify username and password here
        # For example, check against user data in the database
        if username == "correct_username" and password == "correct_password":
            # Login success
            session['logged_in'] = True
            return redirect("/")  # Redirect to another page, e.g., home
        else:
            # Login failed
            return "Login Failed"

    # GET request - show the login form
    return render_template('login.html')

@app.route("/submit", methods=["POST"])
# When new transaction is created it is processed and added to transaction
def submit():
    start = timer()
    user = request.form["user"]
    up_file = request.files["v_file"]
    
    #save the uploaded file in destination
    up_file.save(os.path.join("app/Uploads/",secure_filename(up_file.filename)))
    #add the file to the list to create a download link
    files[up_file.filename] = os.path.join(app.root_path, "Uploads", up_file.filename)
    #determines the size of the file uploaded in bytes 
    file_states = os.stat(files[up_file.filename]).st_size 
    #create a transaction object
    post_object = {
        "user": user, #user name
        "v_file" : up_file.filename, #filename
        "file_data" : str(up_file.stream.read()), #file data
        "file_size" : file_states   #file size
    }
    print(post_object)
    # Submit a new transaction
    address = "{0}/new_transaction".format(ADDR)
    requests.post(address, json=post_object)
    end = timer()
    print(end - start)
    return redirect("/")

@app.route("/submit/<string:variable>",methods = ["GET"])
def download_file(variable):
    p = files[variable]
    return send_file(p,as_attachment=True)

app.run(host="0.0.0.0", port=8000)
