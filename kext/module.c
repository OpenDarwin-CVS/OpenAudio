
#include "ds_module.h"

#include <mach/mach_types.h>

KMOD_EXPLICIT_DECL(org.opendarwin.darwinsound,  "0.1",
		   ds_start, ds_stop)

__private_extern__ kmod_start_func_t *_realmain = 0;
__private_extern__ kmod_stop_func_t *_antimain = 0;
__private_extern__ int _kext_apple_cc = __APPLE_CC__ ;
