import datetime
from Block import Block
import json

class Blockchain:


    def __init__(self):
        self.pending = [] # pending list of data that needs to go on chain.
        start = self.load_blockchain()
        if(start == 0):
            self.chain = [] # blockchain
            genesis = self.create_genesis()
            genesis.hash = genesis.calc_hash()
            self.chain.append(genesis)
            self.save_blockchain(self.chain)
        else:
            self.chain = start
        print(self.chain)

    def create_genesis(self): 
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
    
    def save_blockchain(blockchain):
        with open('blockchain.txt', 'w') as file:
            json.dump(blockchain, file, indent=4)

    def load_blockchain():
        try:
            with open('blockchain.txt', 'r') as file:
                return json.load(file)
        except (FileNotFoundError, json.JSONDecodeError):
            print("No existing blockchain found or error in parsing. Starting a new blockchain.")
            return 0  # or return a new blockchain with just the genesis block

blockchain = Blockchain()
blockchain.add_block("23123131")

print("Blockchain:")
for block in blockchain.chain:
    print("data: ", block.transactions)
    print("time: ", block.timestamp)
    print("prev hash: ", block.prev_hash)
    #print("cur hash: ", block.hash)