import datetime
from Block import Block
import json

class Blockchain:


    def __init__(self):
        self.pending = [] # pending list of data that needs to go on chain.
        start = self.load_blockchain()
        if(start == "null"):
            self.chain = [] # blockchain
            genesis = self.create_genesis()
            genesis.hash = genesis.calc_hash()
            self.chain.append(genesis)
            self.save_blockchain(self.chain)
        else:
            self.chain = start
        print(self.chain)

    def create_genesis(self): 
        block = Block({'user': 'System', 'v_file': 'Genesis', 'file_data': "0", 'file_size': 0},self.getDateTime(), "0")
        print("bLOCK CREATED")
        try:
            #print("TESTSSSSS ",json.dumps((block.transactions,block.timestamp, block.prev_hash)))
            bl = block.to_dict()
            print("TEST ",json.dumps((bl)))
        except TypeError as e:
            print("Serialization error:", e)

        return Block({'user': 'System', 'v_file': 'Genesis', 'file_data': "0", 'file_size': 0},self.getDateTime(), "0")
    
    def add_block(self,data): #TODO ADD VALIDATION OF BLOCK FIRST
        prev_block = self.chain[-1]
        new_block = Block(data,self.getDateTime(), prev_block.hash)
        new_block.hash = new_block.calc_hash()
        self.chain.append(new_block)
    
    def getDateTime(self):
        dt = datetime.datetime.now()
        dt_str = dt.isoformat()
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
            json.dump(chain, file, indent=4)

    def load_blockchain(self):
        try:
            with open('blockchain.txt', 'r') as file:
                chain = []
                dict = json.load(file)
                print(dict)
                for block in dict:
                    Block(transactions=block.get('transactions', []),timestamp=block.get('timestamp',''), prev_hash=block.get('prev_hash', ''), nonce=block.get('nonce',0),hash=block.get('hash',''))
                    chain.append(Block)
                return chain
        except (FileNotFoundError, json.JSONDecodeError):
            print("No existing blockchain found or error in parsing. Starting a new blockchain.")
            return "null"  # or return a new blockchain with just the genesis block

blockchain = Blockchain()

    #print("cur hash: ", block.hash)