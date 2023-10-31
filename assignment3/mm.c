#define MOIRA                           1
#   define MOIRA_M2                     1
#   define MOIRA_M3                     1


#define GENJI                           1
#   define GENJI_ASSERT_HEAP_LISTS      0
#   define GENJI_ASSERT_BLOCKS          0

// EXTRACT
#   define  EXTRACT_ASSERT_HEAP_LISTS   0
#   define  EXTRACT_ASSERT_VALID        0
// INSERT
#   define  INSERT_ASSERT_HEAP_LISTS    0
#   define  INSERT_ASSERT_VALID         0

// unlink
#   define  UNLINK_ASSERT_HEAP_LISTS    0

#define GDBPASSERT_PRINT                0


#define ASSERT                          0
#define LOG                             0
#   define LOG_LL                       0
#   define LOG_MOIRA                    0
#      define LOG_MOIRA_DUMP_HEAP       0
#      define LOG_MOIRA_MERGES          0
#         define LOG_MOIRA_M2          	0
#         define LOG_MOIRA_M3          	0
#   define LOG_NO_MEM                   0

#if LOG_MOIRA
FILE *moira_log;
#endif //LOG_MOIRA

size_t
nextPowerOfTwo(size_t v) 
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	v++;
	return(v);
}

#define LPO2 5 // sizeof(FBlock) == 24 > (1<<4)
int
getCat(size_t v)
{
	size_t po2 = -1;
	while(v)
	{
		v >>= 1;
		po2 += 1;
	}
	return po2 < LPO2 ? LPO2 : po2;
}



typedef struct Meta_T
{
#define M\
	size_t used : 1;\
	size_t size  : 5;\
	size_t psize : 5 
	M; 
	/* 
	 * We use bitfields because we can make strong assumptions about how big blocks are
	 * But those assumptions also make this fail testcase 6, too much internal fragmentation.
	 * :kekhands: 2022-10-26
	 */
	
	/* Note(Qwendo): Into this Meta struct two more pointers could fit before
	 * we would reach the 8 byte limit. 2022-10-26 
	 */
} Meta;


#define AB 1
#define FB 0


#define N_S_C 25 // We have 20 * (1<<20) bytes maximum Heap size

#define ALIGN_BSIZE(s) (ALIGN(s) + ALIGNEMENT - 1)
#define blockSizeWithPayload(s) (s + sizeof(Meta))
#define valid(x) ((mem_heap_lo() <= (void *)x )&& (mem_heap_hi() + 1 > (void *)x))

typedef struct ABlockHeader_T
{
	M;
	size_t   data[];// for ALIGNMENT OF 8 BYTES
	/*
	 * Note(Qwendo): This is not standard C, wont work on msvc (only clang and gcc support unsized arrays)
	 * afaik 2022-10-26
	 */
} ABlock;

typedef struct FBlockHeader_T
{
	M;
	struct FBlockHeader_T *next;
	struct FBlockHeader_T *prev;
} FBlock;

typedef struct HHeader_T
{
	Meta   *last;
	FBlock *lists[N_S_C];
} HHeader;

#if ASSERT
void
assertHHeap()
{
	HHeader *h = (HHeader *)mem_heap_lo();
	for(int i = 0; i < N_S_C; i++)
	{
		assert(valid(h->lists[i]) || h->lists[i] == NULL);
	}
}
#endif //ASSERT

#if GDBPASSERT_PRINT
#	define gdbpassert(x) do{printf( # x ": %d\n",  !!(x)); if(!(x)){return(0);}}while(0)
#else
#	define gdbpassert(x) do{                               if(!(x)){return(0);}}while(0)
#endif //GDBPASSERT_PRINT

int
cB(FBlock *a)
{
	gdbpassert(a);
	gdbpassert(valid(a));
	gdbpassert(a->used == FB || a->used == AB);
	gdbpassert(a->size < N_S_C && (a->psize < N_S_C || (void *)a == (char *)mem_heap_lo() + sizeof(HHeader)));
	gdbpassert(a->size >= LPO2);
	return(1);
}	

int
checkBlock(FBlock *a)
{
#	if ASSERT
	assert(a);
	assert(valid(a));
	assert(a->used == FB || a->used == AB);
	assert(a->size < N_S_C && a->psize < N_S_C);
	assert(a->size < N_S_C && (a->psize < N_S_C || (void *)a == (char *)mem_heap_lo() + sizeof(HHeader)));
#	endif // ASSERT
	return(1);
}

