import hashlib

class Block:


    def __init__(self, transactions, timestamp, prev_hash):
        #self.index = index
        self.transactions = transactions
        self.timestamp = timestamp
        self.prev_hash = prev_hash
        self.nonce = 0

    def to_dict(self):
        return {
            'transactions': self.transactions,  # Assuming transactions are already JSON serializable
            'timestamp': self.timestamp,
            'prev_hash': self.prev_hash,
            'nonce': self.nonce,
            'hash': self.hash if self.hash else None
        }

    def calc_hash(self):
        sha = hashlib.sha256()
        data = str(self.transactions) + str(self.timestamp) + str(self.prev_hash) + str(self.nonce)
        print(data)
        sha.update(data.encode('utf-8'))
        return sha.hexdigest()
    
