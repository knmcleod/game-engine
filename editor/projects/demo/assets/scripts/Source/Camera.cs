using GE;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace demo
{
    public class Camera : Entity
    {
        public Entity FollowEntity;

        public float FollowDistance = 1.0f;
        void OnCreate()
        {
            FollowEntity = FindEntityByTag("Player");
            Translation = new Vector3(0);
        }

        void OnUpdate(float timestep)
        {
            if(FollowEntity != null)
            {
                Translation = new Vector3(FollowEntity.Translation.x,
                    FollowEntity.Translation.y, FollowDistance);
            }
            else
            {
                FollowEntity = FindEntityByTag("Player");
            }
        }
    }
}