void
unlinkFB(FBlock *f)
{
#if UNLINK_ASSERT_HEAP_LISTS
	assertHHeap();
#endif //UNLINK_ASSERT_HEAP_LISTS
	HHeader *h = (HHeader *)mem_heap_lo();
	if(h->lists[f->size] == f) {h->lists[f->size] = f->next;}
	if(f->prev) {f->prev->next = f->next;}
	if(f->next) {f->next->prev = f->prev;}
	f->next = NULL;
	f->prev = NULL;
#if UNLINK_ASSERT_HEAP_LISTS
	assertHHeap();
#endif //UNLINK_ASSERT_HEAP_LISTS
}


#if ASSERT
int
search(FBlock *f)
{
	if(!f) return 1;
	HHeader *h = (HHeader *)mem_heap_lo();
	FBlock *r = h->lists[f->size];
	while(r)
	{
		if(r == f) {return(1);}
		r = r->next;
		
	}
	return(0);
}

int searchAll(void * a)
{
	HHeader *h = (HHeader *)mem_heap_lo();
	for(int i = 0; i < N_S_C; i++)
	{
			FBlock *r = h->lists[i];
			while(r)
			{
				if(r == a) {return(1);}
				r = r->next;
			}
	}
	return(0);
}

#endif //ASSERT

#if LOG
void
pR()
{
#	if LOG_LL
	FILE *log_ll = fopen("LOG_LinkedLists.csv", "w+");
#	endif //LOG_LL
	HHeader *h = (HHeader *)mem_heap_lo();
	for(int i = 0; i < N_S_C; i++)
	{
		int n = 0;
		FBlock *r = h->lists[i];
		while(r)
		{
			n++;
			r = r->next;
		}
		printf("% 3d % 4d\n", i, n);
#		if LOG_LL
		fprintf(log_ll, "%d, %d\n", i, n);
#		endif //LOG_LL

	}
}

#endif //LOG

FBlock *
extract(size_t cat)
{
#	if EXTRACT_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif

	HHeader *h = mem_heap_lo();
	FBlock *f = h->lists[cat];
	if(!f) {return(NULL);}
#	if EXTRACT_ASSERT_VALID
	assert(valid(f) || f == NULL);
	assert(!f || valid(f->next) || f->next == NULL);
	assert(!f || valid(f->prev) || f->prev == NULL);
	assert(checkBlock(f));
#	endif //EXTRACT_ASSERT_VALID

	h->lists[cat] = f->next;
	if(h->lists[cat]) {h->lists[cat]->prev = NULL;}



#	if EXTRACT_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif
	return(f);
}

void
insert(FBlock *const f)
{
#	if INSERT_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif //INSERT_ASSERT_HEAP_LISTS
#	if INSERT_ASSERT_VALID
	assert(checkBlock(f));
#	endif //INSERT_ASSERT_VALID
	HHeader *h = mem_heap_lo();
	f->next = NULL;
	f->next = h->lists[f->size];
	f->prev = NULL;
	if(h->lists[f->size]) 
	{
		h->lists[f->size]->prev = f;
	}
	h->lists[f->size] = f;

#	if ASSERT && INSERT_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif //INSERT_ASSERT_HEAP_LISTS

#	if ASSERT && INSERT_ASSERT_VALID
	assert(valid(f) || f == NULL);
	assert(!f || valid(f->next) || f->next == NULL);
	assert(!f || valid(f->prev) || f->prev == NULL);
	assert(checkBlock(f));
#	endif //INSERT_ASSERT_VALID
}

FBlock *
splitHalf(FBlock *f, size_t n_cat)
{

	HHeader *h = (HHeader *)mem_heap_lo();
#	if ASSERT
	FBlock fp = *f;
	assert(checkBlock(f));
	assert(f->size >= LPO2);
	assert(f->size - 1 == n_cat);
#	endif // ASSERT
	char *next   = (char*)f + (1<<f->size);
	ABlock *an = NULL;

	char *second = (char*)f + (1<<n_cat);
	if(h->last == (Meta *)f) {h->last = (Meta *)second;}
	FBlock *s = (FBlock *)second;
	s->size  = n_cat;	
	f->size  = n_cat;
	s->psize = f->size; 
	s->next  = NULL;
	s->prev  = NULL;
	s->used  = FB;
	if(valid(next)) 
	{
		an = (ABlock *)next;
		an->psize = s->size;
	}
#	if ASSERT
	assert(checkBlock(s));
	assert(checkBlock((FBlock *)h->last));
	assert(checkBlock(f));
	assert(f->size == fp.size - 1);
	assert(f->psize == fp.psize);
	assert(f->used == fp.used);
#	endif //ASSERT
	insert(s);
	return(f);
}

