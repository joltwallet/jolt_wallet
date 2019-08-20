/**
 * @file noise.h
 * @brief Computation noise interrupts to discourage side-channel attacks.
 */

#ifndef JOLT_SYSCORE_NOISE_H__
#define JOLT_SYSCORE_NOISE_H__

/**
 * @brief Initialize and start interrupt to perform noise computations
 *
 */
void jolt_noise_init();

#endif
