# Exponential Function Calculator by Threadpool

## Introduction

Similar idea as in project2_multi_process.
Only difference is calculating the exponential function by using multi-thread, which is managed by thread pool.

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