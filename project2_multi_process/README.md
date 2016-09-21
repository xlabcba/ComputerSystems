# Exponential Function Calculator by Multi-process

## Introduction

This project aims at building an exponential function calculator by using master-worker architecture. 

The exponential function can be expressed as following:

<img src="https://github.com/xlabcba/ComputerSystems/blob/master/project2_multi_process/figures/equation.png"/>

The master is launched using the following command (e^x for n in [0..n]):

```
$ ./master --worker_path ./worker --wait_mechanism MECHANISM -x 2 -n 12
```

The master spawns n child worker processes, each of which computers x^n / n!:

```
e.g. worker 3: 2^3 / 3! : 1.3333
```

The worker is individually testable:

```
$ ./worker -x 2 -n 3 x^n / n! : 1.3333
```

Following mechanisms in the Master have been implemented:

* Sequential: first worker, followed by second, third, and so on.
* Select: by select system call to read the workers' output in the order it becomes available.
* Poll: The above using poll system call.
* Epoll: The above using epoll system call.

## Run Instruction

1. Open the terminal
2. Go to the directory containing the program
3. Type command “make clean”
4. Type command “make build”
5. Type command “make check MECHANISM=m X=x N=n” 

    m is one mechanism of sequential/select/poll/epoll

    x is power x of e^x you want to calculate

    n is the number of terms you want to expend to estimate the corresponding e^x

6. get result

(For select mechanism, the limitation would be 1024 for n)