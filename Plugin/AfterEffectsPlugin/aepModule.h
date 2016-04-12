#ifndef aepModule_h
#define aepModule_h


// AfterEffects plugin entry point func
typedef PF_Err(*aepEntryPoint)(
    PF_Cmd			cmd,
    PF_InData		*in_data,
    PF_OutData		*out_data,
    PF_ParamDef		*params[],
    PF_LayerDef		*output,
    void			*extra);


class aepModule
{
public:
    aepModule();
    ~aepModule();

    bool load(const char *path);
    void unload();

    aepInstance* createInstance();
    aepEntryPoint getEntryPoint();

public:
    PF_Err      callPF(int cmd);
    PF_InData&  getPFInData();
    PF_OutData& getPFOutData();

    const std::string& getAbout() const;
    bool        hasDialog() const;
    bool        isInplace() const;

private:
    utj::module_t   m_module;
    aepEntryPoint   m_entrypoint;
    std::string     m_about;

    PF_InData       m_pf_in;
    PF_OutData      m_pf_out;
};

#endif // aepModule_h
