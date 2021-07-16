compile:
	gcc -g -W -o server server.c -lrt -lpthread
	gcc -g -W -o client client.c -lrt -lpthread


client_run:
	./client -i 1 -a 127.0.0.1 -p 20000 -o queries.txt
	./client -i 2 -a 127.0.0.1 -p 20000 -o queries.txt
	./client -i 3 -a 127.0.0.1 -p 20000 -o queries.txt
	./client -i 4 -a 127.0.0.1 -p 20000 -o queries.txt
	./client -i 5 -a 127.0.0.1 -p 20000 -o queries.txt

run:
	./server -p 20000 -o pathToLogFile -l 5 -d test_csv.csv

	

