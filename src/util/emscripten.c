#include "util/emscripten.h"

#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void emscripten_idbfs_prepare()
{
#if defined(PLATFORM_WEB)
    EM_ASM(
        Asyncify.handleSleep(complete => {
            FS.mkdir('/cfg');
            FS.mount(IDBFS, {}, '/cfg');

            // Then sync
            FS.syncfs(
                true, function(err) {
                    console.log('IDBFS mount and sync complete');
                    complete();
                });
        }););
#endif
}

void emscripten_idbfs_sync()
{
#if defined(PLATFORM_WEB)
    EM_ASM(
        Asyncify.handleSleep(complete => { FS.syncfs(
                                                false, function(err) {
                                                    // Error
                                                    console.log('IDBFS sync complete');
                                                    complete();
                                                }); }););
#endif
}