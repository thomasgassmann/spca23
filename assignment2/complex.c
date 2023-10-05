#include <stddef.h>

/*
 * Copyright 2011 Steven Gribble
 *  You should have received a copy of the GNU General Public License
 *  along with 333exercises.  If not, see <http://www.gnu.org/licenses/>.
 */

// A complex number is ((real) + i*(imaginary))

// Student TODO: define the struct type complex, containing doubles for the real and imaginary parts of a complex number (in that order).
struct complex
{
    double real;
    double imaginary;
};

// Adds a and b, returns the result.
struct complex complex_add(struct complex a, struct complex b);

// Subtracts b from a, returns the result.
// Student TODO: Similarly, declare the function complex_subtract.
struct complex complex_subtract(struct complex a, struct complex b);

// Multplies a times b, returns the result.
// Student TODO: Similarly, declare the function complex_multiply.
struct complex complex_multiply(struct complex a, struct complex b);

// Divides a by b, returns the result.  On some
// architectures and compilers, this function
// might cause a divide-by-zero exception to be thrown
// to the OS for some values of b; on other architectures,
// this might cause the fields of the returned complex
// to be set to floating point NAN.
// Student TODO: Similarly, declare the function complex_divide.
struct complex complex_divide(struct complex a, struct complex b);

/*
 * Copyright 2011 Steven Gribble
 *
 *  This file is the solution to an exercise problem posed during
 *  one of the UW CSE 333 lectures (333exercises).
 *
 *  333exercises is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  333exercises is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with 333exercises.  If not, see <http://www.gnu.org/licenses/>.
 */

struct complex complex_add(struct complex a, struct complex b)
{
    struct complex res;
    res.real = a.real + b.real;
    res.imaginary = a.imaginary + b.imaginary;
    return res;
}

struct complex complex_subtract(struct complex a, struct complex b)
{
    struct complex res;
    res.real = a.real - b.real;
    res.imaginary = a.imaginary - b.imaginary;
    return res;
}

struct complex complex_multiply(struct complex a, struct complex b)
{
    // (a + ib) (c + id) =
    // ac - bd + (ad + bc)i
    struct complex res;
    res.real = a.real * b.real - a.imaginary * b.imaginary;
    res.imaginary = a.real * b.imaginary + a.imaginary * b.real;
    return res;
}

struct complex complex_divide(struct complex a, struct complex b)
{
    // (a + ib) / (c + id) =
    // ((a + ib) * (c - id)) / ((c + id) * (c - id)) =
    // (ac + bd + (bc - ad)i) / (c^2 + d^2) =
    struct complex res;
    if (b.real == 0 && b.imaginary == 0)
    {
        res.real = 0.0 / 0.0;
        res.imaginary = 0.0 / 0.0;
        return res;
    }

    double denominator = b.real * b.real + b.imaginary * b.imaginary;

    res.real = a.real * b.real + a.imaginary * b.imaginary;
    res.imaginary = a.imaginary * b.real - a.real * b.imaginary;

    res.real /= denominator;
    res.imaginary /= denominator;

    return res;
}

int main() {
    
}