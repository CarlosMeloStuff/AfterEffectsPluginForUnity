#include "pch.h"
#include "aepInternal.h"


aepModule::aepModule()
    : m_module()
    , m_entrypoint()
{
    memset(&m_pf_in, 0, sizeof(m_pf_in));
    memset(&m_pf_out, 0, sizeof(m_pf_out));
    m_pf_in.inter = aepGetHostCallbacks();
    m_pf_in.utils = &aepGetUtilCallbacks();
    m_pf_in.pica_basicP = &aepGetSPBasicSuite();
}

aepModule::~aepModule()
{
    unload();
}



bool aepModule::load(const char *path)
{
    unload();

    m_module = utj::DLLLoad(path);
    if (!m_module) { return false; }

    // check m_module is AfterEffects plugin
    (void*&)m_entrypoint = utj::DLLGetSymbol(m_module, "EntryPointFunc");
    if (!m_entrypoint) {
        (void*&)m_entrypoint = utj::DLLGetSymbol(m_module, "entryPointFunc");
    }

    if (!m_entrypoint) {
        unload();
        return false;
    }

    callPF(PF_Cmd_ABOUT);
    m_about = m_pf_out.return_msg;
    callPF(PF_Cmd_GLOBAL_SETUP);

    return true;
}

void aepModule::unload()
{
    if (m_module) {
        callPF(PF_Cmd_GLOBAL_SETDOWN);

        utj::DLLUnload(m_module);
        m_module = nullptr;
        m_entrypoint = nullptr;
    }
}

aepInstance* aepModule::createInstance()
{
    return new aepInstance(this);
}

aepEntryPoint aepModule::getEntryPoint()
{
    return m_entrypoint;
}

PF_Err aepModule::callPF(int cmd)
{
    if (!m_entrypoint) { return 0; }
    return m_entrypoint(cmd, &m_pf_in, &m_pf_out, nullptr, nullptr, this);
}

PF_InData&  aepModule::getPFInData() { return m_pf_in; }
PF_OutData& aepModule::getPFOutData() { return m_pf_out; }

const std::string& aepModule::getAbout() const { return m_about; }
bool aepModule::hasDialog() const { return (m_pf_out.out_flags & PF_OutFlag_I_DO_DIALOG) != 0; }
bool aepModule::isInplace() const { return (m_pf_out.out_flags & PF_OutFlag_I_WRITE_INPUT_BUFFER) != 0; }
