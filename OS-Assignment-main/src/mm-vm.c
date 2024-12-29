// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt.rg_start >= rg_elmt.rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt.rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = &rg_elmt;
  // printf("enlist from %d to %d\n",rg_elmt.rg_start, rg_elmt.rg_end);
  return 0;
}
int enlist_vm_freerg_list_by_vmaid(struct mm_struct *mm, struct vm_rg_struct rg_elmt, int vmaid)
{
#ifdef MM_PAGING_HEAP_GODOWN
#else
#endif
  if (rg_elmt.rg_start >= rg_elmt.rg_end)
    return -1;
  struct vm_area_struct *cur_vma = get_vma_by_num(mm, vmaid);
  struct vm_rg_struct *rg_node = cur_vma->vm_freerg_list;
  int added = 0;
  while (rg_node)
  {
    if (rg_node->rg_end == rg_elmt.rg_start)
    {
      rg_node->rg_end = rg_elmt.rg_end;
      added = 1;
    }
    else if (rg_node->rg_start == rg_elmt.rg_end)
    {
      rg_node->rg_start = rg_elmt.rg_start;
      added = 1;
    }
    rg_node = rg_node->rg_next;
  }
  if (added == 0)
  {
    if (cur_vma->vm_freerg_list)
      rg_elmt.rg_next = rg_node;
    cur_vma->vm_freerg_list = &rg_elmt;
  }
  return 0;
}
/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = 0;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    vmait++;
    pvma = pvma->vm_next;
  }

  return pvma;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr) // gs
{
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;
  /* TODO: commit the vmaid */

  rgnode.vmaid = vmaid;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;

    *alloc_addr = rgnode.rg_start;
    // printf("vm allocate from %d to %d \n",rgnode.rg_start,rgnode.rg_end);
    return 0;
  }

  /* TODO: get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */

  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int inc_limit_ret = -1;
  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  int old_sbrk = cur_vma->sbrk;
  /* TODO INCREASE THE LIMIT
   * inc_vma_limit(caller, vmaid, inc_sz)
   */
  if (inc_vma_limit(caller, vmaid, size, &inc_limit_ret) < 0)
  {
    printf("increase vma_limit failed \n");
    return -1;
  }
  /* TODO: commit the limit increment */
#ifdef MM_PAGING_HEAP_GODOWN
  if (cur_vma->vm_id == 1)
  {
    rgnode.rg_start = old_sbrk;
    rgnode.rg_end = old_sbrk - size;
    {
      struct vm_rg_struct *free_rg_node = init_vm_rg(rgnode.rg_start, rgnode.rg_end, vmaid);
      // if(enlist_vm_rg_node(&cur_vma->vm_freerg_list, free_rg_node) < 0){
      // }
    }
    cur_vma->sbrk = rgnode.rg_end;
    // printf("enlist from %d to %d\n",cur_vma->vm_freerg_list->rg_start, cur_vma->vm_freerg_list->rg_end);
  }
  else
  {
    rgnode.rg_start = old_sbrk;
    rgnode.rg_end = old_sbrk + size;
    {
      struct vm_rg_struct *free_rg_node = init_vm_rg(rgnode.rg_start, rgnode.rg_end, vmaid);
      // if(enlist_vm_rg_node(&cur_vma->vm_freerg_list, free_rg_node) < 0){
      //   printf("enlist free space in alloc failed \n");
      // }
      // else{
      //       printf("enlist from %d to %d\n",cur_vma->vm_freerg_list->rg_start, cur_vma->vm_freerg_list->rg_end);
      // }
    }
    cur_vma->sbrk = rgnode.rg_end;
  }
#else
  rgnode.rg_start = old_sbrk;
  rgnode.rg_end = old_sbrk + size;
  if (cur_vma->vm_end > rgnode.rg_end)
  {
    struct vm_rg_struct *free_rg_node = init_vm_rg(rgnode.rg_start, rgnode.rg_end, vmaid);
    if (enlist_vm_rg_node(&cur_vma->vm_freerg_list, free_rg_node) < 0)
    {
      printf("enlist free space in alloc failed \n");
    }
    // else{
    //   printf("enlist from %d to %d\n",cur_vma->vm_freerg_list->rg_start, cur_vma->vm_freerg_list->rg_end);
    // }
  }
  cur_vma->sbrk = rgnode.rg_end;
