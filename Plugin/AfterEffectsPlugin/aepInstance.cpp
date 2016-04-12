#include "pch.h"
#include "aepInternal.h"


aepInstance::aepInstance(aepModule *mod)
    : m_module(mod)
    , m_entrypoint(mod->getEntryPoint())
{
    memset(&m_pf_in, 0, sizeof(m_pf_out));
    memset(&m_pf_out, 0, sizeof(m_pf_in));

    (void*&)m_pf_in.effect_ref = this;
    m_pf_in.inter = aepGetHostCallbacks();
    m_pf_in.utils = &aepGetUtilCallbacks();
    m_pf_in.pica_basicP = &aepGetSPBasicSuite();

    {
        PF_ParamDef def;
        memset(&def, 0, sizeof(def));
        strcpy(def.name, "Input");
        def.param_type = PF_Param_LAYER;
        addParam(0, def);
    }

    callPF(PF_Cmd_ABOUT);
    m_about = m_pf_out.return_msg;

    callPF(PF_Cmd_GLOBAL_SETUP);
    callPF(PF_Cmd_PARAMS_SETUP);
}

aepInstance::~aepInstance()
{
    callPF(PF_Cmd_GLOBAL_SETDOWN);
}


int aepInstance::getNumParams() const
{
    return (int)m_params.size();
}

aepParam* aepInstance::getParam(int i)
{
    return m_params[i].get();
}

aepParam* aepInstance::getParamByName(const char *name)
{
    for(auto& p : m_params) {
        if (!p) { continue; }
        if (strcmp(p->getName(), name) == 0) {
            return p.get();
        }
    }
    return nullptr;
}

void aepInstance::setInput(aepLayer *inp)
{
    aepLayerParamValue lpv = {inp};
    getParam(0)->setValue(&lpv);
}

aepLayer* aepInstance::getDstImage()
{
    if (isInplace()) {
        aepLayerParamValue lpv;
        getParam(0)->getValue(&lpv);
        return lpv.value;
    }
    else {
        return &m_output;
    }
}

void aepInstance::beginSequence(int width, int height)
{
    m_output.resize(width, height);
    callPF(PF_Cmd_SEQUENCE_SETUP);
}

aepLayer* aepInstance::render(double time)
{
    callPF(PF_Cmd_FRAME_SETUP);
    callPF(PF_Cmd_RENDER);
    callPF(PF_Cmd_FRAME_SETDOWN);
    return &m_output;
}

void aepInstance::endSequence()
{
    callPF(PF_Cmd_SEQUENCE_SETDOWN);
}

PF_Err aepInstance::callPF(int cmd)
{
    return m_entrypoint(cmd, &m_pf_in, &m_pf_out, &m_pf_params[0], &m_output.m_pf, this);
}
const std::string& aepInstance::getAbout() const { return m_about; }
bool aepInstance::hasDialog() const { return (m_pf_out.out_flags & PF_OutFlag_I_DO_DIALOG) != 0; }
bool aepInstance::isInplace() const { return (m_pf_out.out_flags & PF_OutFlag_I_WRITE_INPUT_BUFFER) != 0; }

aepParam* aepInstance::addParam(int pos, const PF_ParamDef& pf)
{
    auto *param = new aepParam(this, pf);

    // pos < 0 means append to back
    if (pos < 0) {
        pos = (int)m_params.size();
        m_params.emplace_back(aepParamPtr(param));
    }
    else {
        if (pos >= (int)m_params.size()) {
            m_params.resize((size_t)pos + 1);
        }
        m_params[pos] = aepParamPtr(param);
    }

    m_pf_params.resize(m_params.size());
    m_pf_params[pos] = &param->getPFParam();
    return param;
}

void aepInstance::addSupportedFormat(PF_PixelFormat fmt)
{
    m_pf_formats.push_back(fmt);
}

void aepInstance::clearSupportedFormat()
{
    m_pf_formats.clear();
}

bool aepInstance::isFormatSupported(PF_PixelFormat fmt)
{
    auto i = std::find(m_pf_formats.begin(), m_pf_formats.end(), fmt);
    return i != m_pf_formats.end();
}
