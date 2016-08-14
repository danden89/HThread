#ifndef HCONTEXT_H
#define	HCONTEXT_H

namespace hthread
{

#define BOOST_CONTEXT_DECL 
#define BOOST_CONTEXT_CALLDECL

typedef void* fcontext_t;

//extern "C" BOOST_CONTEXT_DECL
//intptr_t BOOST_CONTEXT_CALLDECL \
//jump_fcontext(fcontext_t * ofc, fcontext_t nfc,
//          intptr_t vp, bool preserve_fpu = false);

extern "C" BOOST_CONTEXT_DECL
intptr_t BOOST_CONTEXT_CALLDECL \
safe_jump_fcontext(fcontext_t * ofc, fcontext_t nfc,
            intptr_t vp, bool preserve_fpu = false,
            unsigned long long* pjump_lock = nullptr);


extern "C" BOOST_CONTEXT_DECL
intptr_t BOOST_CONTEXT_CALLDECL \
try_jump_fcontext(fcontext_t * ofc, fcontext_t nfc,
            intptr_t vp, bool preserve_fpu = false,
            unsigned long long* pjump_lock = nullptr);

extern "C" BOOST_CONTEXT_DECL
fcontext_t BOOST_CONTEXT_CALLDECL \
make_fcontext(void * sp, std::size_t size, void (* fn)(intptr_t));

}

#endif	/* HCONTEXT_H */

