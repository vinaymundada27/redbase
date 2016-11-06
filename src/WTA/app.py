from flask import Flask, render_template, request, json, send_from_directory, json
import socket
import sys

app = Flask(__name__)

@app.route('/')
def submit():
	return render_template('index.html')

def tupleSplit(tuple):
    vals = tuple.split(',')
    d = {}
    count = 0
    for val in vals:
    	d[str(count)] = val
    	count += 1
    return d

@app.route('/settablename', methods=['POST'])
def settablename():
	table = request.json['tablename']
	target = open('static/tablename', 'w')
	target.write(table)
	return json.dumps({'result' : 'OK'})

@app.route('/loadattr', methods=['POST'])
def loadattr():
	table = request.json['tablename']
	file_name = 'static/' + table + '_attr'
	content = []
	with open(file_name) as f:
		content = f.readlines()
		content = [x.strip('\n') for x in content]
	content = ','.join(content)
	print(content)
	return json.dumps({'names' : content})

@app.route('/response', methods=['POST'])
def receive():
	query = request.json['query']
	#process query
	result=retrieveQuery(query)
	tuples=result.decode().split('\n')
	tuples.pop()
	print("tuples : ", tuples)

	dic = list(map(tupleSplit, tuples))
	print(dic)
	
	# write json to file data.json used by all graphs
	string = json.dumps(dic)
	target = open('static/data.json', 'w')
	target.write(string)

	# values=tuples['result'].split('\n')
	# print 'values : ', values

	# # write result to file data.json
	# var json=jQuery.parseJSON(r);  

 #        var values = json['result'].split('\n');
 #        console.log(values);
 #        var dict = values.map(function(tup) {
 #            var strs = tup.split(',');
 #            var d = {};
 #            d["X"] = strs[0];
 #            d["Y"] = strs[1];
 #            return d;

	return json.dumps({'result':'OK'});

def retrieveQuery(query):
	vinayb_ip = "127.0.0.1"
	port=8888
	max_length=102400
	clientSocket=socket.socket()
	clientSocket.connect((vinayb_ip,port))
	clientSocket.send(query.encode())
	data=clientSocket.recv(max_length)
	print(data)
	clientSocket.close
	return data

if __name__ =="__main__":
	app.run()

