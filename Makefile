all:
	cd client_files/ && gcc client.c -o client
	cd server_files/ && gcc server.c -o server