#endif

  printf("vm allocate from %d to %d \n", rgnode.rg_start, rgnode.rg_end);
  /* TODO: commit the allocation address
  // *alloc_addr = ...
  */

  caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
  caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
  caller->mm->symrgtbl[rgid].vmaid = vmaid;
  *alloc_addr = old_sbrk;
  return 0;
}
//======================================================ADD SOME FUNCTION===================================//
#define MAX_ARRAY_SIZE 100
struct vm_rg_struct *vm_rg_push_back(struct vm_rg_struct *rglist, int start, int end, int vmaid)
{ // them node vao cuoi danh sach
  struct vm_rg_struct *newNode = init_vm_rg(start, end, vmaid);
  if (rglist == NULL)
    return newNode;
  struct vm_rg_struct *curr = rglist;
  while (curr->rg_next != NULL)
  {
    curr = curr->rg_next;
  }
  curr->rg_next = newNode;
  return rglist;
}

int collectIntervals(struct vm_rg_struct *rglist, int intervals[][2], int size)
{ // convert linklist to array for support later task
  int cnt = 0;
  struct vm_rg_struct *curr = rglist;
  while (curr != NULL && cnt < size)
  {
    intervals[cnt][0] = curr->rg_start;
    intervals[cnt][1] = curr->rg_end;
    ++cnt;
    curr = curr->rg_next;
  }
  return cnt;
}

int compare(const void *a, const void *b)
{ // ep kieu theo yeu cau cua qsort
  int *interval1 = (int *)a;
  int *interval2 = (int *)b;
  if (interval1[0] != interval2[0])
    return (interval1[0] - interval2[0]) > 0;
  return (interval1[1] - interval2[1]) > 0;
}
#ifdef MM_PAGING_HEAP_GODOWN
int compare_HEAP_GODOWN(const void *a, const void *b)
{
  int *interval1 = (int *)a;
  int *interval2 = (int *)b;
  if (interval1[0] != interval2[0])
    return (interval1[0] - interval2[0]) < 0;
  return (interval1[1] - interval2[1]) < 0;
}
#endif

struct vm_rg_struct *mergeIntervals(struct vm_rg_struct *rglist)
{ // merge all intervals
  if (rglist == NULL)
    return NULL;
  int vmaid = rglist->vmaid;

  // convert linklist to array
  int intervals[MAX_ARRAY_SIZE][2]; // max size 100
  int cnt = collectIntervals(rglist, intervals, MAX_ARRAY_SIZE);

  // sort the intervals to a..b->b..c->c..d->...
#ifdef MM_PAGING_HEAP_GODOWN
  if (vmaid == 0)
    qsort(intervals, cnt, sizeof(intervals[0]), compare);
  else
    qsort(intervals, cnt, sizeof(intervals[0]), compare_HEAP_GODOWN);
#else
  qsort(intervals, cnt, sizeof(intervals[0]), compare);
#endif

  // create new linklist to store all intervals when merging
  struct vm_rg_struct *newHead = NULL;
  int start = intervals[0][0], end = intervals[0][1];
  // begin merging
#ifdef MM_PAGING_HEAP_GODOWN
  if (vmaid == 0)
    for (int i = 1; i < cnt; ++i)
    { // RAM
      if (intervals[i][0] <= end)
      {
        // merge intervals
        end = intervals[i][1];
      }
      else
      {
        // if cant merge, push_back it to linklist
        newHead = vm_rg_push_back(newHead, start, end, vmaid);
        start = intervals[i][0];
        end = intervals[i][1];
      }
    }
  else
    for (int i = 1; i < cnt; ++i)
    { // HEAP GO DOWN
      if (intervals[i][0] >= end)
      {
        // merge intervals
        end = intervals[i][1];
      }
      else
      {
        // if cant merge, push_back it to linklist
        newHead = vm_rg_push_back(newHead, start, end, vmaid);
        start = intervals[i][0];
        end = intervals[i][1];
      }
    }
#else
  for (int i = 1; i < cnt; ++i)
  {
    if (intervals[i][0] <= end)
    {
      // merge intervals
      end = intervals[i][1];
    }
    else
    {
      // if cant merge, push_back it to linklist
      newHead = vm_rg_push_back(newHead, start, end, vmaid);
      start = intervals[i][0];
      end = intervals[i][1];
    }
  }
#endif
  // Add last interval
  newHead = vm_rg_push_back(newHead, start, end, vmaid);
  return newHead;
}

