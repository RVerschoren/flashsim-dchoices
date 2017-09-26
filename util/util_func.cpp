/* Copyright 2017 Robin Verschoren */

/* util_func.cpp*/

/* FlashSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version. */

/* FlashSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License
 * along with FlashSim.  If not, see <http://www.gnu.org/licenses/>. */

/****************************************************************************/

/* Implementation of utility functions.
 * 2017 Robin Verschoren
 */

#include "../util.h"
#include <fstream>
#include <iostream>
#include <iomanip>


std::string space_to_zero(std::string text) {
    for(std::string::iterator it = text.begin(); it != text.end(); ++it) {
        if(*it == ' ') {
            *it = '0';
        }
    }
    return text;
}

std::string print_fract(const double d, const unsigned int width)
{
    //Fractional part as string
    const int fraction = std::pow(10, width)*(d - std::floor(d));
    std::stringstream ss;
     ss << std::setw(width) << fraction;
    return space_to_zero(ss.str());
}

std::string num2str(const unsigned int num)
{
    std::stringstream ss;
     ss << num;
    return ss.str();
}

std::string create_oracle_filename(const std::string traceID, const double f, const unsigned int nrFrames)
{
    return (traceID + "-" + print_fract(f) + "-" + num2str(nrFrames) + "-oracle.csv");

}
