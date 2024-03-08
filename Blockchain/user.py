from cryptography.fernet import Fernet
import json
import os

class User:
    # Example user data
    user1 = {"username": "Joris", "password": "unknown", "user_type": "admin"}
    
    with open('secret.key', 'rb') as key_file:
        print("fernet key found")
        key = key_file.read()
        print("key=", key)
        
    # if key == "" or None:
    #     print("fernet key not found")
    #     with open('secret.key', 'wb') as key_file:
    #         key = Fernet.generate_key()
    #         key_file.write(key)

    cipher_suite = Fernet(key)#.encode()

    def __init__(self):
        #self.index = index
        start = self.load()
        if(start == "null"):
            self.list = []
            self.list.append(self.user1)
            #self.addAccount("stakeholder", "stakeholder", "view")
            self.save(self.list)
        else:
            self.list = start

    def addAccount(self, name, password, type):
        new_user = {"username": name, "password": password, "user_type": type}
        self.list.append(new_user)
        self.save(self.list)

    def changePass(self, name, new_password):
        list = []
        try:
            with open('protected.txt', 'r') as file:
                dict = json.load(file)
                for p in dict:
                    d = json.loads(self.decrypt_data(p))
                    print(d)
                    if d['username'] == name:
                        print("password changed to", new_password)
                        d['password'] = new_password
                    list.append(d)
        except (FileNotFoundError, json.JSONDecodeError):
            print("No existing database found or error in parsing. Starting a new database.")
        self.save(self.list)
        print("saved=", list)


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
#admin.save()
