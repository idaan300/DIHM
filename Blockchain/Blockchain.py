import datetime
from Block import Block

class Blockchain:


    def __init__(self):
        self.chain = []
        genesis = self.create_genesis()
        self.chain.append(genesis)

    def create_genesis(self):
        return Block("Genesis Block",datetime.datetime.now(), "0")
    
    def add_block(self,data):
        prev_block = self.chain[-1]
        new_block = Block(data,datetime.datetime.now(), prev_block)
        self.chain.append(new_block)
    


blockchain = Blockchain()
blockchain.add_block("23123131")

print("Blockchain:")
for block in blockchain.chain:
    print("data: ", block.transactions)
    print("time: ", block.timestamp)
    print("prev hash: ", block.prev_hash)
    #print("cur hash: ", block.hash)