#ifndef RMG_ALLOC_H
#define RMG_ALLOC_H 1

void *rmg_malloc(int n, size_t size );
void *rmg_malloc_init(int n, size_t size, char *type );
void *rmg_calloc(int n, size_t size );
void rmg_free( void *ptr );
void my_free( void *ptr );

#define my_free rmg_free
#define my_malloc( _ptr_, _nobj_, _type_ ) \
        _ptr_ = rmg_malloc((int) _nobj_, sizeof(_type_))

#define my_calloc( _ptr_, _nobj_, _type_ ) \
        _ptr_ = rmg_calloc((int) _nobj_, sizeof(_type_))

#define my_malloc_init( _ptr_, _nobj_, _type_ ) \
        _ptr_ = rmg_malloc_init((int) _nobj_, sizeof(_type_), #_type_)

#endif
