using GE;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GE
{
    public static class Log
    {
        public static void LogCoreInfo(string message)
        {
            InternalCalls.Log_Core_Info(message);
        }

        public static void LogCoreTrace(string message)
        {
            InternalCalls.Log_Core_Trace(message);
        }

        public static void LogCoreWarn(string message)
        {
            InternalCalls.Log_Core_Warn(message);
        }

        public static void LogCoreError(string message)
        {
            InternalCalls.Log_Core_Error(message);
        }

        public static void LogCoreAssert(object arg, string message)
        {
            InternalCalls.Log_Core_Assert(arg, message);
        }
    }
}
