using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ
{
    [Serializable]
    public abstract class AEFxParam
    {
        public string name;
        public aepAPI.aepParamType type;

        public AEFxParam(aepAPI.aepParam p)
        {
            var info = default(aepAPI.aepParamInfo);
            aepAPI.aepGetParamInfo(p, ref info);

            name = info.name;
            type = info.type;
        }

        public virtual void Sanitize() {}
    }
    [Serializable]
    public class AEFxBoolParam : AEFxParam
    {
        public aepAPI.aepBoolParamValue value;

        public AEFxBoolParam(aepAPI.aepParam p)
            : base(p)
        {
            aepAPI.aepGetParamValue(p, ref value);
        }

        public override void Sanitize()
        {
            value.value = value.value != 0 ? 1 : 0;
        }
    }
    [Serializable]
    public class AEFxIntParam : AEFxParam
    {
        public aepAPI.aepIntParamValue value;

        public AEFxIntParam(aepAPI.aepParam p)
            : base(p)
        {
            aepAPI.aepGetParamValue(p, ref value);
        }

        public override void Sanitize()
        {
            value.value = Math.Min(Math.Max(value.value, value.min), value.max);
        }
    }
    [Serializable]
    public class AEFxDoubleParam : AEFxParam
    {
        public aepAPI.aepDoubleParamValue value;

        public AEFxDoubleParam(aepAPI.aepParam p)
            : base(p)
        {
            aepAPI.aepGetParamValue(p, ref value);
        }

        public override void Sanitize()
        {
            value.value = Math.Min(Math.Max(value.value, value.min), value.max);
        }
    }
    [Serializable]
    public class AEFxColorParam : AEFxParam
    {
        public aepAPI.aepColorParamValue value;

        public AEFxColorParam(aepAPI.aepParam p)
            : base(p)
        {
            aepAPI.aepGetParamValue(p, ref value);
        }
    }
    [Serializable]
    public class AEFxPoint2DParam : AEFxParam
    {
        public aepAPI.aepPoint2DParamValue value;

        public AEFxPoint2DParam(aepAPI.aepParam p)
            : base(p)
        {
            aepAPI.aepGetParamValue(p, ref value);
        }
    }
    [Serializable]
    public class AEFxPoint3DParam : AEFxParam
    {
        public aepAPI.aepPoint3DParamValue value;

        public AEFxPoint3DParam(aepAPI.aepParam p)
            : base(p)
        {
            aepAPI.aepGetParamValue(p, ref value);
        }
    }


    public static class aepAPI
    {
        public struct aepLayer
        {
            public IntPtr ptr;
            public static implicit operator bool(aepLayer v) { return v.ptr != IntPtr.Zero; }
        }
        public struct aepModule
        {
            public IntPtr ptr;
            public static implicit operator bool(aepModule v) { return v.ptr != IntPtr.Zero; }
        }
        public struct aepInstance
        {
            public IntPtr ptr;
            public static implicit operator bool(aepInstance v) { return v.ptr != IntPtr.Zero; }
        }
        public struct aepParam
        {
            public IntPtr ptr;
            public static implicit operator bool(aepParam v) { return v.ptr != IntPtr.Zero; }
        }

        public enum aepParamType
        {
            Unknown,
            Bool,
            Int,
            Double,
            Point2D,
            Point3D,
            Color,
            Layer,
            Path, // not supported yet
        };

        public struct aepParamInfo
        {
            public IntPtr name_;
            public aepParamType type;
            public string name { get { return ToS(name_); } }
        };

        public static string ToS(IntPtr ptr)
        {
            return ptr == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(ptr);
        }

        [Serializable]
        public struct aepPoint2D
        {
            public int x, y;
        };
        [Serializable]
        public struct aepPoint3D
        {
            public double x, y, z;
        };
        [Serializable]
        public struct aepColor
        {
            public byte a, r, g, b;
        };
        public struct aepLayerData
        {
            public IntPtr pixels;
            public int width, height, rowbytes;
        };

        [Serializable]
        public struct aepBoolParamValue
        {
            public int value, def;
        };
        [Serializable]
        public struct aepIntParamValue
        {
            public int value, def, min, max;
        };
        [Serializable]
        public struct aepDoubleParamValue
        {
            public double value, def, min, max;
        };
        [Serializable]
        public struct aepPoint2DParamValue
        {
            public aepPoint2D value, def;
        };
        [Serializable]
        public struct aepPoint3DParamValue
        {
            public aepPoint3D value, def;
        };
        [Serializable]
        public struct aepColorParamValue
        {
            public aepColor value, def;
        };

        [DllImport("AfterEffectsPlugin")] public static extern aepLayer     aepCreateLayer();
        [DllImport("AfterEffectsPlugin")] public static extern void         aepDestroyLayer(aepLayer layer);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepResizeLayer(aepLayer layer, int width, int height);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetLayerData(aepLayer layer, ref aepLayerData dst);

        [DllImport("AfterEffectsPlugin")] public static extern aepModule    aepLoadModule(string path);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepUnloadModule(aepModule mod);
        [DllImport("AfterEffectsPlugin")] public static extern aepInstance  aepCreateInstance(aepModule mod);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepDestroyInstance(aepInstance ins);

        [DllImport("AfterEffectsPlugin")] public static extern int          aepGetNumParams(aepInstance ins);
        [DllImport("AfterEffectsPlugin")] public static extern aepParam     aepGetParam(aepInstance ins, int i);
        [DllImport("AfterEffectsPlugin")] public static extern aepParam     aepGetParamByName(aepInstance ins, string name);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamInfo(aepParam param, ref aepParamInfo dst);

        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetInput(aepInstance ins, aepLayer layer);
        [DllImport("AfterEffectsPlugin")] public static extern aepLayer     aepGetResult(aepInstance ins);
        [DllImport("AfterEffectsPlugin")] public static extern aepLayer     aepRender(aepInstance ins, double time, int width, int height);

        [DllImport("AfterEffectsPlugin")] public static extern IntPtr       GetRenderEventFunc();
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGuardBegin();
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGuardEnd();
        [DllImport("AfterEffectsPlugin")] public static extern void         aepEraseDeferredCall(int id);
        [DllImport("AfterEffectsPlugin")] public static extern int          aepRenderDeferred(aepInstance inst, double time, int width, int height, int id);


        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, IntPtr value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, ref aepBoolParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, ref aepIntParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, ref aepDoubleParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, ref aepPoint2DParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, ref aepPoint3DParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepGetParamValue(aepParam param, ref aepColorParamValue value);

        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, IntPtr value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, ref aepBoolParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, ref aepIntParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, ref aepDoubleParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, ref aepPoint2DParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, ref aepPoint3DParamValue value);
        [DllImport("AfterEffectsPlugin")] public static extern void         aepSetParamValue(aepParam param, ref aepColorParamValue value);

        public static AEFxParam CreateToonzParam(aepParam param)
        {
            var info = default(aepParamInfo);
            aepGetParamInfo(param, ref info);

            AEFxParam ret = null;
            switch (info.type)
            {
                case aepParamType.Bool: ret = new AEFxBoolParam(param); break;
                case aepParamType.Int: ret = new AEFxIntParam(param); break;
                case aepParamType.Double: ret = new AEFxDoubleParam(param); break;
                case aepParamType.Point2D: ret = new AEFxPoint2DParam(param); break;
                case aepParamType.Point3D: ret = new AEFxPoint3DParam(param); break;
                case aepParamType.Color: ret = new AEFxColorParam(param); break;
            }
            return ret;
        }

        public static void SetParamValue(aepParam param, AEFxParam v)
        {
            var t = v.GetType();
            if (t == typeof(AEFxBoolParam))
            {
                aepSetParamValue(param, ref ((AEFxBoolParam)v).value);
            }
            else if (t == typeof(AEFxIntParam))
            {
                aepSetParamValue(param, ref ((AEFxIntParam)v).value);
            }
            else if (t == typeof(AEFxDoubleParam))
            {
                aepSetParamValue(param, ref ((AEFxDoubleParam)v).value);
            }
            else if (t == typeof(AEFxPoint2DParam))
            {
                aepSetParamValue(param, ref ((AEFxPoint2DParam)v).value);
            }
            else if (t == typeof(AEFxPoint3DParam))
            {
                aepSetParamValue(param, ref ((AEFxPoint3DParam)v).value);
            }
            else if (t == typeof(AEFxColorParam))
            {
                aepSetParamValue(param, ref ((AEFxColorParam)v).value);
            }
        }
    }
}