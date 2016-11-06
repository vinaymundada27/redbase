from flask import Flask, render_template, request, json, send_from_directory, json
import socket
import sys

app = Flask(__name__)

@app.route('/')
def submit():
	return render_template('index.html')

def tupleSplit(tuple):
    val = tuple.split(',')
    d = {}
    d['X']=val[1]
    d['Y']=val[0]
    return d

@app.route('/response', methods=['POST'])
def receive():
	query = request.json['query']
	
	#process query
	result=retrieveQuery(query)
	tuples=result.split('\n')
	tuples.pop()
	print "tuples : ", tuples

	dic = map(tupleSplit, tuples)
	
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
	clientSocket.send(query)
	data=clientSocket.recv(max_length)
	print data
	clientSocket.close
	return data

if __name__ =="__main__":
	app.run()
