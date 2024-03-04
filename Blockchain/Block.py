import hashlib

class Block:


    def __init__(self, transactions, timestamp, prev_hash):
        #self.index = index
        self.transactions = transactions
        self.timestamp = timestamp
        self.prev_hash = prev_hash
        self.nonce = 0

    def calc_hash(self):
        sha = hashlib.sha256()
        data = str(self.transactions) + str(self.timestamp) + str(self.prev_hash) + str(self.nonce)
        sha.update(data.encode('utf-8'))
        return sha.hexdigest()
    
