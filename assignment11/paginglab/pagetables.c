#include <assert.h>
#include <errno.h>  // for EINVAL, ENOMEM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // memset

#include "paging.h"

/*
 * This assignment is a practical page-table exercise for the CASP[1]
 * course at ETH Zurich.
 * We simulate x86_64 paging.
 *
 * [1] Computer architecture and systems programming
 */

// Generate some sequences of virtual page addresses
// This simplifies the code a little
#define VIRTADDR2(i) (0x12345678UL + 4 * i * (BASE_PAGE_SIZE))
//                                   \_ Map every fourth page
// Register cr3 points to the page map level 4 (chapter18, slide 57)
pml4e_t *cr3;

// --------------------------------------------------
// Helper functions

/*
 * \brief Parse a virtual address
 *
 * Extract virtual page number and virtual page offset
 * from given virtual address for 4K pages.
 *
 * Virtual address:
 * -----------------------------------
 * | vpn1 | vpn2 | vpn3 | vpn4 | vpo |
 * -----------------------------------
 *    9      9      9      9      12     <- size in bits
 *
 * \param va The virtual address to be parsed
 * \param vpn1 Index for level 1 page table
 * \param vpn2 Index for level 2 page table
 * \param vpn3 Index for level 3 page table
 * \param vpn4 Index for level 4 page table
 * \param vpo Offset within page
 */
void parse_virt_addr(vaddr_t va, uint32_t *vpn1, uint32_t *vpn2,
                     uint32_t *vpn3, uint32_t *vpn4, uint32_t *vpo) {
    *vpo = va & 0xFFF;
    va >>= 12;
    *vpn4 = va & 0x1FF;
    va >>= 9;
    *vpn3 = va & 0x1FF;
    va >>= 9;
    *vpn2 = va & 0x1FF;
    va >>= 9;
    *vpn1 = va & 0x1FF;
}

pml4e_t *get_pml4e(uint32_t vpn1) {
    return &cr3[vpn1];
}

pdpe_t *get_pdpe(pml4e_t pml4e, uint32_t vpn2) {
    pdpe_t *pdpt = (pdpe_t *)((uint64_t)pml4e.d.base_addr << BASE_PAGE_BITS);
    return pdpt + vpn2;
}

pde_t *get_pde(pdpe_t pdpe, uint32_t vpn3) {
    pde_t *pde = (pde_t *)((uint64_t)pdpe.d.base_addr << BASE_PAGE_BITS);
    return pde + vpn3;
}

pte_t *get_pte(pde_t pde, uint32_t vpn4) {
    pte_t *pte = (pte_t *)((uint64_t)pde.d.base_addr << BASE_PAGE_BITS);
    return pte + vpn4;
}

// --------------------------------------------------

void *alloc_table(void) {
    void *m;
    if (posix_memalign(&m, BASE_PAGE_SIZE, BASE_PAGE_SIZE) != 0) {
        exit(EXIT_FAILURE);
    }

    memset(m, 0, BASE_PAGE_SIZE);
    return m;
}

/*
 * \brief set a page table entry to be present and pointing to the given
 * address.
 *
 * \param d the entry to update
 * \param pa the address to write into the entry
 * \param rw read_write flag
 */
void set_directory_entry(struct directory_entry *d, uint64_t pa, uint8_t rw) {
    d->ps = 0;
    d->base_addr = pa >> BASE_PAGE_BITS;
    d->read_write = rw;
    d->present = 1;
}

// --------------------------------------------------
// High level manipulation functions

/*
 * \brief Map a 1G page.
 *
 * \param pa Physical address of page to be mapped
 * \param va Virtual address of page to be mapped
 * \param rw Whether or not to map with write permissions
 */
bool map_huge(paddr_t pa, vaddr_t va, uint8_t rw) {
    uint32_t vpn1, vpn2, garbage;
    parse_virt_addr(va, &vpn1, &vpn2, &garbage, &garbage, &garbage);
    pml4e_t *pml4e = get_pml4e(vpn1);
    if (!pml4e->d.present) {
        pdpe_t *pdpt = alloc_table();
        set_directory_entry(&pml4e->d, (int64_t)pdpt, rw);
    } else {
        pml4e->d.read_write |= rw;
    }

    pdpe_t *pdpe = get_pdpe(*pml4e, vpn2);
    if (pdpe->d.present) {
        // can't map, va is already mapped
        return false;
    }

    pdpe->page.ps = 1;
    pdpe->page.read_write |= rw;
    // we get the 18 most significant bits (i.e. shift out the lower 9 + 9 + 12 = 30 bits)
    // the master solution seems to be wrong here, only getting the 14 most significant bits
    pdpe->page.base_addr = (pa >> (BASE_PAGE_BITS + 2 * PAGE_TABLE_ENTRIES_BITS)) & 0x3FFFF;
    pdpe->page.present = 1;
    return true;
}
/*
 * \brief Map a 2M page.
 *
 * \param pa Physical address of page to be mapped
 * \param va Virtual address of page to be mapped
 * \param rw Whether or not to map with write permissions
 */
