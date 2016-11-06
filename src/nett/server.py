import socket
import sys

def spawnServer():
	serverSocket = socket.socket()
	port = 8899
	host="10.50.42.93"
	serverSocket.bind((host,port))
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