// print the list for debug
void printList(struct vm_rg_struct *rglist)
{
  struct vm_rg_struct *curr = rglist;
  int c = 0;
  while (curr != NULL)
  {
    printf("%d..%d", curr->rg_start, curr->rg_end);
    if (curr->rg_next != NULL)
      printf(" => ");
    curr = curr->rg_next;
    ++c;
  }
  // c-=1;
  printf("\n");
  printf("free list have %d node\n", c);
}

//==================================================END ADD SOME FUNCTION===================================//
/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int rgid) // MD
{
  struct vm_rg_struct rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later
  // rgnode.vmaid = 0;  //dummy initialization
  // rgnode.vmaid = 1;  //dummy initialization
  if (!caller)
    return -1;

  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;
  /* TODO: Manage the collect freed region to freerg_list */
  rgnode = caller->mm->symrgtbl[rgid];
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, rgnode.vmaid);
  struct vm_rg_struct *vm_freerg_list = cur_vma->vm_freerg_list;
  // DEBUG
  //  printf("=============== Before merge ===============\n");
  //  printList(vm_freerg_list);
  vm_freerg_list = vm_rg_push_back(vm_freerg_list, rgnode.rg_start, rgnode.rg_end, rgnode.vmaid);
  vm_freerg_list = mergeIntervals(vm_freerg_list);
  cur_vma->vm_freerg_list = vm_freerg_list;
  caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].rg_end = 0;
  // DEBUG
  //  printf("=============== After merge ===============\n");
  //  printList(vm_freerg_list);
  return 0;
}

/*pgalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int pgaddr(struct pcb_t *proc, int rgid)
{
  printf("rgid:%d, start %d => end %d\n", rgid, proc->mm->symrgtbl[rgid].rg_start,
         proc->mm->symrgtbl[rgid].rg_end);
  if (1)
  {
    int vmaid = proc->mm->symrgtbl[rgid].vmaid;
    struct vm_area_struct *cur_vma = get_vma_by_num(proc->mm, 0);
    printf("========== free list: vmaid 0 ==========\n");
    printList(cur_vma->vm_freerg_list);
    struct vm_area_struct *cur_vma1 = get_vma_by_num(proc->mm, 1);
    printf("========== free list: vmaid 1 ==========\n");
    printList(cur_vma1->vm_freerg_list);
  }
  return 0;
}
int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 0 */
  // printf("enlist from %d to %d\n",proc->mm->mmap->vm_freerg_list->rg_start, proc->mm->mmap->vm_freerg_list->rg_end);
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*pgmalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify vaiable in symbole table)
 */
int pgmalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 1 */
  return __alloc(proc, 1, reg_index, size, &addr);
}

