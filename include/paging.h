#ifndef PAGING_H
#define PAGING_H

#include "types.h"

// Page size: 4KB
#define PAGE_SIZE 4096

// Page directory/table entry flags
#define PAGE_PRESENT    0x1    // Page is in memory
#define PAGE_WRITE      0x2    // Page is writable
#define PAGE_USER       0x4    // Page is user-accessible
#define PAGE_ACCESSED   0x20   // Page has been accessed
#define PAGE_DIRTY      0x40   // Page has been written to

// Page directory entry (points to page table)
typedef uint32_t page_directory_entry_t;

// Page table entry (points to physical page)
typedef uint32_t page_table_entry_t;

// Page directory (1024 entries, each 4 bytes = 4KB total)
struct page_directory {
    page_directory_entry_t entries[1024];
} __attribute__((aligned(4096)));

typedef struct page_directory page_directory_t;

// Page table (1024 entries, each 4 bytes = 4KB total)
typedef struct {
    page_table_entry_t entries[1024];
} __attribute__((aligned(4096))) page_table_t;

// Paging functions
void paging_init(void);
page_directory_t* paging_create_address_space(void);
void paging_destroy_address_space(page_directory_t* pd);
void paging_switch_directory(page_directory_t* pd);
void paging_map_page(page_directory_t* pd, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void paging_identity_map_kernel(page_directory_t* pd);
uint32_t paging_get_physical_address(page_directory_t* pd, uint32_t virtual_addr);

// Get current page directory
page_directory_t* paging_get_current_directory(void);

#endif