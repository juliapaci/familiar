#include <webgpu/webgpu.h>

#include <stdio.h>

int main(void) {
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = NULL;

    WGPUInstance instance = wgpuCreateInstance(&desc);

    if(!instance) {
        fprintf(stderr, "couldnt init webgpu\n");
        return 1;
    }

    printf("wgpu instance: %p\n", instance);

    return 0;
}
