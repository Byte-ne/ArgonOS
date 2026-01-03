#include "paging.h"
#include "memory.h"
#include "string.h"
#include "kernel.h"

// Kernel's page directory (identity mapped)
static page_directory_t* kernel_directory = 0;
static page_directory_t* current_directory = 0;

// Page table pool (for simplicity, pre-allocate some page tables)
static page_table_t page_tables[32] __attribute__((aligned(4096)));
static uint32_t next_page_table = 0;

// External function to enable paging (in assembly)
extern void paging_enable(uint32_t page_directory_physical);

// Get a free page table
static page_table_t* alloc_page_table(void) {
    if (next_page_table >= 32) {
        return 0;  // Out of page tables
    }
    page_table_t* pt = &page_tables[next_page_table++];
    memset(pt, 0, sizeof(page_table_t));
    return pt;
}

// Initialize paging system
void paging_init(void) {
    // Allocate kernel page directory
    kernel_directory = (page_directory_t*)kmalloc(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    
    // Identity map kernel (1MB to 4MB: 0x00100000 to 0x00400000)
    paging_identity_map_kernel(kernel_directory);
    
    // Identity map VGA memory (0xB8000)
    paging_map_page(kernel_directory, 0xB8000, 0xB8000, PAGE_PRESENT | PAGE_WRITE);
    
    // Switch to kernel directory and enable paging
    current_directory = kernel_directory;
    paging_enable((uint32_t)kernel_directory);
    
    terminal_writestring("Paging enabled (4KB pages)\n");
}

// Identity map kernel memory (virtual = physical)
void paging_identity_map_kernel(page_directory_t* pd) {
    // Map 3MB of kernel space (0x00100000 to 0x00400000)
    // This is 768 pages (3MB / 4KB)
    
    for (uint32_t addr = 0x00100000; addr < 0x00400000; addr += PAGE_SIZE) {
        paging_map_page(pd, addr, addr, PAGE_PRESENT | PAGE_WRITE);
    }
    
    // Also identity map first 1MB (for safety, some BIOS stuff)
    for (uint32_t addr = 0; addr < 0x00100000; addr += PAGE_SIZE) {
        paging_map_page(pd, addr, addr, PAGE_PRESENT | PAGE_WRITE);
    }
}

// Map a virtual page to physical page
void paging_map_page(page_directory_t* pd, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    // Extract indices from virtual address
    uint32_t pd_index = virtual_addr >> 22;              // Top 10 bits
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;    // Middle 10 bits
    
    // Get page table (or create if doesn't exist)
    if (!(pd->entries[pd_index] & PAGE_PRESENT)) {
        // Need to create page table
        page_table_t* pt = alloc_page_table();
        if (!pt) {
            terminal_writestring("ERROR: Out of page tables!\n");
            return;
        }
        
        // Set page directory entry to point to this page table
        pd->entries[pd_index] = ((uint32_t)pt) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    // Get the page table
    page_table_t* pt = (page_table_t*)(pd->entries[pd_index] & 0xFFFFF000);
    
    // Set page table entry
    pt->entries[pt_index] = (physical_addr & 0xFFFFF000) | flags;
}

// Create new address space (for a new task)
page_directory_t* paging_create_address_space(void) {
    page_directory_t* pd = (page_directory_t*)kmalloc(sizeof(page_directory_t));
    memset(pd, 0, sizeof(page_directory_t));
    
    // Copy kernel mappings (so task can access kernel code)
    paging_identity_map_kernel(pd);
    
    // Map VGA memory
    paging_map_page(pd, 0xB8000, 0xB8000, PAGE_PRESENT | PAGE_WRITE);
    
    return pd;
}

// Destroy address space (when task exits)
void paging_destroy_address_space(page_directory_t* pd) {
    if (pd == kernel_directory) {
        return;  // Don't destroy kernel directory!
    }
    
    // TODO: Free page tables (for now, just free the directory)
    kfree(pd);
}

// Switch to different page directory (task switch)
void paging_switch_directory(page_directory_t* pd) {
    current_directory = pd;
    
    // Load CR3 with physical address of page directory
    __asm__ volatile("mov %0, %%cr3" : : "r"(pd));
}

// Get physical address from virtual address
uint32_t paging_get_physical_address(page_directory_t* pd, uint32_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t offset = virtual_addr & 0xFFF;
    
    if (!(pd->entries[pd_index] & PAGE_PRESENT)) {
        return 0;  // Page table not present
    }
    
    page_table_t* pt = (page_table_t*)(pd->entries[pd_index] & 0xFFFFF000);
    
    if (!(pt->entries[pt_index] & PAGE_PRESENT)) {
        return 0;  // Page not present
    }
    
    uint32_t physical_page = pt->entries[pt_index] & 0xFFFFF000;
    return physical_page | offset;
}

// Get current page directory
page_directory_t* paging_get_current_directory(void) {
    return current_directory;
}