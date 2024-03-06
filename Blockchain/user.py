from cryptography.fernet import Fernet
import json

class user:
    # Example user data
    user1 = {"username": "Joris", "password": "unknown", "user_type": "admin"}
    key = Fernet.generate_key()
    cipher_suite = Fernet(key)

    def __init__(self):
        #self.index = index
        start = self.load()
        if(start == "null"):
            self.list = []
            self.list.append(self.user1)
            self.save(self.list)
        else:
            self.list = start

    def addAccount(self, name, password, type):
        new_user = {"username": name, "password": password, "user_type": type}
        self.list.append(new_user)
        self.save(self.list)


    def save(self, list):
        with open('protected.txt', 'w') as file:
            #chain = self.serializeBlockchain(blockchain)
            encrypted = []
            for i in list:
                c = self.encrypt_data(json.dumps(i))
                encrypted.append(c)
            json.dump(encrypted, file, indent=4)

    def load(self):
        try:
            with open('protected.txt', 'r') as file:
                list = []
                dict = json.load(file)
                print("dict: ",dict)
                for p in dict:
                    d = json.loads(self.decrypt_data(p))
                    list.append(d)
                return list
        except (FileNotFoundError, json.JSONDecodeError):
            print("No existing database found or error in parsing. Starting a new database.")
            return "null"  # or return a new blockchain with just the genesis block
    
    def encrypt_data(self, data):
        return self.cipher_suite.encrypt(data.encode()).decode()

    # Function to decrypt data
    def decrypt_data(self,data):
        return self.cipher_suite.decrypt(data.encode()).decode()
    
admin = user()
#admin.save()