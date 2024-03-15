import base64

# Read the contents of the file as bytes
with open('test.pdf', 'rb') as file:
    file_data = file.read()

# Encode the binary data as Base64
file_data_base64 = base64.b64encode(file_data)

# Convert the Base64-encoded bytes to a string
file_data_base64_string = file_data_base64.decode('utf-8')

# Print the Base64-encoded string
print(file_data_base64_string)

# Decode the Base64-encoded string back to binary
decoded_data = base64.b64decode(file_data_base64_string)
print(decoded_data)
with open('test1.pdf', 'wb') as file:
    file.write(decoded_data)

# Verify that the decoded data matches the original data
print(decoded_data == file_data)  # Should print True if decoding is successful
