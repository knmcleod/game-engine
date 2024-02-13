using System;
using System.Runtime.CompilerServices;

namespace GE
{
    internal class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong uuid, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong uuid, ref Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong uuid, ref Vector2 impulse, ref Vector2 point, ref bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong uuid, ref Vector2 impulse, ref bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong uuid, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);


    }
}
