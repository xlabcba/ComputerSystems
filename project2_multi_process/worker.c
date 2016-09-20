#include "worker.h"

unsigned long long calcFac(int n){
    unsigned long long result;
    result = 1;
    for(int i = 2; i <= n; i++){
        result *= i;
    }
    return result;
}

unsigned long long calcPow(int x, int n){
    unsigned long long result;
    result = 1;
    for(n; n > 0; n--){
        result *= x;
    }
    return result;
}

double calcResult(int x, int n){
    unsigned long long fraction, numerator;
    double result;
    fraction = calcPow(x,n);
    numerator = calcFac(n);
    result = (double) fraction / numerator;
    return result;
}

int main(int argc, char *argv[]){
    int x, n;
    double result;

    x = atoi(argv[2]); // get base
    n = atoi(argv[4]); // get factor
    result = calcResult(x,n);

    if (argc == 6) {
        write(1, &result, sizeof(double));
    } else if (argc == 5) {
        printf("x^n / n! : %f\n", result);
    }

    return 0;

}