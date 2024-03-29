using System;
using System.Collections.Generic;
using System.Linq;
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

}