void
dumpHeapLists(FILE *f)
{
	HHeader *h = (HHeader *)mem_heap_lo();
	for(int i = 0; i < N_S_C; i++)
	{
		int c = 0;
		FBlock *r = h->lists[i];
		while(r)
		{
			c++;
			r = r->next;
		}
		fprintf(f, "% 3d, %d\n",i, c);
	}
}

void
dumpHeap(FILE *f)
{
	fprintf(f, "[\n");
	Meta *a = (Meta *)((char *)mem_heap_lo() + ALIGN(sizeof(HHeader)));
	while(valid(a))
	{
		fprintf(f, "{\"kind\": \"%c\", \"size\": %zu}",a->used == FB ? 'f' : 'u', (size_t)1<<a->size);
		a = (Meta *)((char *)a + (1<<a->size));
		if(valid(a)) fprintf(f, ",\n");
	}
	fprintf(f, "\n]");
}


void
documentHeap()
{
	static int rq = 0;
	char fname[100];
	sprintf(fname, "dumps/list_dump%05d.csv", rq);
	FILE *dump = fopen(fname, "w+");
	dumpHeapLists(dump);
	fclose(dump);
	sprintf(fname, "vis/dump/dump-%05d.json", rq);
	dump = fopen(fname, "w+");
	dumpHeap(dump);
	fclose(dump);
	rq++;
}

#if MOIRA
void
mergeTwo(FBlock *l, FBlock *t)
{
#	if MOIRA_M2
	char *next    = (char *)t + (1<<t->size);
	Meta *n       = (Meta *)next;
	size_t combined_sizes = (1<<t->size) + (1<<l->size);
	if(combined_sizes == nextPowerOfTwo(combined_sizes))
	{
#		if LOG && LOG_MOIRA && LOG_MOIRA_MERGES && LOG_MOIRA_M2
		printf("%d %d\n",l->size, t->size);
#		endif //LOG_MOIRA_M2
		size_t cat = getCat(nextPowerOfTwo(combined_sizes));
		unlinkFB(t);
		unlinkFB(l);
		l->size = cat;
		n->psize = cat;
		insert(l);
#		if ASSERT
		assert(!searchAll(t));
		assert(search(l));
#		endif //ASSERT
		return;
	}
#	endif // MOIRA_M2
}
#endif //MOIRA


#if MOIRA
void
mergeThree(FBlock *l, FBlock *m, FBlock *t)
{
#	if MOIRA_M3
	size_t s_lmt = (1<<l->size) + (1<<m->size) + (1<<t->size);
	size_t s_mt  =                (1<<m->size) + (1<<t->size);
	size_t s_lm  = (1<<l->size) + (1<<m->size)               ;
	if(s_lmt == nextPowerOfTwo(s_lmt))
	{

		char *next = (char *)t + (1<<t->size);
		Meta *n    = (Meta *)next;
		size_t cat = getCat(s_lmt);
#		if LOG && LOG_MOIRA && LOG_MOIRA_MERGES && LOG_MOIRA_M3
		printf("%d %d %d -> %d\n",l->size,m->size, t->size, cat);
#		endif //LOG_MOIRA_M3
		unlinkFB(l);
		unlinkFB(m);
		unlinkFB(t);
		l->size = cat;
		n->psize = cat;
		insert(l);
		
	}else if(s_mt == nextPowerOfTwo(s_mt))
	{
		mergeTwo(m,t);
	}else if(s_lm == nextPowerOfTwo(s_lm))
	{
		mergeTwo(l,m);
	}
#	endif //MOIRA_M3
}
#endif //MOIRA

void
moira(FBlock *f)
{
#	if MOIRA

#	if LOG_MOIRA
	static size_t request_number = 0;
	fprintf(moira_log, "%zu, %u,\n", request_number++, f->size);
#	if LOG_MOIRA_DUMP_HEAP
	if(request_number % 10000 == 0)
	{
		documentHeap();
	}
#	endif //LOG_MOIRA_DUMP_HEAP
#endif // LOG_MOIRA

	char *n_c = (char *)f + (1<<f->size);
	char *p_c = (char *)f - (1<<f->psize);
	FBlock *n = (FBlock *)n_c;
	n = valid(n) && n->used == FB ? n : NULL;
	FBlock *p = (FBlock *)p_c;
	p = valid(p) && p->used == FB ? p : NULL;
	int c = 0 + (!!n) + ((!!p)<<1);
	switch(c)
	{
	case 0:
	default: break;
	break;case 1:
		mergeTwo(f, n);
	break;case 2:
		mergeTwo(p, f);
	break;case 3:
		mergeThree(p,f,n);
	break;
	}

#	endif // MOIRA
}