/*pgfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  return __free(proc, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PTE_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn;
    // int vicfpn;
    // uint32_t vicpte;

    int tgtfpn = PAGING_PTE_SWP(pte); // the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    if (find_victim_page(caller->mm, &vicpgn) != 0)
      return -1;
    /* Get free frame in MEMSWP */
    if (MEMPHY_get_freefp(caller->active_mswp, &swpfpn) != 0)
      return -1;
    /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
    /* Copy victim frame to swap */
    //__swap_cp_page();x
    /* Copy target frame from swap to mem */
    //__swap_cp_page();
    __swap_cp_page(caller->mram, PAGING_FPN(caller->mm->pgd[vicpgn]), caller->active_mswp, swpfpn);
    __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, PAGING_FPN(caller->mm->pgd[vicpgn]));
    /* Update page table */
    // pte_set_swap() &mm->pgd;
    pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);
    /* Update its online status of the target page */
    // pte_set_fpn() & mm->pgd[pgn];
    pte_set_fpn(&pte, tgtfpn);

    enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
  }

  *fpn = PAGING_PTE_FPN(pte);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram, phyaddr, data);

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_write(caller->mram, phyaddr, value);

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid); //&mm->symrgtbl[rgid]
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  // xu li read region chua duoc alloc hoac da duoc free
  if (currg->rg_start == currg->rg_end)
  {
    printf("===== Trying to read region that is (not alloc yet) or (freed) =====\n");
    return -1;
  }
  // xu li read voi offset vuot qua pham vi alloc
  int region_size = currg->rg_end - currg->rg_start;
  if (region_size < 0)
    region_size = 0 - region_size;
  if (offset >= region_size)
  {
    printf("===== Read with offset out of bound =====\n");
    return -1;
  }

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*pgwrite - PAGING-based read a region memory */
int pgread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t destination)
{
  BYTE data;
  int val = __read(proc, source, offset, &data);

  destination = (uint32_t)data;
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int rgid, int offset, BYTE value)
{ // process  //destination
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL)
  {
    // printf("===== Write to region that is (not alloc yet) or (freed) =====\n");
    return -1;
  } /* Invalid memory identify */

  // xu li write vao region chua duoc alloc hoac da duoc free
  if (currg->rg_start == currg->rg_end)
  {
    printf("===== Write to region that is (not alloc yet) or (freed) =====\n");
    return -1;
  }
  // xu li write voi offset vuot qua pham vi alloc
  int region_size = currg->rg_end - currg->rg_start;
  if (region_size < 0)
    region_size = 0 - region_size;
  if (offset >= region_size)
  {
    printf("===== Write with offset out of bound =====\n");
    return -1;
  }

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*pgwrite - PAGING-based write a region memory */
int pgwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  struct vm_area_struct *cur_vma = get_vma_by_num(proc->mm, 1);
  print_pgtbl(proc, 0, -1); // print max TBL
  print_pgtbl(proc, cur_vma->vm_end, cur_vma->vm_start + PAGING_PAGESZ);
#endif
  // printf("destination = %d\n", destination);
  // MEMPHY_dump(proc->mram);

#endif

  return __write(proc, destination, offset, data);
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;

  for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte = caller->mm->pgd[pagenum];

    if (!PAGING_PTE_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    }
    else
    {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);
    }
  }

  return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz) // MD
{
  struct vm_rg_struct *newrg;
  /* TODO retrive current vm_area to obtain newrg, current comment out due to compiler redundant warning*/
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  newrg = malloc(sizeof(struct vm_rg_struct));

//* TODO: update the newrg boundary
#ifdef MM_PAGING_HEAP_GODOWN
  if (vmaid == 1)
  {
    newrg->rg_start = cur_vma->sbrk;
    newrg->rg_end = cur_vma->sbrk - size;
    newrg->vmaid = vmaid;
    newrg->rg_next = NULL;
  }
  else
  {
    newrg->rg_start = cur_vma->sbrk;
    newrg->rg_end = cur_vma->sbrk + size;
    newrg->vmaid = vmaid;  // vm_region nay thuoc ve vm_area hien tai (cur_vma)
    newrg->rg_next = NULL; //: D
  }
#else
  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = cur_vma->sbrk + size;
  newrg->vmaid = vmaid;  // vm_region nay thuoc ve vm_area hien tai (cur_vma)
  newrg->rg_next = NULL; //: D
#endif
  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend) // gs
{
  struct vm_area_struct *vma = caller->mm->mmap; // head of vm_area
  /* TODO validate the planned memory area is not overlapped */
  while (vma)
  {
    if (vma->vm_id != vmaid)
    {
#ifdef MM_PAGING_HEAP_GODOWN
      if (vmaid == 1 && vmaend < vma->vm_end)
      {
        printf("area rg %d-%d, vma rg %d - %d\n", vmastart, vmaend, vma->vm_start, vma->vm_end);
        return -1;
      }
      else if (vmaid == 0 && vmaend > vma->vm_end)
      {
        return -1;
      }
#else
      if (vmaid == 1 && vmaend > (512 * PAGE_SIZE))
      {
        return -1;
      }
      else if (vmaid == 0 && vmaend > vma->vm_start)
      {
        return -1;
      }
#endif
    }
    vma = vma->vm_next;
  }
  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *@inc_limit_ret: increment limit return
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz, int *inc_limit_ret) // MD
{
  if (!caller || !inc_limit_ret)
  {
    printf("NULL pointer parameter");
    return -1;
  }

  struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz); // size tang len bang boi cua kich thuoc 1 trang ((inc_sz + 255)/256)*256
  int incnumpage = inc_amt / PAGING_PAGESZ;  // number of pages need for this increasing
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  int old_end = cur_vma->vm_end;

  /*Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
  {
    printf("Overlaped between vm_area regions!!!\n");
    free(newrg);
    return -1; /*Overlap and failed allocation */
  }

  /* TODO: Obtain the new vm area based on vmaid */
