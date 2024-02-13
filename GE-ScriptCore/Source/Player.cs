using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GE;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_TransformComponent;
        private Rigidbody2DComponent m_Rigidbody2DComponent;

        void OnCreate()
        {
            Console.WriteLine("Player::OnCreate()");

            m_TransformComponent = GetComponent<TransformComponent>();
            m_Rigidbody2DComponent = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float timestep)
        {

            float speed = 5.0f;
            Vector3 velocity = new Vector3(0, 0, 0);

            if (Input.IsKeyDown(KeyCode.KEY_W))
            {
                velocity.y += speed * timestep;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_S))
            {
                velocity.y -= speed * timestep;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_A))
            {
                velocity.x -= speed * timestep;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_D))
            {
                velocity.x += speed * timestep;
            }

            Vector3 translation = m_TransformComponent.Translation;
            translation += velocity;
            m_TransformComponent.Translation = translation;

            if (Input.IsKeyDown(KeyCode.KEY_E))
            {
                m_Rigidbody2DComponent.ApplyLinearImpulseToCenter(velocity.XY, false);
            }
        }
    }
}
