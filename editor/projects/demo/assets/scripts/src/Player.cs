using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GE;

namespace demo
{
    public class Player : Entity
    {
        private TransformComponent m_TransformComponent;
        private AudioSourceComponent m_AudioSourceComponent;
        private Rigidbody2DComponent m_Rigidbody2DComponent;
        private SpriteRendererComponent m_SpriteRendererComponent;
        private TextRendererComponent m_TextRendererComponent;
 
        private Camera camera;

        public float Speed = 1.0f;

        public bool CenterVelocity = false;
        public Vector4 Color = Vector4.Zero;

        protected override bool OnEvent(Event e)
        {
            return base.OnEvent(e);
        }
        protected override bool OnMousePressed(Event e, MouseCode mouseCode)
        {
            return false;
        }

        protected override void OnCreate()
        {
            m_TransformComponent = GetComponent<TransformComponent>();
            m_AudioSourceComponent = GetComponent<AudioSourceComponent>();
            m_Rigidbody2DComponent = GetComponent<Rigidbody2DComponent>();
            m_SpriteRendererComponent = GetComponent<SpriteRendererComponent>();
            m_TextRendererComponent = GetComponent<TextRendererComponent>();

            Entity cameraEntity = FindEntityByName("Camera");
            if (cameraEntity != null)
                camera = cameraEntity.As<Camera>();

            if(m_SpriteRendererComponent != null)
                Color = m_SpriteRendererComponent.Color;
        }

        protected override void OnUpdate(float ts)
        {
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.KEY_S))
            {
                velocity.y = -1.0f * Speed;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_A))
            {
                velocity.x = -1.0f * Speed;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_D))
            {
                velocity.x = 1.0f * Speed;
            }

            if (Input.IsKeyDown(KeyCode.KEY_SPACE))
            {
                m_AudioSourceComponent?.Play();
                velocity.y = 1.0f * Speed;
            }

            if (m_TextRendererComponent != null)
            {
                // TODO : Add Points system. Points gained by touching platforms ingame
                //m_TextRendererComponent.Text = string.Format("V: {0}, {1}, {2}", velocity.x.ToString("0.0"), velocity.y.ToString("0.0"), velocity.z.ToString("0.0"));
            }

            if(m_SpriteRendererComponent != null && !m_SpriteRendererComponent.Color.IsEqual(Color))
            {
                m_SpriteRendererComponent.Color = Color;
            }

            velocity *= ts;
            if(CenterVelocity)
                m_Rigidbody2DComponent.ApplyLinearImpulseToCenter(velocity.XY, true);
            else
                m_Rigidbody2DComponent.ApplyLinearImpulse(velocity.XY, m_TransformComponent.Pivot.XY, true);

            if (camera != null)
            {
                if (Input.IsKeyDown(KeyCode.KEY_Q))
                {
                    camera.FollowDistance += Speed * ts;
                }
                else if (Input.IsKeyDown(KeyCode.KEY_E))
                {
                    camera.FollowDistance -= Speed * ts;
                }
            }
            else
            {
                Entity camerEntity = FindEntityByName("Camera");
                if (camerEntity != null)
                    camera = camerEntity.As<Camera>();
            }
        }
    }
}