#include <memory.h>
#include <stdint.h>
#include <logger.h>
#include <stddef.h>
#include <isr.h>
#include <panic.h>
#include <string.h>
#include <frame_buffer.h>
#include <io.h>

extern void load_page_directory(uint32_t);
extern void enable_paging();

#define PAGE_SIZE 4096

#define KERNEL_START 0x00100000
#define KERNEL_END   0x01000000
#define PAGE_TABLE_AREA_START 0x00500000
#define PAGE_TABLE_AREA_END 0x05000000
#define PAGE_FRAME_ALLOCATOR_AREA_START 0x00900000
#define PAGE_FRAME_ALLOCATOR_AREA_END 0x0F000000
#define HEAP_ALLOCATOR_AREA_START 0x0F000000
#define HEAP_ALLOCATOR_AREA_END 0x11000000

#define BIT_SIZE_OF_UINT32 32

void init_page_frame_allocator();
void init_page_table_allocator();
void* allocate_page_frame(uint32_t);
void identity_map_range(uint32_t*, uint32_t, uint32_t);
void map_page(uint32_t*, uint32_t, uint32_t);
void page_fault_handler(context_t*);
void init_heap_allocator();

#define BITMAP_SIZE (PAGE_FRAME_ALLOCATOR_AREA_END - PAGE_FRAME_ALLOCATOR_AREA_START) / 4096 / 32  // calculate the correct sized bitmap
uint32_t bitmap[BITMAP_SIZE];

uint32_t next_page_table;

uint32_t heap_current;
uint32_t heap_mapped;

void double_fault_handler(context_t* context){
    panic("Double fault!");
}

int liballoc_free(void* ptr, int m){
    return 0;
}
void init_paging(void) {
    logf("%x\n",BITMAP_SIZE);

    /* Register page fault handler */
    register_interrupt_handler(14, page_fault_handler);
    register_interrupt_handler(8, double_fault_handler);
    /* Clear the page directory by marking not present */
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }
 
    /* Put initial page tables in page directory */
    page_directory[0] = ((uint32_t) first_page_table) | 3;  // supervisor, r/w, present
    page_directory[1] = ((uint32_t) second_page_table) | 3;

    /* Identity map the kernel and page table reserved areas
       We will need to be able to access them later */
    identity_map_range(page_directory, KERNEL_START, KERNEL_END);
    identity_map_range(page_directory, PAGE_TABLE_AREA_START, PAGE_TABLE_AREA_END);

    init_page_frame_allocator();
    init_page_table_allocator();
    init_heap_allocator();

    /* Identity map the VGA buffer so that we can still write to
       the VGA terminal without immediately page faulting */
    map_page(page_directory, 0xb8000, 0xb8000);

    /* Load the page directory and initalize paging */
    load_page_directory((uint32_t) page_directory);
    enable_paging();


    /* TODO: gotta fix this bug, I don't know why it page faults */
    // uint32_t* random_page = allocate_page_frame(0x00900000);
}

void init_page_frame_allocator() {
    /* Clear the bitmap (all pages are unmapped) */
    memset(bitmap, 0, sizeof(bitmap));

    /* Make sure the allocator starts and ends at a page aligned address */
    if (PAGE_FRAME_ALLOCATOR_AREA_START % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not start at a page aligned address!");
    }
    if (PAGE_FRAME_ALLOCATOR_AREA_END % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not end at a page aligned address!");
    }
}

/* Initialize the allocator that takes pages from the
   reserved page table page region */
void init_page_table_allocator() {
    /* This is a watermark allocator which works for now */
    // TODO: change this from a watermark allocator later
    
    /* Make sure that the allocator starts and ends at a page aligned address */
    if (PAGE_TABLE_AREA_START % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not start at a page aligned address!");
    }
    if (PAGE_TABLE_AREA_END % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not end at a page aligned address!");
    }
    next_page_table = PAGE_TABLE_AREA_START;
}

void* allocate_page_frame(uint32_t virtual_address) {
    /* Make sure the address is page aligned */
    if (virtual_address % PAGE_SIZE != 0) {
        panic("Trying to allocate page frame at unaligned address!");
    }

    /* Find a free page using the bitmap */
    int page_index = -1;
    for (int i = 0; i < BITMAP_SIZE; i++) {
        logf("Looking at bitmap %u: %x\n", i, bitmap[i]);
        if (bitmap[i] != 0xFFFFFFFF) {
            /* There is an available page, scan the bits */
            for (int j = 0; j < BIT_SIZE_OF_UINT32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    page_index = i * BIT_SIZE_OF_UINT32 + j;
                    bitmap[i] |= (1 << j);
                    break;
                }
            }
            break;
        }
    }

    if (page_index == -1) {
        /* There are no free pages left */
        panic("Allocator has run out of space!");
    }

    const uint32_t physical_address = PAGE_FRAME_ALLOCATOR_AREA_START + (PAGE_SIZE * page_index);
    map_page(page_directory, virtual_address, physical_address);

    return (uint32_t*) virtual_address;
}

