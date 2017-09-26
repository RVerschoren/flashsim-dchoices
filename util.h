/* Copyright 2017 Robin Verschoren */

/* ssd.h */

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

/* util.h
 * Robin Verschoren
 * Utility classes and functions
 * 	Mainly used in the implementations of the GCA.
 */

#include <random>
#include <ctime>
#include <string>
#include <sstream>

#ifndef _SSD_UTIL_H
#define _SSD_UTIL_H

class RandNrGen final
{
public:
    static RandNrGen& getInstance();

    /**
     * @brief Resets the PRNG.
     * @param seed Optional seed to re-seed the PRNG with.
     */
    void reset(uint32_t seed = time(NULL));

    /**
     * @brief Returns a double pseudorandom value in [0, 1).
     * @return A pseudorandom value in [0, 1)
     */
    double get();

    /**
     * @brief Returns an integer pseudorandom value in [0, S).
     * @param S Least upper bound of the possible returned values
     * @return An integer pseudorandom value in [0, S)
     */
    unsigned long get(unsigned long S);

    /**
     * @brief Returns an integer pseudorandom value in [0, S).
     * @param S Least upper bound of the possible returned values
     * @return An integer pseudorandom value in [0, S)
     */
    unsigned int get(unsigned int S);

private:
    // Singleton
    RandNrGen();
    ~RandNrGen() = default;

    RandNrGen(const RandNrGen&) = delete;
    RandNrGen& operator=(const RandNrGen&) = delete;
    RandNrGen(RandNrGen&&) = delete;
    RandNrGen& operator=(RandNrGen&&) = delete;

    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

};

std::string space_to_zero(std::string text);
std::string print_fract(const double d, const unsigned int width = 3);
std::string num2str(const unsigned int num);
std::string create_oracle_filename(const std::string traceID, const double hotFraction, const unsigned int nrFrames);

#endif
