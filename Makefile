compile:
	g++ -pthread server.cpp -o server -l sqlite3
	g++ client.cpp -o client
	xfce4-terminal -e 'bash -c "./server;  exec bash"'
	xfce4-terminal -e 'bash -c "./client; exit; exec bash"'