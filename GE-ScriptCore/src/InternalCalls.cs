using System;
using System.Linq.Expressions;
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

        // ENTITY

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong uuid, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_GetScriptInstance(ulong uuid);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong uuid, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong uuid, ref Vector3 translation);



        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_GetLoop(ulong uuid, out bool loop);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_SetLoop(ulong uuid, ref bool loop);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_GetPitch(ulong uuid, out float pitch);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_SetPitch(ulong uuid, ref float pitch);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_GetGain(ulong uuid, out float gain);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_SetGain(ulong uuid, ref float gain);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void AudioSourceComponent_Play(ulong uuid);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong uuid, ref Vector2 impulse, ref Vector2 point, ref bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong uuid, ref Vector2 impulse, ref bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong uuid, out Vector2 velocity);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetText(ulong uuid);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetText(ulong uuid, string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetTextColor(ulong uuid, out Vector4 textColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetTextColor(ulong uuid, ref Vector4 textColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetBGColor(ulong uuid, out Vector4 bgColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetBGColor(ulong uuid, ref Vector4 bgColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetLineHeight(ulong uuid);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetLineHeight(ulong uuid, float lineHeight);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetLineSpacing(ulong uuid);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetLineSpacing(ulong uuid, float lineSpacing);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool UIRendererComponet_OnClick(ulong uuid, LambdaExpression func);

        // INPUT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseDown(MouseCode mouseCode);


    }
}
