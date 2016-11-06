from flask import Flask, render_template, request, json, send_from_directory
import socket
import sys

app = Flask(__name__)

@app.route('/')
def submit():
	return render_template('index.html')

@app.route('/response', methods=['POST'])
def receive():
	query = request.json['query']
	
	#process query
	result=retrieveQuery(query)

	#return json
	return json.dumps({'status':'OK','query':query});

@app.route('/loadjson')
def sendJson():
	return none

@app.route('/listen')
def getResults():
	serverSocket = socket.socket(AF_INET, SOCK_STREAM, 0);
	listen = 9999
	vinayb_ip=""
	serverSocket.bind((vinayb_ip,listen));
	serverSocket.listen(1)
	max_length=1024

	while true:
		conn, addr=serverSocket.accept()
		# 'connected'
		data=serverSocket.recv(max_length)
		return data

# @app.route('/bar.html/<path:path>')
# def bar():
# 	return send_from_directory('/', path)

def retrieveQuery(query):
	vinayb_ip = "10.50.42.99"
	chetan_ip = ""
	port=4009
	max_length=1024
	
	try:
		clientSocket=socket.socket()
		clientSocket.connect((vinayb_ip,port))
		data=clientSocket.recv(max_length)
		clientSocket.close	
		return data
	except Exception, e:
		raise e
	finally:
		pass

if __name__ =="__main__":
	app.run()

