import datetime
from Block import Block
import json
import base64
import os
from cryptography.fernet import Fernet

class Blockchain:
    with open('secret1.key', 'rb') as key_file:
        print("fernet key found")
        key = key_file.read()
        print("key=", key)
    cipher_suite = Fernet(key)

    def __init__(self):
        self.pending = [] # pending list of data that needs to go on chain.
        self.difficulty = 1
        #print("loading blockchain from txt")
        start = self.load_blockchain()
        if(start == "null"):
            self.chain = [] # blockchain
            genesis = self.create_genesis()
            genesis.hash = genesis.calc_hash()
            self.chain.append(genesis)
            self.save_blockchain(self.chain)
        else:
            self.chain = start

    def create_genesis(self): 
        block = Block({'user': 'System', 'description': 'File created by system', 'v_file': 'Genesis', 'file_data': "b'DIHM-Digital Inventory of Hazardous Materials'", 'file_size': 0},self.getDateTime(), "0")
        try:
            #print("TESTSSSSS ",json.dumps((block.transactions,block.timestamp, block.prev_hash)))
            bl = block.to_dict()
            print("TEST ",json.dumps((bl)))
        except TypeError as e:
            print("Serialization error:", e)

        return Block({'user': 'System', 'description': 'File created by system', 'v_file': 'Genesis', 'file_data': "0", 'file_size': 0},self.getDateTime(), "0")
    
    def add_block(self,data): #TODO ADD VALIDATION OF BLOCK FIRST
        #prev_block = self.chain[-1]
        new_block = data#Block(data,self.getDateTime(), prev_block.hash)
        self.chain.append(new_block)
        self.save_blockchain(self.chain)
    
    def add_pending(self,data): #TODO ADD VALIDATION OF BLOCK FIRST
        prev_block = self.chain[-1]
        new_block = Block(data,self.getDateTime(), prev_block.hash)
        #new_block.hash = new_block.calc_hash()
        self.pending.append(new_block)
        self.save_pending(self.chain)
    
    def mine(self):
        if(len(self.pending) > 0): #if there is atleast one pending transaction
            new_block = self.pending.pop()
            new_block.hash = self.proofOfWork(new_block)
            self.add_block(new_block)
        else:
            return False
        
    def proofOfWork(self, Block):
        for nonce in range(100000000):
            Block.nonce = nonce
            temphash = Block.calc_hash()
            if temphash.startswith('0' * self.difficulty):
                print("HASHHHHHH FOUND =======", temphash)
                return temphash
        print("=========No HASH FOUND=======")
        return -1

    def delete(self):
        if(len(self.pending) > 0): #if there is atleast one pending transaction
            self.pending.pop()
            self.save_pending(self.chain)
        else:
            return False
        
    def getInfo(self, chain):
        info = ""
        for block in chain:
            if block["prev_hash"] != "0":
                #b = Block(transactions=block.get('transactions', []),timestamp=block.get('timestamp',''), prev_hash=block.get('prev_hash', ''), nonce=block.get('nonce',0))
                info += ("| File: ", block.get('transactions', []), " , Time: ", block.get('timestamp',''), " | ")
        return info

                
    
    def check_chain_validity(self, chain):
        result = True
        er_block = ""
        prev_hash = "0"
        UPLOAD_FOLDER = "app/Uploads/"
        #for every block in the chain
        for block in chain:
            if block["prev_hash"] != "0":
                b = Block(transactions=block.get('transactions', []),timestamp=block.get('timestamp',''), prev_hash=block.get('prev_hash', ''), nonce=block.get('nonce',0))
                block_hash = block["hash"] #get the hash of this block and check if its a valid hash
                decoded_data = base64.b64decode(block["transactions"]["file_data"])
                
                with open(UPLOAD_FOLDER + block["transactions"]["v_file"], 'rb') as file:
                    file_data = file.read()
                if(file_data == decoded_data):
                    print("File integrity in order")
                else:
                    print("File intergrity invalid")
                    result = False

                if block_hash.startswith('0' * self.difficulty):
                    if(b.calc_hash() == block_hash):
                        if prev_hash == b.prev_hash:
                            b.hash = block_hash #update the hash
                            print("======== HASH IN ORDER =========")
                        else:
                            print("prev hash invalid") 
                            result = False
                    else:
                        print("Cur and cal hash invalid")
                        result = False
                else:
                    print("Hash too easy!")
                    result = False
                prev_hash = block_hash #update the previous hash
                if(result == False):
                    er_block = block["transactions"]["v_file"]
            else:
                print("======== HASH IN ORDER =========")
                prev_hash = block["hash"]
        print("result:",result)
        print("error block? ", er_block)
        return result
    
    def getDateTime(self):
        dt = datetime.datetime.now()
        dt_str = dt.strftime("%Y-%m-%d %H:%M:%S")
        return dt_str
    
    def serializeBlockchain(self,blockchain): #for JSON Dumps
        chain = []
        for Block in blockchain:
            b = Block.to_dict()
            chain.append(b)
        return chain
    
    def save_blockchain(self, blockchain):
        with open('blockchain.txt', 'w') as file:
            chain = self.serializeBlockchain(blockchain)
            #chain = self.encrypt_data(chain)
            json.dump(chain, file, indent=4)

    def load_blockchain(self):
        try:
            with open('blockchain.txt', 'r') as file:
                chain = []
                dict = json.load(file)
                #dict = self.decrypt_data(dict)
                for block in dict:
                    #print("entry:",block)
                    b = Block(transactions=block.get('transactions', []),timestamp=block.get('timestamp',''), prev_hash=block.get('prev_hash', ''), nonce=block.get('nonce',0),hash=block.get('hash',''))
                    chain.append(b)
                return chain
        except (FileNotFoundError, json.JSONDecodeError):
            print("No existing blockchain found or error in parsing. Starting a new blockchain.")
            return "null"  # or return a new blockchain with just the genesis block
        
    def save_pending(self, pending):
        with open('pending.txt', 'w') as file:
            chain = self.serializeBlockchain(pending)
            json.dump(chain, file, indent=4)

    def load_pending(self):
        try:
            with open('pending.txt', 'r') as file:
                chain = []
                dict = json.load(file)
                #print("dict: ",dict)
                for block in dict:
                    b = Block(transactions=block.get('transactions', []),timestamp=block.get('timestamp',''), prev_hash=block.get('prev_hash', ''), nonce=block.get('nonce',0),hash=block.get('hash',''))
                    chain.append(b)
                return chain
        except (FileNotFoundError, json.JSONDecodeError):
            print("No existing blockchain found or error in parsing. Starting a new blockchain.")
            return "null"  # or return a new blockchain with just the genesis block
        
    def encrypt_data(self, data):
        encrypted_data_list = []
        for item in data:
            encrypted_item = self.cipher_suite.encrypt(str(item).encode()).decode()
            encrypted_data_list.append(encrypted_item)
        return encrypted_data_list

    def decrypt_data(self, encrypted_data):
        decrypted_data_list = []
        for item in encrypted_data:
            if isinstance(item, str):
                decrypted_item = self.cipher_suite.decrypt(item.encode()).decode()
                decrypted_data_list.append(decrypted_item)
            else:
                decrypted_data_list.append(item)  # Append non-string items as is
        return decrypted_data_list

    


#blockchain = Blockchain()
#blockchain.check_chain_validity(blockchain.chain)
    #print("cur hash: ", block.hash)
