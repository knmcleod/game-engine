using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GE;

namespace demo
{
    public class Player : Entity
    {
        private TransformComponent m_TransformComponent;
        private Rigidbody2DComponent m_Rigidbody2DComponent;

        public float Speed;

        Camera camera;

        void OnCreate()
        {
            m_TransformComponent = GetComponent<TransformComponent>();
            m_Rigidbody2DComponent = GetComponent<Rigidbody2DComponent>();

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

            Log.LogCoreInfo("Velocity Applied = ( " + velocity.x + ", " + velocity.y + ", " + velocity.z + ")");
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
