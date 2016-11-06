import socket

vinayb_ip = "127.0.0.1"
port=8888
clientSocket=socket.socket()
clientSocket.connect((vinayb_ip,port))
req = raw_input("Enter query: ");
clientSocket.send(req)
data=clientSocket.recv(102400)
print data

clientSocket.close