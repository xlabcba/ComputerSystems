all:
	check
clean:
	rm -rf master worker

build:
	gcc -g -o worker worker.c && gcc -g -o master master.c

check:
	./master --worker_path ./worker --wait_mechanism $$MECHANISM -x $$X -n $$N