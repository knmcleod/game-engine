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

        // INPUT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseDown(MouseCode mouseCode);

        // ENTITY
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsHovered(ulong uuid);

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
        internal extern static void TransformComponent_GetPivot(ulong uuid, out Vector3 pivot);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ActiveComponent_GetActive(ulong uuid, out bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ActiveComponent_SetActive(ulong uuid, ref bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ActiveComponent_GetHidden(ulong uuid, out bool hidden);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ActiveComponent_SetHidden(ulong uuid, ref bool hidden);


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
        internal extern static void SpriteRendererComponent_GetColor(ulong uuid, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetColor(ulong uuid, ref Vector4 color);


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
        internal extern static void GUICanvasComponent_GetControlMouse(ulong uuid, out bool control);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUICanvasComponent_SetControlMouse(ulong uuid, ref bool control);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUICanvasComponent_GetShowMouse(ulong uuid, out bool show);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUICanvasComponent_SetShowMouse(ulong uuid, ref bool show);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUICanvasComponent_GetMode(ulong uuid, out GUICanvasComponent.CanvasMode mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUICanvasComponent_SetMode(ulong uuid, ref GUICanvasComponent.CanvasMode mode);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIImageComponent_GetColor(ulong uuid, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIImageComponent_SetColor(ulong uuid, ref Vector4 color);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIButtonComponent_GetEnabledColor(ulong uuid, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIButtonComponent_SetEnabledColor(ulong uuid, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIButtonComponent_GetHoveredColor(ulong uuid, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIButtonComponent_SetHoveredColor(ulong uuid, ref Vector4 color);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string GUIInputFieldComponent_GetText(ulong uuid);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_SetText(ulong uuid, string text);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_GetTextColor(ulong uuid, out Vector4 textColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_SetTextColor(ulong uuid, ref Vector4 textColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_GetBGColor(ulong uuid, out Vector4 bgColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_SetBGColor(ulong uuid, ref Vector4 bgColor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GUIInputFieldComponent_GetLineHeight(ulong uuid);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_SetLineHeight(ulong uuid, float lineHeight);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GUIInputFieldComponent_GetLineSpacing(ulong uuid);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUIInputFieldComponent_SetLineSpacing(ulong uuid, float lineSpacing);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUISliderComponent_GetFill(ulong uuid, out float fill);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GUISliderComponent_SetFill(ulong uuid, ref float fill);

    }
}
