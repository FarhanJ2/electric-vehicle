import socket

HOST = "0.0.0.0"
PORT = 2601

server = socket.socket()
server.bind((HOST, PORT))
server.listen(1)
print("Waiting for Pico...")

client, addr = server.accept()
print("Connected from:", addr)

while True:
    data = client.recv(1024)
    if not data:
        break
    print(data.decode().strip())
