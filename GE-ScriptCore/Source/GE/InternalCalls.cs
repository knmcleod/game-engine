using System;
using System.Runtime.CompilerServices;

namespace GE
{
    internal class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log_Core_Info(string debugMessage);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log_Core_Trace(string debugMessage);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log_Core_Warn(string debugMessage);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log_Core_Error(string debugMessage);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log_Core_Assert(object arg, string debugMessage);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong uuid, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong uuid, ref Vector3 translation);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong uuid, ref Vector2 impulse, ref Vector2 point, ref bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong uuid, ref Vector2 impulse, ref bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong uuid, out Vector2 velocity);

        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong uuid, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByTag(string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_GetScriptInstance(ulong uuid);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);


    }
}