bool map_large(paddr_t pa, vaddr_t va, uint8_t rw) {
    uint32_t vpn1, vpn2, vpn3, garbage;
    parse_virt_addr(va, &vpn1, &vpn2, &vpn3, &garbage, &garbage);
    pml4e_t *pml4e = get_pml4e(vpn1);
    if (!pml4e->d.present) {
        pdpe_t *pdpt = alloc_table();
        set_directory_entry(&pml4e->d, (int64_t)pdpt, rw);
    } else {
        pml4e->d.read_write |= rw;
    }

    pdpe_t *pdpe = get_pdpe(*pml4e, vpn2);
    if (pdpe->d.ps) {
        // the table is already a huge one
        return false;
    }

    if (!pdpe->d.present) {
        pde_t *pde = alloc_table();
        set_directory_entry(&pdpe->d, (int64_t)pde, rw);
    } else {
        pdpe->d.read_write |= rw;
    }

    pde_t *pde = get_pde(*pdpe, vpn3);
    if (pde->page.present) {
        // already mapped
        return false;
    }

    pde->page.ps = 1;
    pde->page.read_write |= rw;
    pde->page.base_addr = (pa >> (BASE_PAGE_BITS + PAGE_TABLE_ENTRIES_BITS)) & 0x7FFFFFF;
    pde->page.present = 1;
    return true;
}

/*
 * \brief Map a 4K page.
 *
 * \param pa Physical address of page to be mapped
 * \param va Virtual address of page to be mapped
 * \param rw Whether or not to map with write permissions.
 */
bool map(paddr_t pa, vaddr_t va, uint8_t rw) {
    uint32_t vpn1, vpn2, vpn3, vpn4, garbage;
    parse_virt_addr(va, &vpn1, &vpn2, &vpn3, &vpn4, &garbage);
    pml4e_t *pml4e = get_pml4e(vpn1);
    if (!pml4e->d.present) {
        pdpe_t *pdpt = alloc_table();
        set_directory_entry(&pml4e->d, (int64_t)pdpt, rw);
    } else {
        pml4e->d.read_write |= rw;
    }

    pdpe_t *pdpe = get_pdpe(*pml4e, vpn2);
    if (!pdpe->d.present) {
        pde_t *pde = alloc_table();
        set_directory_entry(&pdpe->d, (int64_t)pde, rw);
    } else if (pdpe->page.ps) {
        // the table is already a huge one
        return false;
    } else {
        pdpe->d.read_write |= rw;
    }

    pde_t *pde = get_pde(*pdpe, vpn3);
    if (!pde->d.present) {
        pte_t *pte = alloc_table();
        set_directory_entry(&pde->d, (int64_t)pte, rw);
    } else if (pde->page.ps) {
        // the table is already a huge one
        return false;
    } else {
        pde->d.read_write |= rw;
    }

    pte_t *pte = get_pte(*pde, vpn4);
    if (pte->page.present) {
        return false;
    }

    pte->page.read_write = rw;
    pte->page.base_addr = (pa >> BASE_PAGE_BITS) & 0xFFFFFFFFF;
    pte->page.present = 1;
    return true;
}

/*
 * \brief Unmap a page.
 *
 * This needs to work for both, regular (4K), large (2M) and huge (1G) pages.
 *
 * \param va Virtual address of the page to unmap.
 */
void unmap(vaddr_t va) {
    uint32_t vpn1, vpn2, vpn3, vpn4, garbage;
    parse_virt_addr(va, &vpn1, &vpn2, &vpn3, &vpn4, &garbage);
    pml4e_t *pml4e = get_pml4e(vpn1);
    if (!pml4e->d.present) {
        return;
    }

    pdpe_t *pdpe = get_pdpe(*pml4e, vpn2);
    if (pdpe->page.ps) {
        pdpe->raw = 0;  // clear huge page
        return;
    }

    if (!pdpe->d.present) {
        return;  // nothing to do
    }

    pde_t *pde = get_pde(*pdpe, vpn3);
    if (pde->page.ps) {
        pde->raw = 0;  // clear large page
        return;
    }

    if (!pde->d.present) {
        return;
    }

    pte_t *pte = get_pte(*pde, vpn4);
    // clear page
    pte->raw = 0;
}

// --------------------------------------------------
// Evaluation

