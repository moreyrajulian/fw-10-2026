#include <mathlib.h>
#include <stdint.h>

int32_t math_factorial(int32_t n)
{
    if (n < 0 || n > 12) {
        return -1;
    }

    int32_t resultado = 1;

    for (int32_t i = 2; i <= n; i++) {
        resultado = resultado * i;
    }

    return resultado;
}

int32_t math_gcd(int32_t a, int32_t b)
{
    if (a < 0 || b < 0) {
        return -1;
    }

    while (b != 0) {
        int32_t resto = a % b;
        a = b;
        b = resto;
    }

    return a;
}

uint32_t math_isqrt(uint32_t n)
{
    uint32_t resultado = 0;
    uint32_t candidato = 1;

    while (candidato <= n / candidato) {
        resultado = candidato;
        candidato++;
    }

    return resultado;
}

int math_is_prime(int32_t n)
{
    if (n < 2) {
        return 0;
    }

    uint32_t numero = (uint32_t)n;
    uint32_t limite = math_isqrt(numero);

    for (uint32_t divisor = 2; divisor <= limite; divisor++) {
        if (numero % divisor == 0) {
            return 0;
        }
    }

    return 1;
}