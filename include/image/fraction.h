#ifndef FRACTION_H
#define FRACTION_H

#include <string>

namespace image
{
    class Fraction
    {
        public:
            constexpr Fraction(int numerator, int denominator) : numerator(numerator), denominator(denominator) {
                simplefy();
            }
            constexpr bool equals(const Fraction &other) const {
                return (this->numerator == other.numerator && this->denominator == other.denominator);
            }
            std::string toString() const {
                return std::to_string(numerator) + " / " + std::to_string(denominator);
            }
            constexpr int getNumerator() const {
                return numerator;
            }
            constexpr int getDenominator() const {
                return denominator;
            }
        private:
            int numerator, denominator;
            constexpr void simplefy() {
                for (int i = denominator * numerator; i > 1; i--)
                {
                    if ((denominator % i == 0) && (numerator % i == 0))
                    {
                        denominator /= i;
                        numerator /= i;
                    }
                }
            }
    };
};

#include "image/fraction.h"

#endif // FRACTION_H
