import socket
import sys

def spawnServer():
	serverSocket = socket.socket()
	port = 8888
	host="127.0.0.1"
	serverSocket.bind((host,port));
	serverSocket.listen(5)
	print 'Server listening...'
	max_length=1024

	while True:
		conn, addr=serverSocket.accept()
		print 'connection received'
		data=conn.recv(max_length);
		print data
		conn.send('OKKKAYY!')
		# conn.close

if __name__== "__main__" :
	spawnServer()


