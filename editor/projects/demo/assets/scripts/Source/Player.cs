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
        private AudioSourceComponent m_AudioSourceComponent;
        private Rigidbody2DComponent m_Rigidbody2DComponent;
        private TextRendererComponent m_TextRendererComponent;

        public float Speed;

        Camera camera;

        void OnCreate()
        {
            m_AudioSourceComponent = GetComponent<AudioSourceComponent>();
            m_Rigidbody2DComponent = GetComponent<Rigidbody2DComponent>();
            m_TextRendererComponent = GetComponent<TextRendererComponent>();

            Entity cameraEntity = FindEntityByTag("Camera");
            if (cameraEntity != null)
                camera = FindEntityByTag("Camera").As<Camera>();

        }
        void OnUpdate(float timestep)
        {
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.KEY_W))
            {
                velocity.y = 1.0f * Speed;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_S))
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

            velocity *= timestep;
            if (m_AudioSourceComponent != null && Input.IsKeyDown(KeyCode.KEY_SPACE))
                m_AudioSourceComponent.Play();

            if (m_TextRendererComponent != null)
            {
                m_TextRendererComponent.Text = string.Format("V: {0}, {1}, {2}", velocity.x.ToString("0.0"), velocity.y.ToString("0.0"), velocity.z.ToString("0.0"));
            }
            
            m_Rigidbody2DComponent.ApplyLinearImpulseToCenter(velocity.XY, true);

            if(camera != null)
            {
                if (Input.IsKeyDown(KeyCode.KEY_Q))
                {
                    camera.FollowDistance += Speed * timestep;
                }
                else if (Input.IsKeyDown(KeyCode.KEY_E))
                {
                    camera.FollowDistance -= Speed * timestep;
                }
            }
            else
            {
                camera = FindEntityByTag("Camera").As<Camera>();
            }
        }
    }
}
