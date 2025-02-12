using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace GE
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }

        public Vector3 Pivot
        {
            get
            {
                InternalCalls.TransformComponent_GetPivot(Entity.ID, out Vector3 pivot);
                return pivot;
            }
        }
    }

    public class ActiveComponent : Component
    {
        public bool Active
        {
            get
            {
                InternalCalls.ActiveComponent_GetActive(Entity.ID, out bool active);
                return active;
            }
            set
            {
                InternalCalls.ActiveComponent_SetActive(Entity.ID, ref value);
            }
        }
        public bool Hidden
        {
            get
            {
                InternalCalls.ActiveComponent_GetHidden(Entity.ID, out bool hidden);
                return hidden;
            }
            set
            {
                InternalCalls.ActiveComponent_SetHidden(Entity.ID, ref value);
            }
        }

    }
    public class AudioSourceComponent : Component
    {
        public bool Loop
        {
            get
            {
                InternalCalls.AudioSourceComponent_GetLoop(Entity.ID, out bool loop);
                return loop;
            }
            set
            {
                InternalCalls.AudioSourceComponent_SetLoop(Entity.ID, ref value);
            }
        }
        public float Pitch
        {
            get
            {
                InternalCalls.AudioSourceComponent_GetPitch(Entity.ID, out float pitch);
                return pitch;
            }
            set
            {
                InternalCalls.AudioSourceComponent_SetPitch(Entity.ID, ref value);
            }
        }

        public float Gain
        {
            get
            {
                InternalCalls.AudioSourceComponent_GetGain(Entity.ID, out float gain);
                return gain;
            }
            set
            {
                InternalCalls.AudioSourceComponent_SetGain(Entity.ID, ref value);
            }
        }
        
        public void Play()
        {
            InternalCalls.AudioSourceComponent_Play(Entity.ID);
        }
    }

    public class Rigidbody2DComponent : Component
    {
        public Vector2 LinearVelocity
        {
            get
            {
                InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
                return velocity;
            }
        }
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 point, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref point, ref wake);
        }

        public void ApplyLinearImpulseToCenter(Vector2 impulse, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, ref wake);
        }
    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 Color
        {
            get
            {
                InternalCalls.SpriteRendererComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }
            set
            {
                InternalCalls.SpriteRendererComponent_SetColor(Entity.ID, ref value);
            }
        }
    }

    public class TextRendererComponent : Component
    {
        public string Text
        {
            get
            {
                return InternalCalls.TextRendererComponent_GetText(Entity.ID);
            }
            set
            {
                InternalCalls.TextRendererComponent_SetText(Entity.ID, value);
            }
        }

        public Vector4 TextColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetTextColor(Entity.ID, out Vector4 textColor);
                return textColor;
            }
            set
            {
                InternalCalls.TextRendererComponent_SetTextColor(Entity.ID, ref value);
            }
        }

        public Vector4 BGColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetBGColor(Entity.ID, out Vector4 bgColor);
                return bgColor;
            }
            set
            {
                InternalCalls.TextRendererComponent_SetBGColor(Entity.ID, ref value);
            }
        }
        
        public float LineHeight
        {
            get
            {
                return InternalCalls.TextRendererComponent_GetLineHeight(Entity.ID);
            }
            set
            {
                InternalCalls.TextRendererComponent_SetLineHeight(Entity.ID, value);
            }
        }
        
        public float LineSpacing
        {
            get
            {
                return InternalCalls.TextRendererComponent_GetLineSpacing(Entity.ID);
            }
            set
            {
                InternalCalls.TextRendererComponent_SetLineSpacing(Entity.ID, value);
            }
        }

    }

    public class GUICanvasComponent : Component
    {
        public bool ControlMouse
        {
            get
            {
                InternalCalls.GUICanvasComponent_GetControlMouse(Entity.ID, out bool control);
                return control;
            }
            set
            {
                InternalCalls.GUICanvasComponent_SetControlMouse(Entity.ID, ref value);
            }
        }
        public bool ShowMouse
        {
            get
            {
                InternalCalls.GUICanvasComponent_GetShowMouse(Entity.ID, out bool show);
                return show;
            }
            set
            {
                InternalCalls.GUICanvasComponent_SetShowMouse(Entity.ID, ref value);
            }
        }
        public enum CanvasMode
        {
            None = 0,
            Overlay,
            World
        }
        public CanvasMode Mode
        {
            get
            {
                InternalCalls.GUICanvasComponent_GetMode(Entity.ID, out CanvasMode mode);
                return mode;
            }
            set
            {
                InternalCalls.GUICanvasComponent_SetMode(Entity.ID, ref value);
            }
        }

    }

    public class GUILayoutComponent : Component
    {
        // TODO : Mode, Size, Padding
    }

    public class GUIImageComponent : Component
    {
        public Vector4 Color
        {
            get
            {
                InternalCalls.GUIImageComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }
            set
            {
                InternalCalls.GUIImageComponent_SetColor(Entity.ID, ref value);
            }
        }
    }

    public class GUIButtonComponent : Component
    {
        // TODO : Colors, Textures
        public Vector4 ActiveColor
        {
            get
            {
                InternalCalls.GUIButtonComponent_GetEnabledColor(Entity.ID, out Vector4 enabledColor);
                return enabledColor;
            }
            set
            {
                InternalCalls.GUIButtonComponent_SetEnabledColor(Entity.ID, ref value);
            }
        }

        public Vector4 HoveredColor
        {
            get
            {
                InternalCalls.GUIButtonComponent_GetHoveredColor(Entity.ID, out Vector4 hoveredColor);
                return hoveredColor;
            }
            set
            {
                InternalCalls.GUIButtonComponent_SetHoveredColor(Entity.ID, ref value);
            }
        }
    }

    public class GUIInputFieldComponent : Component
    {
        public string Text
        {
            get
            {
                return InternalCalls.GUIInputFieldComponent_GetText(Entity.ID);
            }
            set
            {
                InternalCalls.GUIInputFieldComponent_SetText(Entity.ID, value);
            }
        }

        public Vector4 TextColor
        {
            get
            {
                InternalCalls.GUIInputFieldComponent_GetTextColor(Entity.ID, out Vector4 textColor);
                return textColor;
            }
            set
            {
                InternalCalls.GUIInputFieldComponent_SetTextColor(Entity.ID, ref value);
            }
        }

        public Vector4 BGColor
        {
            get
            {
                InternalCalls.GUIInputFieldComponent_GetBGColor(Entity.ID, out Vector4 bgColor);
                return bgColor;
            }
            set
            {
                InternalCalls.GUIInputFieldComponent_SetBGColor(Entity.ID, ref value);
            }
        }

        public float LineHeight
        {
            get
            {
                return InternalCalls.GUIInputFieldComponent_GetLineHeight(Entity.ID);
            }
            set
            {
                InternalCalls.GUIInputFieldComponent_SetLineHeight(Entity.ID, value);
            }
        }

        public float LineSpacing
        {
            get
            {
                return InternalCalls.GUIInputFieldComponent_GetLineSpacing(Entity.ID);
            }
            set
            {
                InternalCalls.GUIInputFieldComponent_SetLineSpacing(Entity.ID, value);
            }
        }

    }

    public class GUISliderComponent : Component
    {
        public float Fill
        {
            get
            {
                InternalCalls.GUISliderComponent_GetFill(Entity.ID, out float fill);
                return fill;
            }
            set
            {
                InternalCalls.GUISliderComponent_SetFill(Entity.ID, ref value);
            }
        }

    }

}
