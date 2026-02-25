/* ===========================================================================
 * idk-nagger-4 : Flash Writer Utility
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Optional utility to verify flash partition layout at runtime.
 * Reads the partition table and prints offsets/sizes for debugging.
 * ======================================================================== */

#ifndef FLASH_WRITER_H
#define FLASH_WRITER_H

/**
 * @brief Print all partitions found in the flash partition table.
 *        Useful for verifying the layout matches partition_table.csv.
 */
void flash_writer_print_partitions(void);

#endif /* FLASH_WRITER_H */
