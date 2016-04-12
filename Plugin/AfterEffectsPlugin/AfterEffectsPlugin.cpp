#include "pch.h"
#include "aepInternal.h"


aepCLinkage aepExport aepLayer* aepCreateLayer()
{
    return new aepLayer();
}
aepCLinkage aepExport void aepDestroyLayer(aepLayer *layer)
{
    delete layer;
}
aepCLinkage aepExport void aepResizeLayer(aepLayer *layer, int width, int height)
{
    if (!layer) { return; }
    layer->resize(width, height);
}
aepCLinkage aepExport void aepGetLayerData(aepLayer *layer, aepLayerData *dst)
{
    if (!layer || !dst) { return; }
    dst->width      = layer->getWidth();
    dst->height     = layer->getHeight();
    dst->rowbytes   = layer->getPitch();
    dst->pixels     = layer->getData();
}

namespace {
    typedef std::unique_ptr<aepModule> aepModulePtr;
    std::map<std::string, aepModulePtr> g_modules;
}

aepCLinkage aepExport aepModule* aepLoadModule(const char *path)
{
    // if already loaded, return it
    auto i = g_modules.find(path);
    if (i != g_modules.end()) {
        return i->second.get();
    }

    auto ret = new aepModule();
    if (!ret->load(path)) {
        delete ret;
        return false;
    }
    g_modules[path] = aepModulePtr(ret);
    return ret;
}

aepCLinkage aepExport void aepUnloadModule(aepModule *mod)
{
    // do nothing
}


aepCLinkage aepExport aepInstance*  aepCreateInstance(aepModule *mod)
{
    if (!mod) { return nullptr; }
    return mod->createInstance();
}

aepCLinkage aepExport void aepDestroyInstance(aepInstance *ins)
{
    delete ins;
}

aepCLinkage aepExport int aepGetNumParams(aepInstance *ins)
{
    if (!ins) { return 0; }
    return ins->getNumParams();
}
aepCLinkage aepExport aepParam* aepGetParam(aepInstance *ins, int i)
{
    if (!ins) { return nullptr; }
    return ins->getParam(i);
}
aepCLinkage aepExport aepParam* aepGetParamByName(aepInstance *ins, const char *name)
{
    if (!ins) { return nullptr; }
    return ins->getParamByName(name);
}

aepCLinkage aepExport void aepGetParamInfo(aepParam *param, aepParamInfo *dst)
{
    if (!param || !dst) { return; }
    dst->name = param->getName();
    dst->type = param->getType();
}
aepCLinkage aepExport void aepGetParamValue(aepParam *param, void *value)
{
    if (!param || !value) { return; }
    param->getValue(value);
}
aepCLinkage aepExport void aepSetParamValue(aepParam *param, const void *value)
{
    if (!param || !value) { return; }
    param->setValue(value);
}

aepCLinkage aepExport void aepSetInput(aepInstance *ins, aepLayer *layer)
{
    if (!ins) { return; }
    ins->setInput(layer);
}
aepCLinkage aepExport void aepSetDstSize(aepInstance *ins, int width, int height)
{
    if (!ins) { return; }
    ins->setDstSize(width, height);
}
aepCLinkage aepExport aepLayer* aepGetDstImage(aepInstance *ins)
{
    if (!ins) { return nullptr; }
    return ins->getDstImage();
}

aepCLinkage aepExport aepLayer* aepRender(aepInstance *ins, double time)
{
    if (!ins) { return nullptr; }
    return ins->render(time);
}


#ifndef aepStaticLink
// deferred call APIs

#include "PluginAPI/IUnityGraphics.h"

aepCLinkage aepExport void  aepGuardBegin();
aepCLinkage aepExport void  aepGuardEnd();
aepCLinkage aepExport void  aepEraseDeferredCall(int id);
aepCLinkage aepExport int   aepRenderDeferred(aepInstance *inst, double time, int id);
aepCLinkage aepExport UnityRenderingEvent   GetRenderEventFunc();

typedef std::function<void()> DeferredCall;
namespace {
    std::mutex g_deferred_calls_mutex;
    std::vector<DeferredCall> g_deferred_calls;
}

aepCLinkage aepExport void aepGuardBegin()
{
    g_deferred_calls_mutex.lock();
}

aepCLinkage aepExport void aepGuardEnd()
{
    g_deferred_calls_mutex.unlock();
}

aepCLinkage aepExport int aepAddDeferredCall(const DeferredCall& dc, int id)
{
    if (id <= 0) {
        // search empty object and return its position if found
        for (int i = 1; i < (int)g_deferred_calls.size(); ++i) {
            if (!g_deferred_calls[i]) {
                g_deferred_calls[i] = dc;
                return i;
            }
        }

        // 0th is "null" object
        if (g_deferred_calls.empty()) { g_deferred_calls.emplace_back(DeferredCall()); }

        // allocate new one
        g_deferred_calls.emplace_back(dc);
        return (int)g_deferred_calls.size() - 1;
    }
    else if (id < (int)g_deferred_calls.size()) {
        g_deferred_calls[id] = dc;
        return id;
    }
    else {
        utjDebugLog("should not be here");
        return 0;
    }
}

aepCLinkage aepExport void aepEraseDeferredCall(int id)
{
    if (id <= 0 || id >= (int)g_deferred_calls.size()) { return; }

    g_deferred_calls[id] = DeferredCall();
}

// **called from rendering thread**
aepCLinkage aepExport void aepCallDeferredCall(int id)
{
    std::unique_lock<std::mutex> l(g_deferred_calls_mutex);
    if (id <= 0 || id >= (int)g_deferred_calls.size()) { return; }

    auto& dc = g_deferred_calls[id];
    if (dc) { dc(); }
}

aepCLinkage aepExport int aepRenderDeferred(aepInstance *inst, double time, int id)
{
    if (!inst) { return 0; }
    return aepAddDeferredCall([=]() {
        return inst->render(time);
    }, id);
}

static void UNITY_INTERFACE_API UnityRenderEvent(int eventID)
{
    aepCallDeferredCall(eventID);
}
aepCLinkage aepExport UnityRenderingEvent GetRenderEventFunc()
{
    return UnityRenderEvent;
}
#endif // aepStaticLink
