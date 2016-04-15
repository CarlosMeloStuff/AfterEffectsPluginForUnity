#include <cstdio>
#include <vector>
#include "../AfterEffectsPlugin/AfterEffectsPlugin.h"
#pragma warning(disable:4996)

void __stdcall DLLCB(const char *dllname)
{
    printf("  %s\n", dllname);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s plugin_path\n", argv[0]);
        return 0;
    }

    const char *dllpath = argv[1];


    printf("%s\n", dllpath);
    printf("dependent dlls:\n");
    aepEnumerateDependentDLLs(dllpath, DLLCB);


    aepModule *mod = aepLoadModule(dllpath);
    if (!mod) {
        printf("failed to load module\n");
    }
    {
        aepPluginInfo info;
        aepGetPluginInfo(mod, &info);
        printf("%s\n", info.about);
    }

    aepInstance *inst = aepCreateInstance(mod);

    {
        aepParamInfo info;
        int nparams = aepGetNumParams(inst);
        printf("params:\n");
        for (int i = 0; i < nparams; ++i) {
            aepParam *param = aepGetParam(inst, i);
            aepGetParamInfo(param, &info);
            printf("  %s (%d)\n", info.name, info.type);
        }
    }

    aepLayer *input = aepCreateLayer();
    aepResizeLayer(input, 1980, 1080);

    aepSetInput(inst, input);
    aepBeginSequence(inst, 1980, 1080);
    aepRender(inst, 0.0);
    aepEndSequence(inst);

    aepDestroyLayer(input);
    aepDestroyInstance(inst);
    aepUnloadModule(mod);
}
