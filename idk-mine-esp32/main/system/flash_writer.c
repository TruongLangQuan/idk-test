/* ===========================================================================
 * idk-nagger-4 : Flash Writer Utility — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 * ======================================================================== */

#ifdef ESP_PLATFORM

#include "flash_writer.h"
#include "esp_partition.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "FLASH";

void flash_writer_print_partitions(void)
{
    ESP_LOGI(TAG, "=== Flash Partition Table ===");

    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

    while (it != NULL) {
        const esp_partition_t *part = esp_partition_get(it);
        if (part != NULL) {
            ESP_LOGI(TAG, "  %-12s  type=%02x  sub=%02x  offset=0x%08"PRIx32"  size=0x%08"PRIx32" (%"PRIu32" KB)",
                     part->label,
                     part->type,
                     part->subtype,
                     part->address,
                     part->size,
                     part->size / 1024);
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    ESP_LOGI(TAG, "=============================");
}

#else
/* Host stub */
#include <stdio.h>
void flash_writer_print_partitions(void)
{
    printf("[FLASH] Partition table not available on host build\n");
}
#endif