FBlock *
genji(size_t block_size)
{
#	if GENJI

#	if ASSERT && GENJI_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif //GENJI_ASSERT_HEAP_LISTS

	HHeader *h = (HHeader *)mem_heap_lo();
	size_t cat = getCat(block_size);
	size_t found_cat = 0;
	for(int i = cat + 1; i < N_S_C; i++)
	{
		if(h->lists[i]) 
		{
			found_cat = i;
			break;
		}
	}
	if(!found_cat) {return(NULL);}
	FBlock *r = extract(found_cat);

#	if ASSERT && GENJI_ASSERT_BLOCKS
	assert(checkBlock(r));
#	endif // GENJI_ASSERT_BLOCKS

	if(h->last == (Meta *)r)
	{
		size_t n_cat = found_cat - 1;
		r = splitHalf(r, n_cat);
		found_cat = n_cat;
	}

#	if ASSERT && GENJI_ASSERT_BLOCKS
	assert(checkBlock(r));
#	endif // GENJI_ASSERT_BLOCKS

#	if ASSERT && GENJI_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif //GENJI_ASSERT_HEAP_LISTS


	for(int i = found_cat; i > cat; i--)
	{
		size_t n_cat = i - 1;
		r = splitHalf(r, n_cat);
	}

#	if ASSERT && GENJI_ASSERT_HEAP_LISTS
	assertHHeap();
#	endif //GENJI_ASSERT_HEAP_LISTS
	return(r);
#	endif //GENJI
	return NULL;
}

HHeader *h = NULL;

FBlock *
ana(size_t block_size)
{
	HHeader *h = (HHeader *)mem_heap_lo();
	FBlock *f = mem_sbrk(block_size);
	if(!f || !~(size_t)f) {return(NULL);}
	memset(f, 0xFF, sizeof(FBlock));
	f->psize = h->last ? h->last->size : 0;
	h->last = (Meta *)f;
	return(f);
}

int
mm_init(void)
{
#if LOG_MOIRA
	moira_log = fopen("moira.csv", "wa");
#endif //LOG_MOIRA
	h = mem_sbrk(ALIGN(sizeof(HHeader)));
	memset(h, 0, sizeof(HHeader));
	return(0);
}

void *
mm_malloc(size_t s)
{
	size_t size_with_payload = blockSizeWithPayload(ALIGN(s));
	size_t request_size = nextPowerOfTwo(size_with_payload);
	size_t cat = getCat(request_size);
	FBlock *f = extract(cat); 
	if(!f)
	{
		f = genji(request_size);        // splitting first
		if(!f) {f = ana(request_size);} // extend heap second 
		if(!f)                          // fail else kekw
		{
#			if LOG_NO_MEM
			printf("size rq = %zu\ncat = %zu\n", s, cat);
			documentHeap();
#			endif //LOG_NO_MEM
			return(NULL);
		}          // fail third
	}
	f->size = cat;
	f->used = AB;
#	if ASSERT
	checkBlock(f);
#	endif //ASSERT
	ABlock *a = (ABlock *)f;
	memset(a->data, 0xFF, (1<<a->size) - sizeof(ABlock));

#	if ASSERT
	size_t align = a->data;
	assert(align == ALIGN(align));
#	endif//ASSERT
	return(a->data);
}


void 
mm_free(void *ptr)
{
	FBlock *f = (FBlock *)((char *)ptr - sizeof(ABlock));
#	if ASSERT
	assert(ptr == ((ABlock *)f)->data);
#	endif //ASSERT
	size_t cat  = f->size;
	size_t pcat = f->psize;
	memset(f, 0xFF, 1<<f->size); 
	f->used  = FB;
	f->size  = cat;
	f->psize = pcat;
#	if ASSERT
	assert(f->size < 64);
#	endif //ASSERT
	insert(f);
	moira(f);
}

void *
mm_realloc(void *ptr, size_t s)
{
	size_t request_size = blockSizeWithPayload(s);
	FBlock *f = (FBlock *)((char *)ptr - sizeof(ABlock));
	size_t cat = f->size;
	size_t available_size = (1<<cat);
	if(request_size < available_size) {return(ptr);};
	void *n_ptr = mm_malloc(s);
	if(!n_ptr) {return(NULL);};
	memcpy(n_ptr, ptr, available_size);
	mm_free(ptr);
	return(n_ptr);
}

void
mm_deinit()
{
#	if LOG
	//documentHeap();
#	endif //LOG
}
