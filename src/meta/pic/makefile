CC=clang

pic.a : main.o png.o jpg.o pic.o
	$(CC) -o $@ -g $^

main.o : main.c
	$(CC) -o $@ -c $^

png.o : png.c
	$(CC) -o $@ -c $^

jpg.o : jpg.c
	$(CC) -o $@ -g -c $^

pic.o : pic.c
	$(CC) -o $@ -c $^

clean:
	rm *.o