/*
 * \brief Walk the entire page table data structure and free all tables.
 *
 * Free tables at all levels of the page table, but not the pages themselves.
 * This is why we recurse at each level and skip missing entries and
 * huge/large/regular pages.
 */
void free_pagetable(pml4e_t *cr3) {
    for (uint32_t l4 = 0; l4 < PAGE_TABLE_ENTRIES; l4++) {
        pml4e_t *pml4e = cr3 + l4;
        if (!pml4e->d.present) {
            continue;
        }

        pdpe_t *pdpe_base = (pdpe_t *)((uint64_t)(pml4e->d.base_addr) << BASE_PAGE_BITS);
        for (uint32_t l3 = 0; l3 < PAGE_TABLE_ENTRIES; l3++) {
            pdpe_t *pdpe = pdpe_base + l3;
            if (!pdpe->d.present || pdpe->page.ps) {
                continue;
            }

            pde_t *pde_base = (pde_t *)((uint64_t)(pdpe->d.base_addr) << BASE_PAGE_BITS);
            for (uint32_t l2 = 0; l3 < PAGE_TABLE_ENTRIES; l2++) {
                pde_t *pde = pde_base + l2;
                if (!pde->d.present || pde->page.ps) {
                    continue;
                }

                pte_t *pt = (pte_t *)((uint64_t)(pde->d.base_addr) << BASE_PAGE_BITS);
                free(pt);
            }

            free(pde_base);
        }

        free(pdpe_base);
    }

    free(cr3);
}

int main(int argc, char *argv[]) {
    uint64_t i;
    bool r;

    cr3 = alloc_table();

    // map some huge pages
    for (i = 0; i < 8; i++) {
        r = map_huge(0x40000000UL + (i * 1024 * 1024 * 1024),
                     0x1000000000UL - (i * 1024 * 1024 * 1024), 0);
        assert(r);
    }
    // check that we can't overwrite huge pages with huge pages
    for (i = 0; i < 8; i++) {
        r = map_huge(0x40000000UL + (i * 1024 * 1024 * 1024),
                     0x1000000000UL - (i * 1024 * 1024 * 1024), 0);
        assert(!r);
    }

    // check that we can't overwrite huge pages with large pages
    for (i = 0; i < 8; i++) {
        r = map_large(0x40000000UL + (i * 1024 * 1024 * 1024),
                      0x1000000000UL - (i * 1024 * 1024 * 1024), 0);
        assert(!r);
    }

    // check that we can't overwrite huge pages with large pages
    for (i = 0; i < 8; i++) {
        r = map(0x40000000UL + (i * 1024 * 1024 * 1024),
                0x1000000000UL - (i * 1024 * 1024 * 1024), 0);
        assert(!r);
    }
    dump_pagetable(cr3);

    // Unmap half of them again
    for (i = 0; i < 4; i++) {
        unmap(0x1000000000UL - (i * 1024 * 1024 * 1024));
    }
    dump_pagetable(cr3);

    // Map some large pages
    for (i = 0; i < 8; i++) {
        r = map_large(0x00F0F000 + (i * 2 * 1024 * 1024),
                      0x2000000000UL - (i * 1024 * 1024 * 1024), 1);
        assert(r);
    }
    // check that we can't map huge pages when large page exists in region
    for (i = 0; i < 8; i++) {
        r = map_huge(0x40000000UL + (i * 1024 * 1024 * 1024),
                     0x2000000000UL - (i * 1024 * 1024 * 1024), 1);
        assert(!r);
    }

    // check that we can't overwrite large entries with 4K pages
    for (i = 0; i < 8; i++) {
        r = map(0x00F0F000 + (i * 4 * 1024),
                0x2000000000UL - (i * 1024 * 1024 * 1024), 1);
        assert(!r);
    }
    dump_pagetable(cr3);

    // Unmap half of them again
    for (i = 0; i < 4; i++) {
        unmap(0x2000000000UL - (i * 1024 * 1024 * 1024));
    }
    dump_pagetable(cr3);

    // Map the same physical page several times to virtual address space.
    // Some of which are read-only
    for (i = 0; i < 1024; i++) {
        r = map(0x0AFFE000, VIRTADDR2(i), (i % 256));
        assert(r);
    }

    // try to map large pages on region that contains small pages
    for (i = 0; i < 16; i++) {
        r = map_large(0xF0F000, VIRTADDR2(i), 1);
        assert(!r);
    }
    dump_pagetable(cr3);

    // Unmap most of them again
    for (i = 0; i < 1024; i++) {
        if ((i % 64)) {
            unmap(VIRTADDR2(i));
        }
    }

    // Cleanup
    free_pagetable(cr3);

    return 0;
}