void* allocate_page_table() {
    /* Check to make sure we still have more preallocated page tables */
    if (next_page_table >= PAGE_TABLE_AREA_END) {
        panic("Page table allocator has run out of space!");
    }

    uint32_t* page_table = (uint32_t*) next_page_table;
    memset(page_table, 0, PAGE_SIZE);
    next_page_table += PAGE_SIZE;
    return page_table;
}

/* Identity maps the range defined by [start_address, end_address) */
void identity_map_range(uint32_t* kernel_page_directory, uint32_t start_address, uint32_t end_address) {
    for (uint32_t addr = start_address; addr < end_address; addr += PAGE_SIZE) {
        map_page(kernel_page_directory, addr, addr);
    }
}

void map_page(uint32_t* kernel_page_directory, uint32_t virtual_address, uint32_t physical_address) {
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_directory_entry = *(kernel_page_directory + page_directory_index);

    if (!(page_directory_entry & 0x1)) {
        /* Page table is not present so we need to grab a new page table
           and put it into the correct spot */
        logf("Page table not present for virtual address: %x\n", virtual_address);
        logf("Adding a new page table at page_directory_index: %x\n", page_directory_index);

        uint32_t* new_page_table = allocate_page_table();
        logf("New page table allocated at v_addr: %x\n", new_page_table);
        *(kernel_page_directory + page_directory_index) = ((uint32_t) new_page_table) | 3; 

        /* Need to set the page_direcotyr_entry to the new page table and continue */
        page_directory_entry = *(kernel_page_directory + page_directory_index) ;
    }

    uint32_t* page_table = (uint32_t *) (page_directory_entry & 0xFFFFF000);
    uint32_t page_table_index = virtual_address >> 12 & 0x03FF;

    page_table[page_table_index] = physical_address | 3; // supervisor, r/w, present
}

void page_fault_handler(context_t* context)
{
    asm volatile ("sti");
    disable_paging();
    printk("\nPage fault! Unloaded page tables and disabled paging\n");
    panic("page fault interrupt");
}

int liballoc_lock()
{
	return 0;
}

int liballoc_unlock()
{
	return 0;
}

void disable_paging(void)
{
    asm volatile(
        "mov  %%cr0, %%eax        \n\t"
        "btr  $31, %%eax          \n\t"   // reset (clear) bit 31
        "mov  %%eax, %%cr0        \n\t"
        ::
        : "eax", "memory"
     );
}

void* liballoc_alloc(int size)
{
	return heap_allocate((uintptr_t) size);
}

/****************************************************
 * Heap Allocator:
 * - A watermark heap alllocator for the time being
 * TODO: change this from a watermark allocator later
 ****************************************************/

void init_heap_allocator() {
    if (HEAP_ALLOCATOR_AREA_START % 4096 != 0) {
        panic("Heap allocator does not start at a page aligned address!");
    }

    if (HEAP_ALLOCATOR_AREA_END % 4096 != 0) {
        panic("Heap allocator does not end at a page aligned address!");
    }

    /* Both the current location of the heap and the mapped pointer
       should start at the same location */
    heap_current = HEAP_ALLOCATOR_AREA_START;
    heap_mapped = HEAP_ALLOCATOR_AREA_START;
}

void* heap_allocate(size_t size) {
    logf("Heap allocation of size %u bytes at address %x\n", size, heap_current);
    if (heap_current + size > HEAP_ALLOCATOR_AREA_END) {
        panic("Heap is overflowing! Cannot allocate!");
    }

    /* Allocate new pages for the heap on demand */
    while (heap_current + size > heap_mapped) {
        allocate_page_frame(heap_mapped);
        heap_mapped += PAGE_SIZE;
    }

    /* Clear the memory before allocating */
    memset((void*)heap_current, 0, size);

    uint32_t ret = heap_current;
    heap_current += size;

    return (void*) ret;
}

unsigned char lowmem(){
    outb(0x70, 0x30);
    return inb(0x71);
}

unsigned char himem(){
    outb(0x70, 0x31);
    return inb(0x71);
}

//get CMOS memory count
unsigned short memorycount(){
    return lowmem() | himem() << 8;
}
