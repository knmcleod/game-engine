using System;
using System.Runtime.CompilerServices;

namespace GE
{
    public struct Vector3
    {
        public float x, y, z;

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string text, int parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NativeLog_VectorDot(ref Vector3 parameter);

    }

    public class EntityClass
    {
        public EntityClass()
        {
            Log("Entity Constructor", 1010);

            Vector3 position = new Vector3(1, 1, 1);
            Vector3 result = Log(position);

            Console.WriteLine($"{result.x}, {result.y}, {result.z}");
            Console.WriteLine("{0}", InternalCalls.NativeLog_VectorDot(ref position));
        }

        public void PrintMessage(string message)
        {
            Console.WriteLine(message);
        }

        public void PrintInt(int value)
        {
            Console.WriteLine(value);
        }
    
        private void Log(string text, int parameter)
        {
            InternalCalls.NativeLog(text, parameter);
        }

        private Vector3 Log(Vector3 parameter)
        {
            InternalCalls.NativeLog_Vector(ref parameter, out Vector3 result);
            return result;
        }
    }
}