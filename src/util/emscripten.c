#include "util/emscripten.h"

#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void emscripten_idbfs_prepare()
{
    printf("IDBFS pre-preparing\n");
#if defined(PLATFORM_WEB)
    EM_ASM(
        Asyncify.handleSleep(complete => {
            FS.mkdir('/cfg');
            FS.mount(IDBFS, {}, '/cfg');

            // Then sync
            FS.syncfs(
                true, function(err) {
                    console.log('IDBFS prepared');
                    complete();
                });
        }););
#endif
    printf("IDBFS post-preparing\n");
}

void emscripten_idbfs_sync()
{
    printf("IDBFS pre-syncing\n");
#if defined(PLATFORM_WEB)
    EM_ASM(
        Asyncify.handleSleep(complete => { FS.syncfs(
                                                false, function(err) {
                                                    // Error
                                                    console.log('IDBFS synced');
                                                    complete();
                                                }); }););
#endif
    printf("IDBFS post-syncing\n");
}