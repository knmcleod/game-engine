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
        public Vector3 Velocity;

        void OnCreate()
        {
            Console.WriteLine("Player::OnCreate()");

            m_TransformComponent = GetComponent<TransformComponent>();
            m_Rigidbody2DComponent = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float timestep)
        {
            if (Input.IsKeyDown(KeyCode.KEY_W))
            {
                Velocity.y += Speed * timestep;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_S))
            {
                Velocity.y -= Speed * timestep;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_A))
            {
                Velocity.x -= Speed * timestep;
            }
            else if (Input.IsKeyDown(KeyCode.KEY_D))
            {
                Velocity.x += Speed * timestep;
            }

            Vector3 translation = m_TransformComponent.Translation;
            translation += Velocity;
            m_TransformComponent.Translation = translation;

            if (Input.IsKeyDown(KeyCode.KEY_E))
            {
                m_Rigidbody2DComponent.ApplyLinearImpulseToCenter(Velocity.XY, false);
            }
        }
    }
}
