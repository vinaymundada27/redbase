import socket

vinayb_ip = "10.50.41.166"
port=8888
clientSocket=socket.socket()
clientSocket.connect((vinayb_ip,port))

clientSocket.send('Hey wassup!')
data=clientSocket.recv(1024)
print data

clientSocket.close