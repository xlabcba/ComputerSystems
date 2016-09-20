all:
	check
clean:
	rm -rf master threadpool

build:
	gcc -g master.c threadpool.c -D THPOOL_DEBUG -lpthread -o master

check:
	./master -thread_number $$T -x $$X -n $$N