#ifdef MM_PAGING_HEAP_GODOWN
  if (cur_vma->vm_id == 1)
  {
    if (area->rg_end < cur_vma->vm_end)
    {
      cur_vma->vm_end -= inc_amt;
    }
  }
  else
  {
    if (area->rg_end > cur_vma->vm_end)
    {
      cur_vma->vm_end += inc_amt;
    }
  }
#else
  if (area->rg_end > cur_vma->vm_end)
  {
    cur_vma->vm_end += inc_amt;
  }
#endif
  *inc_limit_ret = cur_vma->vm_end;

  if (vm_map_ram(caller, area->rg_start, area->rg_end,
                 old_end, incnumpage, newrg) < 0)
  {
    printf("Failed in mapping memory to RAM\n");
    free(newrg);
    return -1; /* Map the memory to MEMRAM */
  }
  // printf("New vm region with ID %d, range form %d to %d \n", vmaid, old_end, cur_vma->vm_end);
  // printf("Increased vm_area %d \n", vmaid);
  return *inc_limit_ret;
  // return 0;
}

/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn) // SH
{
  struct pgn_t *pg = mm->fifo_pgn;
  if (!pg)
    return -1;
  /* TODO: Implement the theorical mechanism to find the victim page */
  *retpgn = pg->pgn;
  mm->fifo_pgn = mm->fifo_pgn->pg_next;
  pg->pg_next = NULL;
  if (pg->pg_next == NULL)
  {
    *retpgn = pg->pgn;
    free(pg);
    return 0;
  }
  struct pgn_t *tmp = pg;
  while (tmp->pg_next->pg_next != NULL)
  {
    tmp = tmp->pg_next;
  }
  *retpgn = tmp->pg_next->pgn;
  free(tmp->pg_next);
  tmp->pg_next = NULL;
  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
  {
    // printf("free space null\n");
    return -1;
  }
  // else  printf("free space not null\n");

  // printf("enlist vmaid %d from %d to %d\n",vmaid,cur_vma->vm_freerg_list->rg_start, cur_vma->vm_freerg_list->rg_end);
  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* Traverse on list of free vm region to find a fit space */
  while (rgit != NULL && rgit->vmaid == vmaid)
  {
    // printf("free list: %d - %d \n", rgit->rg_start,rgit->rg_end);
    if (rgit->rg_start + size <= rgit->rg_end)
    { /* Current region has enough space */
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;

      /* Update left space in chosen region */
      if (rgit->rg_start + size < rgit->rg_end)
      {
        rgit->rg_start = rgit->rg_start + size;
      }
      else
      { /*Use up all space, remove current node */
        /*Clone next rg node */
        struct vm_rg_struct *nextrg = rgit->rg_next;

        /*Cloning */
        if (nextrg != NULL)
        {
          rgit->rg_start = nextrg->rg_start;
          rgit->rg_end = nextrg->rg_end;

          rgit->rg_next = nextrg->rg_next;

          free(nextrg);
        }
        else
        {                                /*End of free list */
          rgit->rg_start = rgit->rg_end; // dummy, size 0 region
          rgit->rg_next = NULL;
        }
      }
      break;
    }
    else
    {
      rgit = rgit->rg_next; // Traverse next rg
    }
  }

  if (newrg->rg_start == -1) // new region not found
    return -1;

  return 0;
}

// #endif